#include <cstdlib>

#include "util.h"
#include "decoder.h"
#include "file_io.h"

using namespace alz;

namespace {

using std::make_shared;

void decode_file(const char *input_path,
                 const char *output_path) {
    shared_ptr<FileSource> src = make_shared<FileSource>(input_path);
    shared_ptr<FileSink> sink = make_shared<FileSink>(output_path);
    Decoder dec(src, sink);
    assert(src->open_file());
    assert(sink->open_file());
    Timer timer("decode_file");
    dec.decode();
    src->close_file();
    sink->close_file();
}

} // namespace


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "Usage: %s input output\n",
                argv[0]);
        return EXIT_FAILURE;                
    }
    decode_file(argv[1], argv[2]);
    return EXIT_SUCCESS;
}
