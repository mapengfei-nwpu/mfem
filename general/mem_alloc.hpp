// Copyright (c) 2010-2020, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-443211.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license.  We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#ifndef MFEM_MEM_ALLOC
#define MFEM_MEM_ALLOC

#include "../config/config.hpp"
#include <cstddef>

namespace mfem
{

template <class Elem, int Num>
class StackPart
{
public:
   StackPart<Elem, Num> *Prev;
   Elem Elements[Num];
};

template <class Elem, int Num>
class Stack
{
private:
   StackPart <Elem, Num> *TopPart, *TopFreePart;
   int UsedInTop, SSize;
public:
   Stack() { TopPart = TopFreePart = NULL; UsedInTop = Num; SSize = 0; }
   int Size() const { return SSize; }
   void Push (Elem E);
   Elem Pop();
   void Clear();
   size_t MemoryUsage() const;
   ~Stack() { Clear(); }
};

template <class Elem, int Num>
void Stack <Elem, Num>::Push (Elem E)
{
   StackPart <Elem, Num> *aux;
   if (UsedInTop == Num)
   {
      if (TopFreePart == NULL)
      {
         aux = new StackPart <Elem, Num>;
      }
      else
      {
         TopFreePart = (aux = TopFreePart)->Prev;
      }
      aux->Prev = TopPart;
      TopPart = aux;
      UsedInTop = 0;
   }
   TopPart->Elements[UsedInTop++] = E;
   SSize++;
}

template <class Elem, int Num>
Elem Stack <Elem, Num>::Pop()
{
   StackPart <Elem, Num> *aux;
   if (UsedInTop == 0)
   {
      TopPart = (aux = TopPart)->Prev;
      aux->Prev = TopFreePart;
      TopFreePart = aux;
      UsedInTop = Num;
   }
   SSize--;
   return TopPart->Elements[--UsedInTop];
}

template <class Elem, int Num>
void Stack <Elem, Num>::Clear()
{
   StackPart <Elem, Num> *aux;
   while (TopPart != NULL)
   {
      TopPart = (aux = TopPart)->Prev;
      delete aux;
   }
   while (TopFreePart != NULL)
   {
      TopFreePart = (aux = TopFreePart)->Prev;
      delete aux;
   }
   UsedInTop = Num;
   SSize = 0;
}

template <class Elem, int Num>
size_t Stack <Elem, Num>::MemoryUsage() const
{
   size_t used_mem = 0;
   StackPart <Elem, Num> *aux = TopPart;
   while (aux != NULL)
   {
      used_mem += sizeof(StackPart <Elem, Num>);
      aux = aux->Prev;
   }
   aux = TopFreePart;
   while (aux != NULL)
   {
      used_mem += sizeof(StackPart <Elem, Num>);
      aux = aux->Prev;
   }
   // Not counting sizeof(Stack <Elem, Num>)
   return used_mem;
}


template <class Elem, int Num>
class MemAllocNode
{
public:
   MemAllocNode <Elem, Num> *Prev;
   Elem Elements[Num];
};

template <class Elem, int Num>
class MemAlloc
{
private:
   MemAllocNode <Elem, Num> *Last;
   int AllocatedInLast;
   Stack <Elem *, Num> UsedMem;
public:
   MemAlloc() { Last = NULL; AllocatedInLast = Num; }
   Elem *Alloc();
   void Free (Elem *);
   void Clear();
   size_t MemoryUsage() const;
   ~MemAlloc() { Clear(); }
};

template <class Elem, int Num>
Elem *MemAlloc <Elem, Num>::Alloc()
{
   MemAllocNode <Elem, Num> *aux;
   if (UsedMem.Size() > 0)
   {
      return UsedMem.Pop();
   }
   if (AllocatedInLast == Num)
   {
      aux = Last;
      Last = new MemAllocNode <Elem, Num>;
      Last->Prev = aux;
      AllocatedInLast = 0;
   }
   return &(Last->Elements[AllocatedInLast++]);
}

template <class Elem, int Num>
void MemAlloc <Elem, Num>::Free (Elem *E)
{
   UsedMem.Push (E);
}

template <class Elem, int Num>
void MemAlloc <Elem, Num>::Clear()
{
   MemAllocNode <Elem, Num> *aux;
   while (Last != NULL)
   {
      aux = Last->Prev;
      delete Last;
      Last = aux;
   }
   AllocatedInLast = Num;
   UsedMem.Clear();
}

template <class Elem, int Num>
size_t MemAlloc <Elem, Num>::MemoryUsage() const
{
   size_t used_mem = UsedMem.MemoryUsage();
   MemAllocNode <Elem, Num> *aux = Last;
   while (aux != NULL)
   {
      used_mem += sizeof(MemAllocNode <Elem, Num>);
      aux = aux->Prev;
   }
   // Not counting sizeof(MemAlloc <Elem, Num>)
   return used_mem;
}

}

#endif
