#ifndef MATCHREQUEST_HPP
#define MATCHREQUEST_HPP

#include <numeric>

#include "types.hpp"

class Descriptor {
public:
  constexpr const static std::uint32_t lengthInBytes = 32;
    
private:
  std::int8_t *p;
    
public:
  Descriptor(std::int8_t *p)
  :p {p}
  {}
    
public:
  // assignment op
  Descriptor& operator = (const std::string& str) {
      ::memcpy(p, str.c_str(), lengthInBytes);
      return *this;
  }
  
  std::string to_string() const {
      return std::string{ reinterpret_cast<const char *>(p), Descriptor::lengthInBytes };
  }
};

class MatchRequest {

  friend BytesBuffer& operator >> (BytesBuffer& buffer, MatchRequest& req);
  friend BytesBuffer& operator << (BytesBuffer& buffer, const MatchRequest& req);

private:
  std::uint32_t _random;
  std::vector<std::string> _descriptors;

public:
  MatchRequest() = default;
  explicit MatchRequest(const std::uint32_t random) {
    _random = random;
  }

public:
  std::uint32_t random() const {
    return _random;
  }

  std::uint32_t descriptorsCount() const {
    return _descriptors.size();
  }

  std::uint32_t lengthInBytes() const {
    return sizeof(MatchRequestHeader) + descriptorsLengthInBytes();
        }

  std::uint32_t descriptorsLengthInBytes() const {
    return _descriptors.size() * Descriptor::lengthInBytes;
  }

public:
  auto descriptorsBegin() -> typename decltype(_descriptors)::iterator {
    return _descriptors.begin();
  }
    
  auto descriptorsBegin() const -> typename decltype(_descriptors)::const_iterator {
    return _descriptors.cbegin();
  }
    
  auto descriptorsEnd() -> typename decltype(_descriptors)::iterator {
    return _descriptors.end();
  }

  auto descriptorsEnd() const -> typename decltype(_descriptors)::const_iterator {
    return _descriptors.cend();
  }

  template<typename InputIterator, typename String>
  auto insert(InputIterator where, String&& val) {
    return _descriptors.insert(where, std::forward<String>(val));
  }
};

const std::uint32_t Descriptor::lengthInBytes;

class DescriptorsIterator : public std::iterator<std::input_iterator_tag, Descriptor> {
private:
  std::int8_t *p;
  std::uint32_t c;

public:
  DescriptorsIterator(std::int8_t *p, std::uint32_t c = 0)
    : p{ p }
    , c{ c }
  {}
  DescriptorsIterator(std::uint32_t end)
    : p{ nullptr }
    , c{ end }
  {}

  DescriptorsIterator& operator ++() {
    ++c; return *this;
  }

  Descriptor operator * () {
    return Descriptor{ p + c * Descriptor::lengthInBytes };
  }

  bool operator == (const DescriptorsIterator& i) const { return c == i.c; }
  bool operator != (const DescriptorsIterator& i) const { return c != i.c; }
};

BytesBuffer& operator >> (BytesBuffer& buffer, MatchRequest& req) {
  const auto bufferSize = buffer.second;
  auto p = buffer.first;

  if (bufferSize < sizeof(MatchRequestHeader) ||
      (bufferSize - sizeof(MatchRequestHeader)) % Descriptor::lengthInBytes != 0) {
    throw SerializationException{ "invalid buffer size" };
  }

  auto header = reinterpret_cast<MatchRequestHeader *>(p);

  // check length again
  if (header->length != bufferSize - sizeof(MatchRequestHeader)) {
    throw SerializationException{ "invalid buffer size" };
  }

  auto dataSection = p + sizeof(MatchRequestHeader);
  auto countOfDesc = header->length/Descriptor::lengthInBytes;

  std::vector<std::string> descriptors(countOfDesc);
  std::transform(DescriptorsIterator{dataSection}, 
                 DescriptorsIterator{countOfDesc}, 
                 descriptors.begin(),
                 [](const Descriptor& desc) {
                   return desc.to_string();
                 });

  req._random = header->random;
  req._descriptors = std::move(descriptors);

  return buffer;
}

BytesBuffer& operator << (BytesBuffer& buffer, const MatchRequest& req) {
  const auto bufferSize = buffer.second;
  auto p = buffer.first;

  if (req.lengthInBytes() > bufferSize) {
    throw SerializationException{ "invalid buffer size" };
  }

  MatchRequestHeader header;
  std::memset(&header, sizeof(MatchRequestHeader), 0);

  header.random = req.random();
  header.length = req.descriptorsLengthInBytes();

  ::memcpy(p, &header, sizeof(MatchRequestHeader));
  std::copy(req.descriptorsBegin(), req.descriptorsEnd(), DescriptorsIterator{p + sizeof(MatchRequestHeader)});

  return buffer;
}

#endif // MATCHREQUEST_HPP