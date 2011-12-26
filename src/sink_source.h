// -*-c++-*-

/**
 * Abstract away I/O: inspired Sink and Source classes in
 * Google's Snappy implementation.
 * < http://code.google.com/p/snappy/source/browse/trunk/snappy-sinksource.h > 
 */
#ifndef ALZ_SINK_SOURCE_H_
#define ALZ_SINK_SOURCE_H_

#include <cstdlib>

namespace alz {

class Sink {
  public:
    Sink();
    virtual ~Sink();

    virtual void append(const char *bytes, size_t n) = 0;
    virtual const char *peek_back(size_t offset) = 0;
    virtual size_t pos() const = 0;

    virtual void flush();
};

class Source {
  public:
    Source();
    virtual ~Source();

    virtual size_t available() const = 0;
    virtual size_t pos() const = 0;
    virtual const char *peek() = 0;
    virtual const char *peek_back(size_t offset) = 0;
    virtual void skip(size_t n) = 0;
};

class ByteArraySink : public Sink {
  public:
    explicit ByteArraySink(char *dest);
    virtual ~ByteArraySink();

    virtual void append(const char *bytes, size_t n);
    virtual const char *peek_back(size_t offset) { return dest_ - offset - 1; }
    virtual size_t pos() const { return pos_; }
  private:
    char *dest_;
    size_t pos_;
};

class ByteArraySource : public Source {
  public:
    ByteArraySource(const char *src, size_t n);
    virtual ~ByteArraySource();

    virtual size_t pos() const { return pos_; }
    virtual size_t available() const { return left_; }
    virtual const char *peek() { return src_; }
    virtual const char *peek_back(size_t offset) { return src_ - offset - 1; }
    virtual void skip(size_t n);
  private:
    const char *src_;
    size_t left_;
    size_t pos_;
};

inline void ByteArraySource::skip(size_t n) {
    left_ -= n;
    src_ += n;
    pos_ += n;
}

} // namespace alz

#endif // ALZ_SINK_SOURCE_H_
