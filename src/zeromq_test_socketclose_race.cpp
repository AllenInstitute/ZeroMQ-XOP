#include "ZeroMQ.h"

#include <atomic>
#include <chrono>
#include <future>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

// Generous upper bound: without the fix, a sender thread that grabs a
// stale, already-closed socket pointer can hang inside libzmq while still
// holding the Publisher mutex, which the closing loop's next iteration
// then blocks on forever -- we must never wait on that unbounded.
constexpr auto TIMEOUT = 15s;

constexpr int ITERATIONS = 2000;

} // anonymous namespace

// variable zeromq_test_socketclose_race()
//
// Regression test for the GET_SOCKET/SocketWithMutex fetch-then-relock race
// (GlobalData::GetOrCreateSocket, SocketWithMutex.h): one thread repeatedly
// closes and rebinds the Publisher socket via GlobalData::CloseConnections()
// (the same path zeromq_stop() uses) while another concurrently calls
// ZeroMQPublisherSend() -- the same function HeartbeatPublisher's
// always-running background thread uses -- in a tight loop (no 5s sleep),
// to get many race attempts quickly. This is the actual production
// interaction that used to freeze Igor Pro.
extern "C" int
zeromq_test_socketclose_race(zeromq_test_socketclose_raceParams *p)
{
  BEGIN_OUTER_CATCH

  std::promise<void> donePromise;
  auto doneFuture = donePromise.get_future();

  // Detached thread + bounded wait, same pattern as
  // zeromq_test_hb_startstop(): a std::future from std::async would block
  // in its destructor until the task finishes, which would turn a detected
  // hang right back into a hang here.
  std::thread worker(
      [promise = std::move(donePromise)]() mutable
      {
        try
        {
          std::atomic<bool> stop{false};

          // Mimics HeartbeatPublisher::WorkerThread's own use of
          // ZeroMQPublisherSend(), minus its 5s sleep.
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
