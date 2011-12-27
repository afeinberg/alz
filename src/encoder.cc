#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),         
         init_pos_(sink_->pos()) { }

Encoder::~Encoder() {
    
#ifdef ALZ_DEBUG_
    printf("hash_found = %d, hash_not_found = %d\n",
           hash_found_,
           hash_not_found_);
#endif // ALZ_DEBUG_
    
}

void Encoder::init_hash() {
    
#ifdef ALZ_DEBUG_
    hash_found_ = 0;
    hash_not_found_ = 0;
#endif // ALZ_DEBUG_
    
    hash_tbl_.reserve(kHashLen);
    std::fill(hash_tbl_.begin(),
              hash_tbl_.end(),
              pair<size_t, uint8_t>(0, 0));
}

void Encoder::encode() {
    init_hash();
    output_byte();
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


/**
 * Note: this is slow!
 *
 * My optimizations here were as follows:
 *
 * 1) Use a rolling hash type algorithm (see memmem_opt.c) vs. the
 *    naive use of of memem() in libc. I also tried another algorithm,
 *    Boyer-Moore-Horspool, but performed slightly worse than the
 *    rolling hash algorithm used.
 *
 * 2) Using a hash table to keep track of previous matches. This took
 *    several iterations. First approach was to naively use
 *    std::unordered_map from STL, supplying a custom hash and equals
 *    function. This actually ended up being _slower_ than not using a
 *    hash table at all.
 *
 *    My next approach was to implement just the hash table buckets,
 *    but no collision resolution algorithm (to avoid the overhead of
 *    keeping a linked list for the chaining method of collision
 *    resolution).
 *
 *    I then experimented with implementing chaining collision
 *    resolution, using a memory pool for allocating the linked list
 *    nodes. However, I found that given the small sliding window
 *    (4096 bytes, i.e., 2^12) there was performance benefit (and even
 *    a slight performance) cost to using chaining vs. simply ignoring
 *    hash collisions (treating them as cache misses). This, of
 *    course, came at the cost of higher memory utilization due to a
 *    larger bucket table. If you note in the code, when ALZ_DEBUG_ is
 *    enabled I keep track of hits and misses -- the percentage
 *    increase in cache hits was only a few percent higher as result
 *    of using chaining collision resolution.
 *
 *    Based on these results, I chose the "use just the hash table
 *    buckets, no resolution, treat collisions as cache misses"
 *    approach.
 *
 *    One approach I did not, however, experiment with was using a
 *    very small fast to compute hash function (with lots of
 *    collisions). The reason I didn't experiment with this is that
 *    cost of computing the hash is fairly minor due to the cost of
 *    finding the match by searching.
 *
 * After being told by Glen that I could look at other lz77
 * implementations, I found that a better approach would have been to
 * pre-build a data structure for efficient matches and then use that
 * to do efficient searching for matches. This is similar to the
 * Knuth-Morris-Pratt search algorithm, which is used by some lz77
 * implementations.
 *
 * However, it looks like it would have required essentially a
 * re-write of the structures I used to do the "sliding window".
 */
bool Encoder::find_match(const char *inp, size_t look_ahead) {
   
    if (find_in_hash(inp, look_ahead)) {
        
#ifdef ALZ_DEBUG_
        hash_found_++;
#endif // ALZ_DEBUG_
        
        return true;
    }
    
    size_t off;
    size_t lim;
    
    if (src_->pos() - 1 > constants::kMaxOffset) {
        off = constants::kMaxOffset;
        lim = constants::kMaxOffset;
    } else {
        off = src_->pos() - 1;
        lim = src_->pos() - 1;        
    }
    const char *win;
    if (matched_) {
        win = src_->peek_back(match_locn_);
        lim = match_locn_;
    } else {
        win = src_->peek_back(off);
    }

    size_t pos;
    if (find_in_window(win, lim, inp, look_ahead, &pos)) {
        
#ifdef ALZ_DEBUG_
        hash_not_found_++;
#endif // ALZ_DEBUG_
        
        match_locn_ = lim - pos;        
        match_len_ = look_ahead;
        add_to_hash(inp, match_len_, match_locn_);
        return true;
    }
    return false;
}

// Ugly code, essentially wraps the search function
bool Encoder::find_in_window(const char *haystack,
                             size_t haystack_len,
                             const char *needle,
                             size_t needle_len,
                             size_t *needle_pos) {
    
    const char *needle_found;
    needle_found = static_cast<const char *>(rolling_hash(haystack,
                                                          haystack_len,
                                                          needle,
                                                          needle_len));    
    if (needle_found != NULL) {
        *needle_pos = needle_found - haystack;
        return true;
    } 
    return false;
}

bool Encoder::find_in_hash(const char *inp, uint8_t len) {
    size_t hash = hash_fn(inp, len);
    pair<size_t, uint8_t> val = hash_tbl_[hash];
    if (val.second == 0) {
        return false;
    }
    size_t try_locn = src_->pos() - val.first;
    if (try_locn > constants::kMaxOffset) {
        hash_tbl_[hash] = pair<size_t, uint8_t>(0, 0);
        return false;
    } else {
        if ((val.second >= len) &&
            (memcmp(inp, src_->peek_back(try_locn), len) == 0)) {
            match_locn_ = try_locn;
            match_len_ = len;
            //DLOG("found");
            return true;
        }
    }
    return false;
}
    
} // namespace alz
