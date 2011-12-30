// -*-c++-*-

#ifndef ALZ_UTIL_H_
#define ALZ_UTIL_H_

#include <sys/time.h>
#include <cstdio>

namespace alz {

namespace util {

inline long time_usecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec + tv.tv_sec * 1000000;
}
    
} // namespace util

class Timer {
  public:
    Timer(const char *descr);            
    ~Timer();
    
  private:
    const char *descr_;
    long usecs_start_;
};

inline Timer::Timer(const char *descr)
        :descr_(descr) {
#ifdef ALZ_DEBUG_
    usecs_start_ = util::time_usecs();
#endif // ALZ_DEBUG_
}

inline Timer::~Timer() {
#ifdef ALZ_DEBUG_
    long usecs_end = util::time_usecs();
    fprintf(stderr, "%s: %ld\n", descr_, usecs_end - usecs_start_);
#endif // ALZ_DEBUG_
}



} // namespace alz

#endif // ALZ_UTIL_H_
