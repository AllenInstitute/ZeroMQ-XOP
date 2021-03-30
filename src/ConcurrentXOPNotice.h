#pragma once

#include <string>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

/// Thread safe version of XOPNotice, even for private threads
///
/// Remarks:
/// - All threads, also the main thread, must use
///   XOPNotice_ts() so that the messages are ordered
/// - Output to the history should be done at the IDLE event via
///   OutputQueuedNotices(). For that the XOP has to be marked as
///   `SetXOPType(RESIDENT | IDLES);`.

enum class ExperimentModification
{
  Normal, ///< Mark experiment as modified
  Silent  ///< Don't mark the experiment as modified
};

struct OutputMessage;
using OutputMessagePtr = std::shared_ptr<OutputMessage>;

/// Threadsafe output to history
void OutputToHistory_TS(std::string str, ExperimentModification mode);

void OutputQueuedNotices();
