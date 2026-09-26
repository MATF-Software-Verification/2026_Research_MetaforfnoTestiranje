# Opis sistema

## Autori

- Đorđe Marić 1020/2025
- Lazar Cvijić 1030/2025

## Opis problema

Sistem za pretraživanje prima upit i vraća dokumente koji mu odgovaraju. Testiranje ovakvog sistema je teško jer
ne postoji praktično **proročište** (engl. *test oracle*): za realan korpus i proizvoljan upit ne možemo unapred reći koji dokumenti
*treba* da budu vraćeni.

**Metamorfno testiranje** zaobilazi potrebu za proročištem. Umesto da se jedan izlaz poredi sa poznatim odgovorom,
proverava se *relacija između izlaza za dva povezana ulaza*. Ako upit `q` vraća skup `R(q)`, a iz njega poznatom
transformacijom izvedemo novi upit `q'`, onda između `R(q)` i `R(q')` mora da važi neko svojstvo, bez obzira na
stvarni sadržaj skupova rezultata.

Ovaj projekat primenjuje metamorfno testiranje na **Elasticsearch**, i može se proširiti na druge sisteme za pretragu. Korpus je jedan PDF dokument. Alat ga
indeksira, pravi upite od reči koje se u njemu zaista nalaze i proverava metamorfne relacije nad
pretraživačem. Narušena relacija ukazuje na grešku ili na nedokumentovano ponašanje u načinu na koji pretraživač
analizira tekst ili izvršava upite.

## Arhitektura sistema

Projekat se sastoji od biblioteke (`library/`) i malog izvršnog programa koji je pokreće
(`runner/`). Zavisnostima upravlja Conan, a prevođenjem CMake. Sav kod se nalazi u prostoru imena
`matf::verification::metamorphic_testing`.

```
                    ┌──────────────────────────── runner/test.cpp ────────────────────────────┐
                    │                                                                          │
  file.pdf ──► pdf::split_pages ──► ElasticsearchSearchClient::index_document (strana = dokument)│
                    │                         │                                                │
                    │                         ▼                                                │
                    │               get_tokens() ──► tokens.txt ──► TokenGenerator(seed)       │
                    │                                                     │                    │
                    │                                                     ▼                    │
                    │          Verifier ◄── MetamorphicRelation × 8 (generate / mutate / holds)│
                    │             │                                                            │
                    │             └──► SearchEngineClient::query(q, op), query(q', op)         │
                    └──────────────────────────────────────────────────────────────────────────┘
                                                        │
                                                        ▼
                                          Elasticsearch 8.15 (Docker)
```

### Moduli

| Modul | Fajlovi | Odgovornost |
|---|---|---|
| Klijenti pretraživača | `clients/search_engine_client.hpp` | Apstraktni interfejs sistema koji se testira: `index_document(id, bytes)` i `query(text, operator) → set<int>`. |
| | `clients/elasticsearch_search_client.{hpp,cpp}` | Pravi klijent. Komunicira sa Elasticsearch-om preko HTTP-a (**cpp-httplib**, **nlohmann_json**, **b64**). |
| Operator upita | `query_operator.hpp` | `enum class QueryOperator { Or, And }`, preslikava se na operator Elasticsearch `match` upita. |
| Generator tokena | `token_generator.{hpp,cpp}` | Čuva rečnik korpusa i `std::mt19937` inicijalizovan semenom. Vraća nasumične validne tokene i nasumične *nevalidne* tokene za koje je garantovano da nisu u rečniku. |
| Metamorfne relacije | `relations/metamorphic_relation.hpp` | Bazna klasa. Relacija određuje kako se generiše polazni ulaz, kako se menja, koji operator upita se koristi i kada relacija važi. |
| | `relations/*.{hpp,cpp}` | Osam konkretnih relacija (videti ispod). |
| Verifikator | `verifier.hpp` | Proverava jednu relaciju nad klijentom: generiše ulaz, menja ga, izvršava oba upita i računa `holds`. |
| Pokretač | `runner/test.cpp` | Izvršni program `test_run`. Obrađuje argumente, indeksira PDF, izvozi tokene, proverava sve relacije i ispisuje rezime. |
| Deljenje PDF-a | `pdf/pdf_splitter.{hpp,cpp}` | Deli PDF u memoriji na PDF-ove od po jedne strane, pomoću biblioteke **qpdf**. |

