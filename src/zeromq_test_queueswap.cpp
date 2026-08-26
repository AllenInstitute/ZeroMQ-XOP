#include "ZeroMQ.h"

#include <chrono>
#include <future>
#include <vector>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

using namespace std::chrono_literals;

constexpr auto TIMEOUT = 10s;

} // anonymous namespace

// variable zeromq_test_queueswap()
//
// Regression test for ConcurrentQueue::apply_to_all's queue-swap fix: the
// functor passed to apply_to_all must be able to push() back into the same
// queue, on the same thread, without deadlocking, because apply_to_all now
// drains the queue into a local container and releases m_mutex before
// invoking the functor, instead of holding it for the whole loop.
extern "C" int zeromq_test_queueswap(zeromq_test_queueswapParams *p)
{
  BEGIN_OUTER_CATCH

  std::promise<void> donePromise;
  auto doneFuture = donePromise.get_future();

  // Run on a detached thread with a bounded wait, exactly like
  // zeromq_test_hb_startstop().
  std::thread worker(
      [promise = std::move(donePromise)]() mutable
      {
        try
        {
          ConcurrentQueue<int> queue;
          queue.push(1);
          queue.push(2);
          queue.push(3);

          std::vector<int> processed;
          queue.apply_to_all(
              [&queue, &processed](int value)
              {
                processed.push_back(value);
                if(value == 1)
                {
                  // Reentrant push from same thread
                  queue.push(99);
                }
              });

          const std::vector<int> expected = {1, 2, 3};
          ASSERT(processed == expected);

          ASSERT(queue.size() == 1);

          std::vector<int> secondPass;
          queue.apply_to_all([&secondPass](int value)
                             { secondPass.push_back(value); });

          const std::vector<int> expectedSecondPass = {99};
          ASSERT(secondPass == expectedSecondPass);
          ASSERT(queue.empty());

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
    throw IgorException(QUEUE_APPLY_DEADLOCK);
  }

  doneFuture.get(); // rethrows if the worker caught an exception

  END_OUTER_CATCH
}
