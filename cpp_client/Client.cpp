#include "Client.hpp"

#include <random>
#include <list>
#include <map>
#include <atomic>

#include <zmq.hpp>

#include "../cpp_common/common.hpp"

#define MAX_SERVERS_COUNT 4

class Client::Impl {
public:
  zmq::context_t _context;
  std::map<void *, zmq::socket_t *> _sockets;

  std::uint32_t _sessionRandom;
  std::atomic_bool _isSearching;

  Impl()
    : _context{ 1 }
    , _sessionRandom{ 0 }
    , _isSearching{ false } {

    // connect
    const char *addresses[] = {
      "tcp://127.0.0.1:50002"
    };

    for (auto i = 0; i != sizeof(addresses) / sizeof(addresses[0]); ++i) {
      auto socket = new zmq::socket_t{ _context, ZMQ_REQ};

      socket->connect(addresses[i]);
      void *zmqSocket = *socket;

      _sockets[zmqSocket] = socket;
    }
  }

  ~Impl() {
    for (auto s : _sockets) {
      delete s.second;
    }
  }
private:
  auto splitDescriptors(std::uint32_t count, const InputDescriptors& desc) {
    std::list<std::pair<InputDescriptors::const_iterator, InputDescriptors::const_iterator>> splited;

    auto unit = desc.size() / count;

    for (auto i = 0; i != count; ++i) {
      auto begin = desc.cbegin() + unit * i;
      auto end = desc.cbegin() + std::min(desc.size(), unit * (i + 1));
      splited.push_back(std::make_pair(begin, end));
    }

    return splited;
  }

  void sendRequest(InputDescriptors::const_iterator begin, InputDescriptors::const_iterator end, zmq::socket_t *socket) {
    MatchRequest matchReq{ _sessionRandom };
    for (;begin != end; ++begin) {
      matchReq.insert(matchReq.descriptorsEnd(), *begin);
    }

    zmq::message_t req(matchReq.lengthInBytes());
            
    BytesBuffer buf = std::make_pair(static_cast<std::int8_t *>(req.data()), req.size());
    buf << matchReq;

    socket->send(req);
  }

  void receiveResponse(std::set<std::string>& result, zmq::socket_t *socket) {
    zmq::message_t resp;
    socket->recv(&resp);
            
    MatchResponse matchResp{ _sessionRandom };
            
    BytesBuffer buf = std::make_pair(static_cast<std::int8_t *>(resp.data()), resp.size());
    buf >> matchResp;
    
    result.insert(matchResp.resultBegin(), matchResp.resultEnd());
  }

public:
   std::set<std::string> doSearching(const InputDescriptors& desc) {
    auto count = _sockets.size();

    auto ranges = splitDescriptors(count, desc);

    std::set<std::string> result;

    zmq::pollitem_t items[MAX_SERVERS_COUNT * 2];
    ::memset(&items, 0, sizeof(items));
      
    zmq::pollitem_t *p = items;
    zmq::pollitem_t *q = (items + MAX_SERVERS_COUNT);
    
    {
      auto i = 0;
      for (const auto& spair : _sockets) {
        items[i].socket = spair.first;
        items[i].events = ZMQ_POLLOUT;
        ++i;
      }
    }
    
    while (count) {
      if (zmq::poll(p, count, 100) == 0) 
        continue;

      auto newCount = 0;

      // clean next poll items
      ::memset(q, 0, sizeof(zmq::pollitem_t) * MAX_SERVERS_COUNT);

      for (auto i = 0; i != count; ++i) {
        if (p[i].revents == ZMQ_POLLOUT) {
          auto range = ranges.back();
          sendRequest(range.first, range.second, _sockets[p[i].socket]);
          ranges.pop_back();
          
          q[newCount].socket = p[i].socket;
          q[newCount].events = ZMQ_POLLIN;

          ++newCount;

        } else if (p[i].revents == ZMQ_POLLIN) {
          receiveResponse(result, _sockets[p[i].socket]);
        }
      }

      count = newCount;
      std::swap(p, q);
    }

    return result;
  }

  bool beginSearching() {
    if (_isSearching.exchange(true)) {
      // busy
      return false;
    }

    std::random_device dev;
    std::mt19937 gen(dev());

    _sessionRandom = gen();

    return true;
  }

  void endSearching() {
    _isSearching = false;
  }
};

Client::Client()
  :_impl { std::make_unique<Impl>() }
{}

Client::~Client() {

}

 // block
std::tuple<std::set<std::string>, bool> Client::search(const InputDescriptors& desc) {
  std::set<std::string> result;

  if (!_impl->beginSearching()) {
    return std::make_tuple(result, false);
  }

  try {
    result = _impl->doSearching(desc);
  } catch(...) {}

  _impl->endSearching();

  return std::make_tuple(result, true);;
}

bool Client::isSearching() const {
  return _impl->_isSearching;
}