### Elasticsearch klijent

Konstruktor priprema čisto okruženje:

1. `wait_until_ready()` do 30 puta proverava `/_cluster/health?wait_for_status=yellow`, pa se alat može pokrenuti
   odmah posle `docker compose up`.
2. `ensure_pipeline()` pravi ingest pipeline `pdf_pipeline`. On koristi **attachment** procesor (Apache Tika
   unutar Elasticsearch-a) da iz PDF-a kodiranog u base64 izvuče tekst u polje `attachment.content`, a zatim
   uklanja sirove bajtove. `indexed_chars: -1` isključuje podrazumevano ograničenje broja izvučenih karaktera.
3. `recreate_index()` briše i ponovo pravi indeks `docs`, sa `pdf_pipeline` kao podrazumevanim pipeline-om.
   Svako pokretanje zato počinje od praznog indeksa.

Ostale operacije:

- `index_document` šalje stranu kodiranu u base64 i koristi `?refresh=true`, tako da je dokument pretraživ čim
  se poziv završi.
- `query` šalje `match` upit nad poljem `attachment.content` sa izabranim operatorom.
- `get_tokens` prvo izlista sve identifikatore dokumenata, a zatim pomoću `_mtermvectors` čita njihove vektore
  termova. Vraća sortiranu uniju svih termova.

## Opis rešenja

### Osnovna ideja

Svaka relacija je trojka **(generator polaznog ulaza, transformacija, predikat relacije)**, uz operator upita
koji predikat podrazumeva. Za relaciju `MR`:

```
q   = MR.generate_input()          // iz rečnika korpusa
q'  = MR.mutate_input(q)
op  = MR.get_operator()
R   = client.query(q,  op)
R'  = client.query(q', op)
prolazi ⇔ MR.holds(R, R')
```

Upravo ovo radi `Verifier::verify_relation`. Verifikator ne zna ništa o pojedinačnim relacijama niti o
Elasticsearch-u, pa se nove relacije i novi pretraživači mogu dodavati nezavisno jedni od drugih.

### Osnovni algoritam (`test_run <file.pdf> [--seed <n>]`)

1. Izabere se seme: vrednost `--seed`, ili nova vrednost iz `std::random_device`. Seme se ispisuje u log.
2. Uspostavi se veza sa Elasticsearch-om i resetuju se pipeline i indeks.
3. PDF se podeli na strane i strana `i` se indeksira kao dokument `i` (identifikatori počinju od 1).
4. Pomoću `get_tokens()` se dobija indeksirani rečnik i upisuje u `tokens.txt`.
5. Od `tokens.txt` i semena se pravi `TokenGenerator`.
6. Verifikator se pokreće po jednom za svaku od osam relacija i pamte se imena relacija koje ne važe.
7. Ispisuje se `SUCCESS: all 8 relations hold (seed N)` ili `FAILURE: k of 8 relations failed (seed N): …`.

### Relacije

`R` je rezultat polaznog upita, a `R'` rezultat izvedenog upita.

