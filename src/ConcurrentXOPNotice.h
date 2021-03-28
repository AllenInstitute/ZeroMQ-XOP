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

void XOPNotice_ts(const std::string &str);
void XOPNotice_ts(const char *noticeStr);

void OutputQueuedNotices();
