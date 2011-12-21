// -*-c++-*-

#ifndef ALZ_DECODER_H_
#define ALZ_DECODER_H_

#include "bit_stream.h"

namespace alz {

class Decoder {
  public:
    Decoder(const shared_ptr<Source> &src,
            const shared_ptr<Sink> &sink);
    ~Decoder();
    
    void decode();
  private:
    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;
    InBitStream inb_;
};
    
} // namespace alz

#endif // ALZ_DECODER_H_
