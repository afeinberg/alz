// -*-c++-*-

#ifndef ALZ_ENCODER_H_
#define ALZ_ENCODER_H_

#include <cstdint>
#include <cstring>
#include <cstdio>

#include <vector>
#include <algorithm>

#include <boost/pool/pool.hpp>

#include "util.h"
#include "bit_stream.h"
#include "constants.h"

namespace alz {

using std::shared_ptr;
using std::vector;
using std::pair;

struct HashNode {
    HashNode *next_;
    size_t pos_;
};

class Encoder {
  public:
    Encoder(const shared_ptr<Source> &src,
            const shared_ptr<Sink> &sink);
    ~Encoder();

    // How many bytes were written
    size_t written() const { return sink_->pos() - init_pos_; }
    
    // Exposed as public for testing in decoder_test.cc
    void emit_literal(char byte);

    // Exposed as public for testing in decoder_test.cc
    void emit_compressed(uint16_t off, uint8_t len);

    // Encode the source to sink
    void encode();

    // Flush out the rest of the buffer to source
    void flush();    
  private:
    static const size_t kMinLookAhead = 3;
    static const size_t kHashLen = 16384;

    // Separate functions to let me experiment with using
    // malloc/free vs. using a memory pool
    HashNode *alloc_node();
    void free_node(HashNode *node);
    static size_t hash_fn(const char *inp);    
    void init_hash();
    void add_to_hash(const char *inp);
    uint8_t find_longest(const char *inp, uint16_t *match_locn);
    
    void output_byte();
    
    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;    
    OutBitStream outb_;
    size_t init_pos_;
    bool matched_;
    HashNode **hash_tbl_;
    boost::pool<> pool_;
#ifdef ALZ_DEBUG_
    size_t added_;
    size_t found_;
#endif // ALZ_DEBUG_
};

    
inline HashNode *Encoder::alloc_node() {
    HashNode *ret = (HashNode *) pool_.malloc();
    assert(ret != NULL);
    return ret;    
}

inline void Encoder::free_node(HashNode *node) {
    assert(node != NULL);
    pool_.free(node);
}

inline size_t Encoder::hash_fn(const char *inp)  {
    // This algorithm seems to work best after
    // several experiments with others (including djb hash,
    // as well as the hash function used by zlib).
    // It's take from ning-compress LZF implementation:
    // < https://github.com/ning/compress/blob/master/src/main/java/com/ning/compress/lzf/ChunkEncoder.java >
    
    uint32_t *ptr = (uint32_t *) inp;
    uint32_t h = (*ptr) & 0xffffff; // get the first 3 bytes
    return ((h * 57321) >> 9) & (kHashLen - 1);    
}

inline void Encoder::emit_literal(char byte) {
    outb_.append(false);
    outb_.append_bits<char, 8>(byte);
}

inline void Encoder::emit_compressed(uint16_t locn, uint8_t len) {
    outb_.append(true);
    outb_.append_bits<uint16_t, 12>(locn);
    outb_.append_bits<uint8_t, 4>(len);
}

inline void Encoder::output_byte() {
    const char *byte = src_->peek();
    emit_literal(*byte);
    src_->skip(1);
}

inline void Encoder::add_to_hash(const char *inp) {
#ifdef ALZ_DEBUG_
    added_++;
#endif // ALZ_DEBUG_
    size_t hash = hash_fn(inp);
    HashNode *new_node = alloc_node();
    new_node->pos_ = src_->pos() + 1;
    new_node->next_ = hash_tbl_[hash];
    hash_tbl_[hash] = new_node;
}

    
} // namespace alz

#endif // ALZ_ENCODER_H_
