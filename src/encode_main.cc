#include <cstdlib>

#include "util.h"
#include "encoder.h"
#include "file_io.h"

using namespace alz;

namespace {

using std::make_shared;

void encode_file(const char *input_path,
                 const char *output_path) {
    shared_ptr<FileSource> src = make_shared<FileSource>(input_path);
    shared_ptr<FileSink> sink = make_shared<FileSink>(output_path);
    Encoder enc(src, sink);
    src->open_file();
    sink->open_file();
    {
        Timer timer("encode_file");
        enc.encode();
        enc.flush();
        sink->flush();
    }
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
    encode_file(argv[1], argv[2]);
    return EXIT_SUCCESS;
}
