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
    printf("added = %ld, found=%ld\n", added_, found_);
    if (hash_tbl_ != NULL) {
        // Note: there's no need to free the individual
        // lists as the pool allocator takes care of that
        // when its destructor is called.
        delete [] hash_tbl_;
    }
}

void Encoder::init_hash() {
    added_  = 0;
    found_ = 0;
    hash_tbl_ = new HashNode*[kHashLen];
    memset(hash_tbl_, 0, kHashLen);
}

bool Encoder::find_in_hash(const char *inp, uint8_t len) {
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
                node = prev->next_;
            } else {
                if (node->next_) {
                    hash_tbl_[hash] = node->next_;
                    node = hash_tbl_[hash]->next_;
                    prev = hash_tbl_[hash];
                }
                break;
            }
            continue;
        } else if (try_locn >= len) {
            if ((node->len_ >= len) &&
                (memcmp(src_->peek_back(try_locn), inp, len) == 0)) {
                match_locn_ = try_locn;
                match_len_ = len;
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
        match_locn_ = 0;
        match_len_ = 0;
        while (looking && look_ahead <= constants::kMaxLen) {
            bool have_match = find_match(src_->peek(), look_ahead);
          
            if (!have_match) {
                added_++;               
                looking = false;
            } else {
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
            emit_compressed(match_locn_, match_len_);
            src_->skip(match_len_);
        }
    }
}

void Encoder::flush() {
    outb_.flush();
    sink_->flush();
}

bool Encoder::find_match(const char *inp, size_t look_ahead) {   
    if (find_in_hash(inp, look_ahead)) {
        found_++;
        return true;
    }
    add_to_hash(src_->peek(), look_ahead);
    return false;
}

    
} // namespace alz
