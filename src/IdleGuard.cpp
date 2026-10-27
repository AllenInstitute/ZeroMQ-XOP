#include "IdleGuard.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

bool idleInProgress = false;

} // anonymous namespace

IdleGuard::IdleGuard()
{
  idleInProgress = true;
}

IdleGuard::~IdleGuard()
{
  idleInProgress = false;
}

bool IsIdleInProgress()
{
  return idleInProgress;
}
