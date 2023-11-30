#ifndef NEW_DATA
  #define T int
  #define LOCAL_SIZE 256
  #define CMP <
#endif

#define size_t unsigned long long

void copy(__global T *From, size_t Size, __global T *To) {
  for (size_t i = 0; i < Size; ++i)
    To[i] = From[i];
}

void mergeGlobalMem(__global T *Lhs, __global T *Rhs, 
                    size_t LhsSize, size_t RhsSize, 
                    __global T *To) {
  size_t Ri = 0;
  size_t ToPos = 0;
  for (size_t Li = 0; Li < LhsSize; ++ToPos) {
    if (Ri == RhsSize) {
      copy(Lhs + Li, LhsSize - Li, To + ToPos);
      return;
    }

    if (Lhs[Li] CMP Rhs[Ri]) {
      To[ToPos] = Lhs[Li];
      ++Li;
    } else {
      To[ToPos] = Rhs[Ri];
      ++Ri;
    }
  }
  copy(Rhs + Ri, RhsSize - Ri, To + ToPos);
}

void mergeIteration(__global T *From, __global T *To, size_t Size, size_t ChunkSize) {
  if (ChunkSize >= Size) {
    copy(From, Size, To);
    return;
  }

  size_t TailSize = Size - ChunkSize;
  size_t RhsSize =  TailSize > ChunkSize ? ChunkSize : TailSize;
  mergeGlobalMem(From, From + ChunkSize, ChunkSize, RhsSize, To);
}

// Size may be not a power of 2
__kernel void mergeChunks(__global T *Arr, __global T *To, unsigned Size, unsigned ChunkSize) {
  size_t Id = get_global_id(0);
  size_t SizePerIter = get_global_size(0) * ChunkSize * 2; 
  size_t ThreadPos = ChunkSize * 2 * Id;

  for (int CurStart = 0; CurStart < Size; CurStart += SizePerIter) {
    if (CurStart + ThreadPos >= Size)
      return;
    mergeIteration(Arr + CurStart + ThreadPos, To + CurStart + ThreadPos, Size - CurStart, ChunkSize);
  }
}