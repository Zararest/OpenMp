#pragma once

#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#endif

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_ENABLE_EXCEPTIONS

#include "../OpenCL/opencl.hpp"

#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

struct Config {
  size_t Size = 1048576;
  size_t ThreadsNum = 128;
  size_t MaxLocalMemSize = 32;
  size_t ChunkSize = 131072;
  std::string DataType = "int";

  using T = int;
};

class Driver {
  Config Cfg;
  std::string Kernel;

  cl::Platform Platform;
  cl::Context  Ctx;
  cl::CommandQueue Queue;

  static void reportFatalError(const std::string &Msg) {
    std::cerr << "Error: " << Msg << std::endl;
    exit(-1);
  }

  cl::Platform getGPUPlatform() {
    auto Platforms = cl::vector<cl::Platform>{};
    cl::Platform::get(&Platforms);

    for (auto P : Platforms) {
      auto DeviceNum = 0u;
      ::clGetDeviceIDs(P(), CL_DEVICE_TYPE_GPU, 0, NULL, &DeviceNum);

      if (DeviceNum > 0)
        return cl::Platform(P);
    }
    reportFatalError("Can't find GPU platform");
    return *Platforms.begin();
  }

  cl::Context getGPUContext(cl_platform_id Platform) {
    cl_context_properties Properties[] 
      = {CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(Platform), 0};
    
    return cl::Context(CL_DEVICE_TYPE_GPU, Properties);
  }

  template <typename It>
  void callKernel(size_t ChunkSize, It Beg, It End) {
    auto Size = std::distance(Beg, End);
    auto CLIn = cl::Buffer(Ctx, CL_MEM_READ_WRITE, Size * sizeof(Config::T));
    auto CLOut = cl::Buffer(Ctx, CL_MEM_READ_WRITE, Size * sizeof(Config::T));
    cl::copy(Queue, Beg, End, CLIn);

    auto Program = cl::Program(Ctx, Kernel, /*build*/ true);
    
    auto KernelStart = 
      cl::KernelFunctor<cl::Buffer, cl::Buffer, 
                        unsigned, unsigned>(Program, "mergeChunks");

    auto ProgramArgs = 
      cl::EnqueueArgs{Queue, cl::NDRange{Cfg.ThreadsNum}, cl::NDRange{Cfg.ThreadsNum}};

    cl::Event KernelRes = KernelStart(ProgramArgs, CLIn, CLOut, Size, ChunkSize);
    KernelRes.wait();

    cl::copy(Queue, CLOut, Beg, End);
  } 

  template <typename It>
  void makeMerge(size_t ChunkSize, It Beg, It End) {
    assert(std::distance(Beg, End));
    auto Size = static_cast<size_t>(std::distance(Beg, End));
    auto Buf = std::vector<int>{};
    auto Pos = 0ul;

    for (; Pos + ChunkSize < Size; Pos += ChunkSize * 2) {
      auto RhsPos = Pos + ChunkSize;
      auto LhsIt = Beg + Pos;
      auto RhsIt = Beg + std::min(Pos + ChunkSize, Size);
      auto RhsSize = std::min(ChunkSize, Size - Pos - ChunkSize);
      std::merge(LhsIt, LhsIt + ChunkSize, 
                 RhsIt, RhsIt + RhsSize, 
                 std::back_inserter(Buf));
    }
    if (Pos < Size)
      std::copy(Beg + Pos, End, std::back_inserter(Buf));
    assert(Buf.size() == Size);
    std::copy(Buf.begin(), Buf.end(), Beg);
  }

  template <typename It>
  void sortOnCPU(size_t ChunkSize, It Beg, It End) {
    auto Size = std::distance(Beg, End);
    auto CurChunkSize = ChunkSize;

    while (CurChunkSize < Size) {
      makeMerge(CurChunkSize, Beg, End);
      CurChunkSize *= 2;
    }
  }

public:
  Driver(Config Cfg) : Cfg{Cfg}, Platform{getGPUPlatform()}, 
                       Ctx{getGPUContext(Platform())},
                       Queue{Ctx, cl::QueueProperties::Profiling | 
                                  cl::QueueProperties::OutOfOrder} {}

  void loadKernel(const std::string &KernelPath) {
    auto KernelFile = std::ifstream{KernelPath};

    if (!KernelFile.is_open())
      reportFatalError("Can't open kernel file");

    auto KernelStream = std::stringstream{};
    KernelStream << "#define NEW_DATA\n";
    KernelStream << "#define LOCAL_SIZE " << std::to_string(Cfg.MaxLocalMemSize) << "\n";
    KernelStream << "#define T " << Cfg.DataType << "\n";
    KernelStream << "#define CMP <\n"; 

    KernelStream << KernelFile.rdbuf();
    
    Kernel = KernelStream.str();
  }

  // iterators are mutable
  template <typename It>
  void sort(It Beg, It End) {
    auto Size = std::distance(Beg, End);
    assert(Size > 0);
    auto CurChunkSize = 1ull;
    auto ChunkSizeThreshold = std::min(static_cast<size_t>(Size), Cfg.ChunkSize);

    while (CurChunkSize < ChunkSizeThreshold) {
      try {
        callKernel(CurChunkSize, Beg, End);
      } catch(cl::Error& Err) {
        std::cerr << "OpenCl:" << Err.err() << ":" << Err.what() << std::endl;
        reportFatalError("Internal error\n");
      }
      CurChunkSize *= 2;
    }  
    sortOnCPU(CurChunkSize, Beg, End);
  }
};