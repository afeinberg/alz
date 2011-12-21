#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink) { }

Encoder::~Encoder() { }

void Encoder::flush() {
    outb_.flush();
}

} // namespace alz
