#ifndef MATCH_RESPONSE_HPP
#define MATCH_RESPONSE_HPP

#include "types.hpp"

#include <set>

class MatchResponse {

  friend BytesBuffer& operator >> (BytesBuffer& buffer, MatchResponse& req);
  friend BytesBuffer& operator << (BytesBuffer& buffer, const MatchRequest& req);

private:
  std::uint32_t _random;
  std::uint32_t _errorCode; // 0 -> ok

  std::set<std::string> _queryResultSet;

public:
  explicit MatchResponse(const std::uint32_t random)
    : _errorCode{ 0 } {
    _random = random;
  }

  MatchResponse(const std::uint32_t random, const std::uint32_t errorCode)
    : _errorCode{ errorCode } {
    _random = random;
  }
public:
  std::uint32_t random() const {
    return _random;
  }

  std::uint32_t errorCode() const {
    return _errorCode;
  }

  std::uint32_t resultCount() const {
    return _queryResultSet.size();
  }

  std::uint32_t lengthInBytes() const {
    return sizeof(MatchResponseHeader) + resultLengthInBytes();
  }

  std::uint32_t resultLengthInBytes() const {
  	const constexpr std::uint32_t zero = 0;
    return _errorCode ? zero : std::accumulate(_queryResultSet.begin(), _queryResultSet.end(), zero, [](std::uint32_t acc, const std::string& s) {
  		                            return acc + s.size() + 1;
  	                           });
	}

public:
  auto resultBegin() -> typename decltype(_queryResultSet)::iterator {
    return _queryResultSet.begin();
  }
    
  auto resultBegin() const -> typename decltype(_queryResultSet)::const_iterator {
    return _queryResultSet.cbegin();
  }
    
  auto resultEnd() -> typename decltype(_queryResultSet)::iterator {
    return _queryResultSet.end();
  }

  auto resultEnd() const -> typename decltype(_queryResultSet)::const_iterator {
    return _queryResultSet.cend();
  }

  template<typename InputIterator, typename String>
  auto insert(InputIterator where, String&& val) {
    return _queryResultSet.insert(where, std::forward<String>(val));
  }
};


BytesBuffer& operator >> (BytesBuffer& buffer, MatchResponse& resp) {
  const auto bufferSize = buffer.second;
  auto p = buffer.first;

  if (bufferSize < sizeof(MatchResponseHeader)) {
    throw SerializationException{ "invalid buffer size" };
  }

  auto header = reinterpret_cast<MatchResponseHeader *>(p);

  // check length again
  if (header->length != bufferSize - sizeof(MatchResponseHeader)) {
    throw SerializationException{ "invalid buffer size" };
  }

  // check random number
  if (header->random != resp.random()) {
    throw SerializationException{ "invalid random number" };
  }

  if (!header->error) {
    auto dataSectionBegin = p + sizeof(MatchResponseHeader);
    auto dataSectionEnd = dataSectionBegin + header->length;

    std::set<std::string> result;
    while (dataSectionBegin < dataSectionEnd) {
      auto stringEnd = std::find(dataSectionBegin, dataSectionEnd, '\0');
      if (stringEnd != dataSectionEnd) {
        result.emplace(dataSectionBegin, stringEnd);
      }

      dataSectionBegin = stringEnd + 1;
    }

    resp._queryResultSet = std::move(result);
  }

  resp._random = header->random;
  resp._errorCode = header->error;

  return buffer;
}

BytesBuffer& operator << (BytesBuffer& buffer, const MatchResponse& resp) {
  const auto bufferSize = buffer.second;
  auto p = buffer.first;

  if (resp.lengthInBytes() > bufferSize) {
    throw SerializationException{ "invalid buffer size" };
  }

  MatchResponseHeader header;
  std::memset(&header, sizeof(MatchResponseHeader), 0);

  header.random = resp.random();
  header.length = resp.resultLengthInBytes();
  header.error = resp.errorCode();

  ::memcpy(p, &header, sizeof(MatchResponseHeader));

  if (!header.error) {
    auto dataSectionBegin = p + sizeof(MatchResponseHeader);
    std::for_each(resp.resultBegin(), resp.resultEnd(), [&](const std::string& s) {
      dataSectionBegin = std::copy(s.cbegin(), s.cend(), dataSectionBegin);
      *dataSectionBegin++ = '\0';
    });
  }

  return buffer;
}

#endif // MATCH_RESPONSE_HPP
