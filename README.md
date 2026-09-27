# 2026_Research_MetaforfnoTestiranje

## Build requirements

| Tool | Version |
|---|---|
| CMake | 4.2 or newer |
| Conan | 2.0 or newer |
| Ninja | any recent |
| Clang | 22 (tested) |
| clang-format | 16 or newer, tested with 22 (formatting only) |
| clang-tidy | 18 or newer, tested with 23 (static analysis only) |

## Build

Install dependencies:

```sh
conan install . -pr:a profiles/linux-clang-debug --build=missing
```

Build:

```sh
cmake --preset conan-debug
cmake --build --preset conan-debug
```

## Run

Start search engine:

```sh
docker compose up -d
```

Index a PDF and test relations:

```sh
./build/Debug/runner/test_run <file.pdf> [--seed <n>]
```
Output will look like:
```text
SUCCESS: all 8 relations hold (seed 42)
FAILURE: 2 of 8 relations failed (seed 42): input_permutation, multiple_term_reduction
```

## VS Code setup

Install the CMake Tools and C/C++ extensions. Then:

1. Run `conan install` as shown above. This creates the presets.
2. Open the project folder.
3. Pick the `conan-debug` preset when VS Code asks.
4. Press F7 to build.

## Formatting

Style lives in `.clang-format` at the repo root. Format every C++ source in place:

```sh
./format.sh
```

Report violations without touching anything (exits non-zero if any are found):

```sh
./format.sh --check
```

Format specific files:

```sh
./format.sh library/src/pdf/pdf_splitter.cpp
```

The script picks the first `clang-format` it finds on `PATH`. Override it with `CLANG_FORMAT=/path/to/clang-format`.

## Static analysis

Checks live in `.clang-tidy` at the repo root. `tidy.sh` needs the compile database, so configure first:

```sh
cmake --preset conan-debug
```

Check every C++ source (exits non-zero if any warning is found):

```sh
./tidy.sh
```

Apply the fixes clang-tidy offers, then reformat:

```sh
./tidy.sh --fix
./format.sh
```

Check specific files:

```sh
./tidy.sh library/src/token_generator.cpp
```

The script picks the first `clang-tidy` it finds on `PATH`, falling back to the
Homebrew LLVM install (`brew install llvm`). Override it with `CLANG_TIDY=/path/to/clang-tidy`
and the build directory with `BUILD_DIR=/path/to/build`.
