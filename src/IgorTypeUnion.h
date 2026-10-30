#pragma once

#include "XOPStandardHeaders.h" // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#include <vector>

// This file is part of the `ZeroMQ-XOP` project and licensed under
// BSD-3-Clause.

#pragma pack(2) // All structures passed to Igor are two-byte aligned.
union IgorTypeUnion
{
  waveHndl waveHandle;
  double variable;
  Handle stringHandle;
  DataFolderHandle dataFolderHandle;
};
#pragma pack()

using IgorTypeUnionVector = std::vector<IgorTypeUnion>;
