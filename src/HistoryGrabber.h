#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

// Class for fetching the history between two time points
class HistoryGrabber
{
public:
  HistoryGrabber();
  std::string GetHistoryUntilNow() const;

private:
  int m_startLine{0};
};