| Relacija | Operator | Polazni ulaz `q` | Izvedeni ulaz `q'` | Važi kada | Obrazloženje |
|---|---|---|---|---|---|
| `capitalization_irrelevance` | OR | jedan token | `q` velikim slovima (ili malim, ako nema malih slova) | `R = R'` | Standardni analizator pretvara termove u mala slova, pa veličina slova ne sme da utiče na rezultat. |
| `whitespace_punctuation_irrelevance` | OR | jedan token | `q` obavijen nasumičnom interpunkcijom (`"…"`, `(…)`, `«…»`, `…` na kraju, `#` na početku, …) i nasumičnim belinama | `R = R'` | Tokenizator odbacuje interpunkciju i beline. |
| `term_addition_monotonicity` | OR | jedan token | `q` + još jedan nasumičan token | `R ⊆ R'` | Disjunkcija sa više termova može samo da pogodi više dokumenata. |
| `duplicate_term_irrelevance` | OR | jedan token | `q q` | `R = R'` | Ponavljanje terma menja skor, a ne skup pogodaka. |
| `multiple_term_reduction` | AND | dva tokena `a b` | `a` | `R ⊆ R'` | Uklanjanje člana konjunkcije može samo da proširi rezultat. |
| `input_permutation` | AND | dva tokena `a b` | `b a` | `R = R'` | Konjunkcija je komutativna. |
| `invalid_term_irrelevance` | OR | jedan token | `q` + nevalidan token | `R = R'` | OR sa termom koji ništa ne pogađa ne dodaje ništa. |
| `invalid_term_relevance` | AND | jedan token | `q` + nevalidan token | `R' = ∅` | AND sa termom koji ništa ne pogađa ne pogađa ništa. |

### Ključne odluke

**Jedna strana PDF-a je jedan dokument.** Kada bi ceo PDF bio jedan dokument, svaki rezultat bi bio ili `{}` ili
`{1}`, pa bi relacije nad skupovima poput `⊆` govorile vrlo malo. Deljenjem na strane pomoću qpdf-a od bilo kog
ulaznog fajla dobija se pravi korpus sa više dokumenata, tako da je za ulazni korpus dovoljan bilo koji PDF.

**Tokeni dolaze iz pretraživača, a ne iz našeg parsera.** Rečnik se čita nazad iz vektora termova
Elasticsearch-a, pa sadrži tačno one termove koje je napravio analizator. Upit napravljen od ovih tokena garantovano pogađa
bar jedan dokument, pa testovi imaju smisla umesto da uglavnom porede prazne skupove. Upisivanje u `tokens.txt`
takođe olakšava pregled rečnika.

**Relacija određuje svoj operator upita.** Ista relacija može da važi za `OR`, a da ne važi za `AND`, i obrnuto.
Dodavanje terma je monotono samo za `OR`, a uklanjanje terma proširuje rezultat samo za `AND`. Zato svaka relacija
navodi operator koji njen predikat podrazumeva (`get_operator()`, podrazumevano `Or`), a verifikator ga koristi za
oba upita. Relacija se nikada ne može pokrenuti sa operatorom koji joj ne odgovara.

**Relacija sama generiše polazni ulaz.** Većini relacija je potreban jedan token, ali `input_permutation` i
`multiple_term_reduction` traže dva. `generate_input()` je virtuelna metoda koja podrazumevano vraća jedan token,
pa svaka relacija traži ulaz onog oblika koji joj je potreban.

**Garantovano nevalidni tokeni.** `get_invalid_token()` generiše nasumične niske od 12 malih slova i ponavlja
postupak dok niska nije van rečnika. Na tome se zasnivaju dve relacije `invalid_term_*`: dodati term dokazivo ne
pogađa nijedan dokument.

**Ponovljivost pomoću jednog semena.** Sva nasumičnost (izbor tokena, nevalidni tokeni, interpunkcija i beline)
potiče iz jednog `std::mt19937` u `TokenGenerator`-u, inicijalizovanog semenom sa komandne linije. Seme se uvek
ispisuje, pa pokretanje sa `--seed N` nad istim PDF-om tačno ponavlja izvršavanje.

**Čisto stanje pri svakom pokretanju.** Indeks se briše i ponovo pravi, a svaki dokument se indeksira sa
`refresh=true`. Rezultat testa zato nikada ne zavisi od podataka iz prethodnog pokretanja niti od intervala
osvežavanja Elasticsearch-a.
