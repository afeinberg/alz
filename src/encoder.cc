#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),         
         init_pos_(sink_->pos()),
         hash_tbl_(NULL),
         pool_(sizeof(HashNode)) { }

Encoder::~Encoder() {
#ifdef ALZ_DEBUG_
    printf("added = %ld, found=%ld\n", added_, found_);
#endif // ALZ_DEBUG_
    if (hash_tbl_ != NULL) {
        // Note: there's no need to free the individual
        // lists as the pool allocator takes care of that
        // when its destructor is called.
        delete [] hash_tbl_;
    }
}

void Encoder::init_hash() {
#ifdef ALZ_DEBUG_
    added_  = 0;
    found_ = 0;
#endif // ALZ_DEBUG_
    hash_tbl_ = new HashNode*[kHashLen];
    memset(hash_tbl_, 0, kHashLen);
}

inline bool Encoder::find_in_hash(const char *inp,
                                  uint8_t len,
                                  uint16_t *match_locn) {
    size_t hash = hash_fn(inp, len);
    HashNode *list = hash_tbl_[hash];
    if (list == NULL) {
        return false;
    }
    HashNode *prev = NULL;
    HashNode *node = list;
    while (node != NULL) {
        size_t try_locn = src_->pos() - node->pos_;
        if (try_locn >= constants::kMaxOffset) {
            if (prev != NULL) {
                prev->next_ = node->next_;
                free_node(node);
                node = prev->next_;
            } else {
                if (node->next_) {
                    hash_tbl_[hash] = node->next_;
                    free_node(node);
                    node = hash_tbl_[hash]->next_;
                    prev = hash_tbl_[hash];
                } else {
                    hash_tbl_[hash] = NULL;
                    free_node(node);
                }
                break;
            }
            continue;
        } else if (try_locn >= len) {
            if ((node->len_ >= len) &&
                (memcmp(src_->peek_back(try_locn), inp, len) == 0)) {
                *match_locn = try_locn;
                return true;
            }
        }
        prev = node;
        node = node->next_;
    }
    return false;
}
    
void Encoder::encode() {
    init_hash();
    while (src_->available() > 0) {
        if(src_->available() < kMinLookAhead) {
            output_byte();
            continue;
        }
        matched_ = false;
        bool looking = true;
        size_t look_ahead = kMinLookAhead;
        uint16_t match_locn = 0;
        uint8_t match_len = 0;
        while (looking && look_ahead <= constants::kMaxLen) {
            bool have_match = find_in_hash(src_->peek(), look_ahead, &match_locn);
            add_to_hash(src_->peek(), look_ahead);
#ifdef ALZ_DEBUG_
            added_++;
#endif // ALZ_DEBUG_
            if (!have_match) {
                looking = false;
            } else {
#ifdef ALZ_DEBUG_
                found_++;
#endif // ALZ_DEBUG_
                match_len = look_ahead;
                matched_ = true;
                if (src_->available() > look_ahead) {
                    look_ahead++;
                } else {
                    looking = false;                    
                }
            }
        }
        if (!matched_) {
            output_byte();            
        } else {
            emit_compressed(match_locn, match_len);
            src_->skip(match_len);
        }
    }
    flush();
}

void Encoder::flush() {
    outb_.flush();
    sink_->flush();
}
   
} // namespace alz
