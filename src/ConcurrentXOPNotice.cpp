#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

namespace
{

void OutputToHistory(std::string str, ExperimentModification mode)
{
  if(str.empty())
  {
    return;
  }

  XOPNotice2(str.append(CR_STR).c_str(),
             mode == ExperimentModification::Normal ? 0x1 : 0);
}

} // anonymous namespace

struct OutputMessage
{
  OutputMessage(std::string msg_, ExperimentModification mode_)
      : msg(std::move(msg_)), mode(mode_)
  {
  }

  std::string msg;
  ExperimentModification mode;
};

void OutputToHistory_TS(std::string str, ExperimentModification mode)
{
  if(RunningInMainThread())
  {
    OutputQueuedNotices();
    OutputToHistory(str, mode);
    return;
  }

  GlobalData::Instance().GetXOPNoticeQueue().push(
      std::make_shared<OutputMessage>(str, mode));
}

void OutputQueuedNotices()
{
  if(!RunningInMainThread())
  {
    return;
  }

  GlobalData::Instance().GetXOPNoticeQueue().apply_to_all(
      [](const OutputMessagePtr &om) { OutputToHistory(om->msg, om->mode); });
}
