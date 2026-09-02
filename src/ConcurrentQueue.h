#pragma once

#include <queue>
#include <mutex>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// Implementation of a multi-consumer/multi-producer queue
///
/// The contention due to locks is deemed acceptable.
///
/// Heavily inspired by
/// https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename T>
class ConcurrentQueue
{
  using Lock = std::unique_lock<std::mutex>;

public:
  void push(T data)
  {
    Lock lock(m_mutex);

    m_queue.emplace(data);
  }

  bool empty() const
  {
    Lock lock(m_mutex);

    return m_queue.empty();
  }

  size_t size() const
  {
    Lock lock(m_mutex);

    return m_queue.size();
  }

  bool try_pop(T &popped_value)
  {
    Lock lock(m_mutex);

    if(m_queue.empty())
    {
      return false;
    }

    popped_value = m_queue.front();
    m_queue.pop();

    return true;
  }

  /// Apply the given functor to all elements in the queue
  ///
  /// @tparam Functor must accept an object of type ConcurrentQueue::T
  ///           and *never* throw
  ///
  /// The queue is drained into a local container before F is invoked, so
  /// m_mutex is never held while F runs. This matters because F can run
  /// foreign code that might, on the same thread, push into this ConcurrentQueue.
  template <typename Functor>
  void apply_to_all(Functor F)
  {
    std::queue<T> drained;

    {
      Lock lock(m_mutex);
      std::swap(drained, m_queue);
    }

    for(; !drained.empty(); drained.pop())
    {
      F(drained.front());
    }
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
};
