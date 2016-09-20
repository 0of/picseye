#include <vector>
#include <fstream>
#include <limits>
#include <cstdlib>

#include <zmq.hpp>

#include "../cpp_common/common.hpp"
#include "MatchService.h"

void successReply(const MatchRequest& req, zmq::message_t& message, const std::vector<std::string>& matched) {
  MatchResponse resp{ req.random() };
  for (const auto& s : matched) {
    resp.insert(resp.resultEnd(), s);
  }
    
  zmq::message_t reply(resp.lengthInBytes());
  BytesBuffer buf = std::make_pair(static_cast<std::int8_t *>(reply.data()), reply.size());
  buf << resp;
  
  message.move(&reply);
}

void start(const std::string& dbPath, std::uint16_t port) {
  MatchTree bktree{ MatchTree::element_type::New(dbPath, dbPath + "_i") };
  MatchService service{ bktree };

  std::string url = "tcp://0.0.0.0:" + std::to_string(port);

  zmq::context_t context{1};
  zmq::socket_t socket (context, ZMQ_REP);
  socket.bind (url);
    
  while (true) {
    zmq::message_t request;

    socket.recv(&request);

    try {
      MatchRequest matchReq;
      BytesBuffer buf = std::make_pair(static_cast<std::int8_t *>(request.data()), request.size());
      buf >> matchReq;

      // DEBUG
      std::copy(matchReq.descriptorsBegin(), matchReq.descriptorsEnd(), std::ostream_iterator<std::string>(std::cout, " "));

      // do match
      auto matched = service.match(matchReq.descriptorsBegin(), matchReq.descriptorsEnd());
      // std::vector<std::string> matched = {"1019", "1038", "1042"};

      zmq::message_t reply;
      successReply(matchReq, reply, matched);

      socket.send(reply);

    } catch (const SerializationException& ) {}
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    // missing required arguments
    return 1;
  }

  auto dbPath = argv[1];
  auto port = std::atoi(argv[2]);

  start(dbPath, static_cast<std::uint16_t>(port));

  return 0;
}