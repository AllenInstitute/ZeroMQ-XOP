#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief Mutex which serves the waiting threads in the order they arrived
///
/// A thread which releases a std::mutex can take it again before a waiting
/// thread wakes up, so the waiting thread can wait forever. MSVC builds
/// std::mutex and std::recursive_mutex on SRW locks, which are neither fair
/// nor FIFO, and the pthread mutex on macOS gives no ordering guarantee
/// either. FairMutex is a ticket lock: lock() draws a ticket and waits until
/// unlock() serves it.
///
/// It is not recursive, locking it again from the owning thread throws.
class FairMutex
{
public:
  FairMutex()                             = default;
  ~FairMutex()                            = default;
  FairMutex(const FairMutex &)            = delete;
  FairMutex &operator=(const FairMutex &) = delete;
  FairMutex(FairMutex &&)                 = delete;
  FairMutex &operator=(FairMutex &&)      = delete;

  void lock();
  void unlock();

private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  uint64_t m_nextTicket{0};
  uint64_t m_servedTicket{0};
  std::thread::id m_owner;
};

using LockGuardFair = std::scoped_lock<FairMutex>;
