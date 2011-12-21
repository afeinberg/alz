#include "sink_source.h"

#include <cstring>

namespace alz {

Sink::Sink() { }

Sink::~Sink() { }

Source::Source() { }

Source::~Source() { }

ByteArraySink::ByteArraySink(char *dest)
        :dest_(dest),
         pos_(0) { }

ByteArraySink::~ByteArraySink() { }

void ByteArraySink::append(const char *bytes, size_t n) {
    memcpy(dest_, bytes, n);
    dest_ += n;
    pos_ += n;
}

ByteArraySource::ByteArraySource(const char *src, size_t n)
        :src_(src),
         left_(n),
         pos_(0) { }

ByteArraySource::~ByteArraySource() { }

} // namespace alz
