#include <matf/verification/metamorphic_testing/pdf/pdf_splitter.hpp>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

namespace matf::verification::metamorphic_testing::pdf {

std::vector<std::vector<std::byte>> split_pages(std::span<const std::byte> pdf) {
    QPDF source;
    source.processMemoryFile("input", reinterpret_cast<const char*>(pdf.data()), pdf.size());
    QPDFPageDocumentHelper source_pages(source);

    std::vector<std::vector<std::byte>> pages;
    for (auto& page : source_pages.getAllPages()) {
        QPDF single;
        single.emptyPDF();
        QPDFPageDocumentHelper(single).addPage(page, false);

        QPDFWriter writer(single);
        writer.setOutputMemory();
        writer.write();

        const auto buffer = writer.getBufferSharedPointer();
        const auto* first = reinterpret_cast<const std::byte*>(buffer->getBuffer());
        pages.emplace_back(first, first + buffer->getSize());
    }
    return pages;
}

} // namespace matf::verification::metamorphic_testing::pdf
