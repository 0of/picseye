#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <stdexcept>

struct alignas(4) MatchRequestHeader {
  std::uint32_t random;
  std::uint32_t length;
};

struct alignas(4) MatchResponseHeader {
  std::uint32_t random;
  std::uint32_t length;
  std::uint32_t error;
};

using BytesBuffer = std::pair<std::int8_t *, std::uint32_t>;

class SerializationException : public std::runtime_error {
public:
  explicit SerializationException(const char* msg)
  	: std::runtime_error(msg) {}
};

#endif // TYPES_HPP