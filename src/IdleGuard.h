#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief RAII guard preventing reentrant IDLE processing in XOPEntry (see
///        ZeroMQ.cpp)
///
/// Marks IDLE processing as in-progress for its lifetime and unconditionally
/// clears that again on destruction -- including when unwinding due to an
/// exception -- so a failure inside the guarded block can never leave IDLE
/// processing permanently disabled. Query the current state via
/// IsIdleInProgress().
class IdleGuard
{
public:
  IdleGuard();
  ~IdleGuard();

  IdleGuard(const IdleGuard &)            = delete;
  IdleGuard &operator=(const IdleGuard &) = delete;
};

bool IsIdleInProgress();
