#pragma once

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// @brief Marks the IDLE processing in XOPEntry as in progress for its lifetime
class IdleGuard
{
public:
  IdleGuard();
  ~IdleGuard();

  IdleGuard(const IdleGuard &)            = delete;
  IdleGuard &operator=(const IdleGuard &) = delete;
  IdleGuard(IdleGuard &&)                 = delete;
  IdleGuard &operator=(IdleGuard &&)      = delete;
};

bool IsIdleInProgress();
