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

constexpr int REENTRANT_VALUE = 99;

} // anonymous namespace

// variable zeromq_test_queueswap()
extern "C" int zeromq_test_queueswap(zeromq_test_queueswapParams *p)
{
  BEGIN_OUTER_CATCH

  std::promise<void> donePromise;
  auto doneFuture = donePromise.get_future();

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
                  queue.push(REENTRANT_VALUE);
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
