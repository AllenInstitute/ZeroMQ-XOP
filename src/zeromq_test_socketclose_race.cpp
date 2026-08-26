#include "ZeroMQ.h"

#include <atomic>
#include <chrono>
#include <future>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

constexpr auto TIMEOUT = 15s;

constexpr int ITERATIONS = 2000;

} // anonymous namespace

// variable zeromq_test_socketclose_race()
extern "C" int
zeromq_test_socketclose_race(zeromq_test_socketclose_raceParams *p)
{
  BEGIN_OUTER_CATCH

  std::promise<void> donePromise;
  auto doneFuture = donePromise.get_future();

  std::thread worker(
      [promise = std::move(donePromise)]() mutable
      {
        try
        {
          std::atomic<bool> stop{false};

          // same as HeartbeatPublisher::WorkerThread without its sleep
          std::thread sender(
              [&stop]()
              {
                while(!stop.load())
                {
                  SendStorageVec sendStorage;
                  sendStorage.emplace_back(SendStorage{"race-test"});
                  sendStorage.emplace_back(SendStorage{""});
                  ZeroMQPublisherSend(sendStorage); // result ignored on purpose
                }
              });

          for(int i = 0; i < ITERATIONS; i++)
          {
            GlobalData::Instance().CloseConnections();

            GET_SOCKET(socket, SocketTypes::Publisher);
            auto rc = zmq_bind(socket.get(), "tcp://127.0.0.1:*");
            ZEROMQ_ASSERT(rc == 0);
            GlobalData::Instance().AddToListOfBindsOrConnections(
                "tcp://127.0.0.1:*", SocketTypes::Publisher);
          }

          stop.store(true);
          sender.join();

          promise.set_value();
        }
        catch(...)
        {
          promise.set_exception(std::current_exception());
        }
      });
  worker.detach();

  if(doneFuture.wait_for(TIMEOUT) != std::future_status::ready)
  {
    throw IgorException(SOCKET_CLOSE_RACE_DEADLOCK);
  }

  doneFuture.get(); // rethrows if the worker caught an exception

  END_OUTER_CATCH
}
