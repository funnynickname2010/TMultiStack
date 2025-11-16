#pragma once
#include <cstddef>
#include <vector>
#include <stdexcept>

template <typename T>
class TMultiStack
{
private:
  struct StackInfo
  {
    T* data;
    size_t size;
    size_t capacity;

    StackInfo();
    void Alloc(size_t cap);
    void Free();
  };

  std::vector<StackInfo> stacks;

  static void ThrowIndex(size_t idx, size_t total);
  void EnsureCapacity(size_t idx, size_t minCap);

public:
  explicit TMultiStack(size_t numStacks = 1, size_t initCap = 0);
  TMultiStack(const TMultiStack& other);
  TMultiStack(TMultiStack&& other) noexcept;

  TMultiStack& operator=(const TMultiStack& other);
  TMultiStack& operator=(TMultiStack&& other) noexcept;

  ~TMultiStack();

  size_t GetNumStacks() const noexcept;
  size_t GetStackSize(size_t idx) const;
  size_t GetStackCapacity(size_t idx) const;

  void Push(size_t idx, const T& value);
  void Push(size_t idx, T&& value);
  void Pop(size_t idx);

  T& Top(size_t idx);
  const T& Top(size_t idx) const;

  void ReserveStack(size_t idx, size_t newCapacity);

  void RepackAll();
  void RepackAll(const std::vector<size_t>& newCaps);

  void AddStack(size_t initialCapacity = 0);
  void RemoveLastStack();

  void Clear() noexcept;
};

//
// ===== StackInfo methods =====
//

template <typename T>
TMultiStack<T>::StackInfo::StackInfo()
  : data(nullptr),
  size(0),
  capacity(0)
{
}

template <typename T>
void TMultiStack<T>::StackInfo::Alloc(size_t cap)
{
  data = nullptr;
  size = 0;
  capacity = 0;

  if (cap > 0)
  {
    data = new T[cap];
    capacity = cap;
  }
}

template <typename T>
void TMultiStack<T>::StackInfo::Free()
{
  if (data != nullptr)
  {
    delete[] data;
    data = nullptr;
  }

  size = 0;
  capacity = 0;
}

//
// ===== Private helpers =====
//

template <typename T>
void TMultiStack<T>::ThrowIndex(size_t idx, size_t total)
{
  throw std::out_of_range("Stack index out of range: " + std::to_string(idx) +
    ", total=" + std::to_string(total));
}

template <typename T>
void TMultiStack<T>::EnsureCapacity(size_t idx, size_t minCap)
{
  StackInfo& s = stacks[idx];

  if (s.capacity >= minCap)
  {
    return;
  }

  size_t newCap = (s.capacity == 0 ? 1 : s.capacity);
  while (newCap < minCap)
  {
    newCap *= 2;
  }

  T* newData = new T[newCap];

  for (size_t i = 0; i < s.size; i++)
  {
    newData[i] = std::move(s.data[i]);
  }

  delete[] s.data;

  s.data = newData;
  s.capacity = newCap;
}

//
// ===== Constructors, destructor, assignment =====
//

template <typename T>
TMultiStack<T>::TMultiStack(size_t numStacks, size_t initCap)
{
  stacks.resize(numStacks);

  if (numStacks == 0)
    throw std::invalid_argument("Number of stacks cannot be zero");

  for (size_t i = 0; i < numStacks; i++)
  {
    stacks[i].Alloc(initCap);
  }
}

template <typename T>
TMultiStack<T>::TMultiStack(const TMultiStack& other)
{
  stacks.resize(other.stacks.size());

  for (size_t i = 0; i < stacks.size(); i++)
  {
    const StackInfo& src = other.stacks[i];
    StackInfo& dst = stacks[i];

    dst.Alloc(src.capacity);

    for (size_t j = 0; j < src.size; j++)
    {
      dst.data[j] = src.data[j];
    }

    dst.size = src.size;
  }
}

template <typename T>
TMultiStack<T>::TMultiStack(TMultiStack&& other) noexcept
  : stacks(std::move(other.stacks))
{
  other.stacks.clear();
}

template <typename T>
TMultiStack<T>& TMultiStack<T>::operator=(const TMultiStack& other)
{
  if (this != &other)
  {
    Clear();
    stacks.resize(other.stacks.size());

    for (size_t i = 0; i < stacks.size(); i++)
    {
      const StackInfo& src = other.stacks[i];
      StackInfo& dst = stacks[i];

      dst.Alloc(src.capacity);

      for (size_t j = 0; j < src.size; j++)
      {
        dst.data[j] = src.data[j];
      }

      dst.size = src.size;
    }
  }

  return *this;
}

template <typename T>
TMultiStack<T>& TMultiStack<T>::operator=(TMultiStack&& other) noexcept
{
  if (this != &other)
  {
    Clear();
    stacks = std::move(other.stacks);
    other.stacks.clear();
  }

  return *this;
}

