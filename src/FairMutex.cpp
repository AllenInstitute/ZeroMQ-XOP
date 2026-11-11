#include "ZeroMQ.h"
#include "FairMutex.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

void FairMutex::lock()
{
  std::unique_lock<std::mutex> lock(m_mutex);

  const auto self = std::this_thread::get_id();
  ASSERT(m_owner != self);

  const auto ticket = m_nextTicket++;
  m_cv.wait(lock, [this, ticket] { return m_servedTicket == ticket; });

  m_owner = self;
}

void FairMutex::unlock()
{
  {
    const LockGuard lock(m_mutex);

    m_owner = std::thread::id();
    m_servedTicket++;
  }

  m_cv.notify_all();
}
