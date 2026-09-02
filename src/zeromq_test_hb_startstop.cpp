#include "ZeroMQ.h"

#include <chrono>
#include <future>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

constexpr auto TIMEOUT = 10s;

} // anonymous namespace

// variable zeromq_test_hb_startstop(variable iterations)
extern "C" int zeromq_test_hb_startstop(zeromq_test_hb_startstopParams *p)
{
  BEGIN_OUTER_CATCH

  const auto iterations = To<int>(p->iterations);
  ASSERT(iterations >= 1);

  std::promise<void> donePromise;
  auto doneFuture = donePromise.get_future();

  std::thread worker(
      [iterations, promise = std::move(donePromise)]() mutable
      {
        try
        {
          // ensure a known, stopped baseline
          HeartbeatPublisher::Instance().Stop();

          for(int i = 0; i < iterations; i++)
          {
            HeartbeatPublisher::Instance().Start();
            HeartbeatPublisher::Instance().Stop();
          }

          // restore the normal steady-state (started) status set up by
          // XOPMain()
          HeartbeatPublisher::Instance().Start();

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
    throw IgorException(HEARTBEAT_STARTSTOP_DEADLOCK);
  }

  doneFuture.get(); // rethrows if the worker caught an exception

  END_OUTER_CATCH
}
