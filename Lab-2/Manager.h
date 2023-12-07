#ifndef MANAGER_H
#define MANAGER_H

#include <mpi.h>
#include <stdio.h>
#include <utility>
#include <cassert>
#include <algorithm>
#include <string>
#include <optional>
#include <vector>

#define DEFAULT_TAG 2

#define DEBUG_SEND

using PID = unsigned;

template <typename Iterator>
using GetTFromIt = typename std::iterator_traits<Iterator>::value_type;

enum class MPIGroup {
  World
};

MPI_Comm getGroup(MPIGroup Group) {
  switch (Group) {
  case MPIGroup::World:
    return MPI_COMM_WORLD;
  default:
    assert(false && "Group is unsupported");
  }
} 


class MPIManager  {

public: 

  MPIManager() {
    MPI_Init(nullptr, nullptr);
  }

  MPIManager(const MPIManager&) = delete;
  MPIManager(MPIManager&&) = default;

  ~MPIManager() {
    MPI_Finalize();
  }

  size_t getMPIGroupSize(MPI_Comm Group = getGroup(MPIGroup::World)) const {
    auto Rank = 0;
    MPI_Comm_size(Group, &Rank);
    return Rank;
  }

  PID getPID(MPI_Comm Group = getGroup(MPIGroup::World)) const{
    auto Size = 0;
    MPI_Comm_rank(Group, &Size);
    return Size;
  }

  double time() const {
    return MPI_Wtime();
  }

  template <typename ItType>
  void send(ItType MsgBeg, ItType MsgEnd, PID Dest, int MsgTag = DEFAULT_TAG, 
            MPI_Comm Group = getGroup(MPIGroup::World)) const;

  template <typename T>
  void send(T Msg, PID Dest, int MsgTag = DEFAULT_TAG, 
            MPI_Comm Group = getGroup(MPIGroup::World)) const {
    auto Size = sizeof(T);
    auto Ret = MPI_Send(reinterpret_cast<char*>(&Msg), Size, MPI_BYTE, Dest,
                        MsgTag, Group); //0 - OK
  }

  template <typename T>
  void sendContainer(T Msg, PID Dest, int MsgTag = DEFAULT_TAG, 
                     MPI_Comm Group = getGroup(MPIGroup::World)) const {
    auto DataSize = Msg.size() * sizeof(typename T::value_type);
    send(DataSize, Dest, MsgTag, Group);
    auto Ret = MPI_Ssend(Msg.data(), DataSize, MPI_BYTE, Dest,
                        MsgTag, Group);
  } 

  template <typename T>
  T recv(PID Src, int MsgTag = DEFAULT_TAG,
         MPI_Comm Group = getGroup(MPIGroup::World)) const {
    auto Size = sizeof(T);
    char Buf[Size] = {};
    MPI_Status Status;
    auto Ret = MPI_Recv(Buf, Size, MPI_BYTE, Src,
                        MsgTag, Group, &Status);
    auto Res = *reinterpret_cast<T*>(Buf);
    return Res;
  }

  template <typename T>
  auto recvVector(PID Src, int MsgTag = DEFAULT_TAG,
                     MPI_Comm Group = getGroup(MPIGroup::World)) const {
    auto DataSize = recv<size_t>(Src, MsgTag, Group);
    auto Size = DataSize / sizeof(T);
    std::vector<T> RetBuf(Size);
    MPI_Status Status;
    auto Ret = MPI_Recv(RetBuf.data(), DataSize, MPI_BYTE, Src,
                        MsgTag, Group, &Status); 
    return RetBuf;
  }

  template <typename T>
  std::optional<T> reduce(T Msg, PID Dest, 
                          void (*Op)(void*, void*, int*, MPI_Datatype*),
                          MPI_Comm Group = getGroup(MPIGroup::World)) {
    MPI_Op NewOp;
    MPI_Op_create(Op, true, &NewOp);
    auto Size = sizeof(T);
    auto RecvBuf = new char[Size];
    MPI_Reduce(reinterpret_cast<char*>(&Msg), RecvBuf, Size, MPI_BYTE, 
               NewOp, Dest, Group);
    if (getPID(Group) == Dest) {
      auto Res = *reinterpret_cast<T*>(RecvBuf);
      delete[] RecvBuf;
      return Res;
    } 
    delete[] RecvBuf;
    return {};
  }
};

template <typename ItType>
void MPIManager::send(ItType MsgBeg, ItType MsgEnd, PID Dest, int MsgTag, 
                      MPI_Comm Group) const {
  std::vector<GetTFromIt<ItType>> Buf{MsgBeg, MsgEnd};
  sendContainer(Buf, Dest, MsgTag, Group);
}

template <>
void MPIManager::send(std::string Msg, PID Dest, int MsgTag, 
          MPI_Comm Group) const {
  send(Msg.length() + 1, Dest, MsgTag, Group);
  auto Ret = MPI_Ssend(Msg.c_str(), Msg.length() + 1, MPI_BYTE, Dest,
                      MsgTag, Group);
}

template <>
std::string MPIManager::recv(PID Src, int MsgTag,
        MPI_Comm Group) const {
  auto Size = recv<decltype(std::string{}.length())>(Src, MsgTag, Group);
  auto Buf = new char[Size + 1];
  MPI_Status Status;
  auto Ret = MPI_Recv(Buf, Size, MPI_BYTE, Src,
                      MsgTag, Group, &Status);
  std::string Res{Buf};
  delete[] Buf;
  return Res;
}

#endif