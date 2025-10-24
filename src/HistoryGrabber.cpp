#include "HistoryGrabber.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

HistoryGrabber::HistoryGrabber() : m_startLine(HistoryLines())
{
}

std::string HistoryGrabber::GetHistoryUntilNow() const
{
  const auto endLine = HistoryLines();

  TULoc startLoc{};
  TULoc endLoc{};

  // -1 because HistoryLines() points
  // after the last paragraph with data
  startLoc.paragraph = std::max(m_startLine - 1, 0);
  endLoc.paragraph   = std::max(endLine - 1, 0);

  Handle historyHandle = WMNewHandle(0);
  ASSERT(historyHandle != nullptr);
  int rc = HistoryFetchText(&startLoc, &endLoc, &historyHandle);

  if(rc)
  {
    WMDisposeHandle(historyHandle);

    DEBUG_OUTPUT("HistoryFetchText returned {}", rc);
    return {};
  }

  auto history = GetStringFromHandle(historyHandle);
  WMDisposeHandle(historyHandle);

  DEBUG_OUTPUT("History ={}", history);

  return CleanupString(history);
}
