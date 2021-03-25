#include "HistoryGrabber.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

HistoryGrabber::HistoryGrabber() : m_startLine(HistoryLines())
{
}

std::string HistoryGrabber::GetHistoryUntilNow() const
{
  const auto endLine = HistoryLines();

  DebugOutput(fmt::format("{}: called\r", __func__));

  TULoc startLoc{};
  TULoc endLoc{};

  // -1 because HistoryLines() points
  // after the last paragraph with data
  startLoc.paragraph = m_startLine - 1;
  endLoc.paragraph   = endLine - 1;

  Handle historyHandle = WMNewHandle(0);
  ASSERT(historyHandle != nullptr);
  int rc = HistoryFetchText(&startLoc, &endLoc, &historyHandle);

  if(rc)
  {
    WMDisposeHandle(historyHandle);

    DebugOutput(
        fmt::format("{}: HistoryFetchText returned {}\r", __func__, rc));
    return {};
  }

  auto history = GetStringFromHandle(historyHandle);
  WMDisposeHandle(historyHandle);

  DebugOutput(fmt::format("{}: History ={}\r", __func__, history));

  return CleanupString(history);
}
