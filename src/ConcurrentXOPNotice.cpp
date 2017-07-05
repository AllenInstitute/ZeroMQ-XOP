#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

void XOPNotice_ts(std::string str)
{
  if(RunningInMainThread())
  {
    OutputQueuedNotices();
    XOPNotice(str.c_str());
    return;
  }

  GlobalData::Instance().GetXOPNoticeQueue().push(str);
}

void XOPNotice_ts(const char *noticeStr)
{
  XOPNotice_ts(std::string(noticeStr));
}

void OutputQueuedNotices()
{
  if(!RunningInMainThread())
  {
    return;
  }

  GlobalData::Instance().GetXOPNoticeQueue().apply_to_all(
      [](std::string str) { XOPNotice(str.c_str()); });
}
