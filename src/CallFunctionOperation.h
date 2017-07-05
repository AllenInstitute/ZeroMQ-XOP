#pragma once

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

class CallFunctionOperation
{
public:
  explicit CallFunctionOperation(json j);
  void CanBeProcessed() const;
  json Call() const;

private:
  friend std::ostream &operator<<(std::ostream &out, CallFunctionOperation op);

  std::string m_name;
  std::vector<std::string> m_params;
};

std::ostream &operator<<(std::ostream &out, CallFunctionOperation op);
std::ostream &operator<<(std::ostream &out, std::vector<std::string> vec);
