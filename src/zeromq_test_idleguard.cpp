#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

/// Marker exception used only to exercise IdleGuard's unwind path without
/// depending on some other subsystem actually failing.
class IdleGuardTestException : public std::exception
{
};

} // anonymous namespace

// variable zeromq_test_idleguard()
//
// Regression test for the IdleGuard fix
extern "C" int zeromq_test_idleguard(zeromq_test_idleguardParams *p)
{
  BEGIN_OUTER_CATCH

  ASSERT(!IsIdleInProgress());

  try
  {
    IdleGuard guard;
    throw IdleGuardTestException{};
  }
  catch(const IdleGuardTestException &)
  {
    // expected
  }

  if(IsIdleInProgress())
  {
    throw IgorException(IDLE_GUARD_NOT_RESET);
  }

  END_OUTER_CATCH
}
