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
#include "memmem_opt.h"

namespace alz {

using std::shared_ptr;
using std::vector;

struct HashNode {
    HashNode *next_;
    size_t pos_;
    uint8_t len_;
};

typedef vector<HashNode *> HashTbl;

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
    static const size_t kMinLookAhead = 2;
    static const size_t kHashLen = 16384;

    static size_t hash_fn(const char *inp, uint8_t len);
    
    static bool find_in_window(const char *haystack,
                               size_t haystack_len,
                               const char *needle,
                               size_t needle_len,
                               size_t *needle_pos);
    void init_hash();
    void add_to_hash(const char *inp, uint8_t len, uint16_t locn);
    bool find_in_hash(const char *inp, uint8_t len);
    
    void output_byte();
    bool find_match(const char *inp,
                    size_t look_ahead);
    
    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;    
    OutBitStream outb_;
    size_t init_pos_;
    bool matched_;
    uint16_t match_locn_;
    uint8_t match_len_;
    HashTbl hash_tbl_;
    boost::pool<> pool_;
#ifdef ALZ_DEBUG_
    int hash_found_;
    int hash_not_found_;
#endif // ALZ_DEBUG_
};


inline size_t Encoder::hash_fn(const char *inp, uint8_t len)  {
    size_t h = 5381;
    const char *last = inp + len;
    for ( ; inp < last; ++inp) {
        h = ((h << 5) + h) ^ *inp;
    }
    return h & (kHashLen - 1);
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

inline void Encoder::add_to_hash(const char *inp, uint8_t len, uint16_t locn) {
    size_t hash = hash_fn(inp, len);
    HashNode *new_node = (HashNode *) pool_.malloc();
    assert(new_node != NULL);
    new_node->pos_ = src_->pos() - locn;
    new_node->len_ = len;
    if (hash_tbl_[hash] != NULL) {
        new_node->next_ = hash_tbl_[hash];
    } else {
        new_node->next_ = NULL;
    }
    hash_tbl_[hash] = new_node;
}

} // namespace alz

#endif // ALZ_ENCODER_H_
