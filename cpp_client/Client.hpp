#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <set>
#include <memory>
#include <tuple>

class Client {
  using InputDescriptors = std::vector<std::string>;

private:
  class Impl;
  std::unique_ptr<Impl> _impl;

public:
  Client();
  ~Client();

public:
  // block
  std::tuple<std::set<std::string>, bool> search(const InputDescriptors& desc);
  bool isSearching() const;
};

#endif // CLIENT_HPP
