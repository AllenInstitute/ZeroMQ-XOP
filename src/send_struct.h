#pragma once

#include <vector>
#include <string>
#include <optional>

#include "ZeroMQ.h"

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

class SendStorage
{
public:
  SendStorage(const void *ptr_param, size_t len_param)
      : ptr(ptr_param), len(len_param)
  {
  }

  SendStorage(std::string str) : storage(str)
  {
  }

  const void *GetPtr() const
  {
    if(storage.has_value())
    {
      return storage->c_str();
    }

    return ptr;
  }

  size_t GetLength() const
  {
    if(storage.has_value())
    {
      return storage->length();
    }

    return len;
  }

private:
  const void *ptr{nullptr};
  size_t len{0};
  std::optional<std::string> storage;
};

SendStorageVec GatherPubData(waveHndl containerWaveHandle);
void ConvertSubData(const ZeroMQMessageSharedPtrVec &vec,
                    waveHndl containerWaveHandle);