template <typename T>
TMultiStack<T>::~TMultiStack()
{
  Clear();
}

//
// ===== Basic queries =====
//

template <typename T>
size_t TMultiStack<T>::GetNumStacks() const noexcept
{
  return stacks.size();
}

template <typename T>
size_t TMultiStack<T>::GetStackSize(size_t idx) const
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  return stacks[idx].size;
}

template <typename T>
size_t TMultiStack<T>::GetStackCapacity(size_t idx) const
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  return stacks[idx].capacity;
}

//
// ===== Stack operations =====
//

template <typename T>
void TMultiStack<T>::Push(size_t idx, const T& value)
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  StackInfo& s = stacks[idx];

  EnsureCapacity(idx, s.size + 1);

  s.data[s.size] = value;
  s.size += 1;
}

template <typename T>
void TMultiStack<T>::Push(size_t idx, T&& value)
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  StackInfo& s = stacks[idx];

  EnsureCapacity(idx, s.size + 1);

  s.data[s.size] = std::move(value);
  s.size += 1;
}

template <typename T>
void TMultiStack<T>::Pop(size_t idx)
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  StackInfo& s = stacks[idx];

  if (s.size == 0)
  {
    throw std::underflow_error("Pop from empty stack");
  }

  s.size -= 1;
}

template <typename T>
T& TMultiStack<T>::Top(size_t idx)
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  StackInfo& s = stacks[idx];

  if (s.size == 0)
  {
    throw std::underflow_error("Top on empty stack");
  }

  return s.data[s.size - 1];
}

template <typename T>
const T& TMultiStack<T>::Top(size_t idx) const
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  const StackInfo& s = stacks[idx];

  if (s.size == 0)
  {
    throw std::underflow_error("Top on empty stack");
  }

  return s.data[s.size - 1];
}

//
// ===== Reallocation helpers =====
//

template <typename T>
void TMultiStack<T>::ReserveStack(size_t idx, size_t newCapacity)
{
  if (idx >= stacks.size())
  {
    ThrowIndex(idx, stacks.size());
  }

  StackInfo& s = stacks[idx];

  if (newCapacity <= s.capacity)
  {
    return;
  }

  T* newData = new T[newCapacity];

  for (size_t i = 0; i < s.size; i++)
  {
    newData[i] = std::move(s.data[i]);
  }

  delete[] s.data;

  s.data = newData;
  s.capacity = newCapacity;
}

template <typename T>
void TMultiStack<T>::RepackAll()
{
  for (StackInfo& s : stacks)
  {
    if (s.capacity == s.size)
    {
      continue;
    }

    T* newData = nullptr;

    if (s.size > 0)
    {
      newData = new T[s.size];

      for (size_t i = 0; i < s.size; i++)
      {
        newData[i] = std::move(s.data[i]);
      }
    }

    delete[] s.data;

    s.data = newData;
    s.capacity = s.size;
  }
}

template <typename T>
void TMultiStack<T>::RepackAll(const std::vector<size_t>& newCaps)
{
  if (newCaps.size() != stacks.size())
  {
    throw std::invalid_argument("RepackAll: newCaps must match stack count");
  }

  for (size_t i = 0; i < stacks.size(); i++)
  {
    StackInfo& s = stacks[i];
    size_t newCap = newCaps[i];

    if (newCap < s.size)
    {
      throw std::invalid_argument("RepackAll: new capacity smaller than size");
    }

    if (newCap == s.capacity)
    {
      continue;
    }

    T* newData = nullptr;

    if (newCap > 0)
    {
      newData = new T[newCap];

      for (size_t j = 0; j < s.size; j++)
      {
        newData[j] = std::move(s.data[j]);
      }
    }

    delete[] s.data;

    s.data = newData;
    s.capacity = newCap;
  }
}

//
// ===== Stack management =====
//

template <typename T>
void TMultiStack<T>::AddStack(size_t initialCapacity)
{
  StackInfo node;
  node.Alloc(initialCapacity);
  stacks.push_back(std::move(node));
}

template <typename T>
void TMultiStack<T>::RemoveLastStack()
{
  if (stacks.empty())
  {
    throw std::out_of_range("RemoveLastStack: no stacks to remove");
  }

  StackInfo& s = stacks.back();
  s.Free();
  stacks.pop_back();
}

template <typename T>
void TMultiStack<T>::Clear() noexcept
{
  for (StackInfo& s : stacks)
  {
    s.Free();
  }

  stacks.clear();
}

//
// Explicit instantiation (optional)
// Remove or modify depending on your usage.
//

template class TMultiStack<int>;
template class TMultiStack<double>;
template class TMultiStack<std::string>;

