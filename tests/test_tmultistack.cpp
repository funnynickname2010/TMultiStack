// tests/test_tmultistack.cpp
#include "../src/TMultiStack.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

//
// Tests for TMultiStack
//

TEST(TMultiStack, cannot_create_with_zero_stacks)
{
  EXPECT_THROW(TMultiStack<int> ms(0, 4), std::invalid_argument);
}

TEST(TMultiStack, default_construction_and_properties)
{
  ASSERT_NO_THROW(TMultiStack<int> ms(3, 2));
  TMultiStack<int> ms(3, 2);
  EXPECT_EQ(ms.GetNumStacks(), 3u);
  for (size_t i = 0; i < ms.GetNumStacks(); i++)
  {
    EXPECT_EQ(ms.GetStackSize(i), 0u);
    EXPECT_GE(ms.GetStackCapacity(i), 0u);
  }
}

TEST(TMultiStack, push_and_top_and_size_behavior)
{
  TMultiStack<int> ms(2, 1);

  ms.Push(0, 10);
  EXPECT_EQ(ms.GetStackSize(0), 1u);
  EXPECT_EQ(ms.Top(0), 10);

  ms.Push(0, 20);
  EXPECT_EQ(ms.GetStackSize(0), 2u);
  EXPECT_EQ(ms.Top(0), 20);

  ms.Push(1, 5);
  EXPECT_EQ(ms.GetStackSize(1), 1u);
  EXPECT_EQ(ms.Top(1), 5);
}

TEST(TMultiStack, push_triggers_capacity_growth)
{
  TMultiStack<int> ms(1, 1);
  size_t initialCap = ms.GetStackCapacity(0);
  EXPECT_GE(initialCap, 0u);

  // push many elements to force reallocation
  for (int i = 0; i < 100; i++)
  {
    ms.Push(0, i);
  }
  EXPECT_EQ(ms.GetStackSize(0), 100u);
  EXPECT_GE(ms.GetStackCapacity(0), ms.GetStackSize(0));
  EXPECT_EQ(ms.Top(0), 99);
}

TEST(TMultiStack, pop_and_top_and_underflow)
{
  TMultiStack<int> ms(1, 2);
  ms.Push(0, 1);
  ms.Push(0, 2);
  EXPECT_EQ(ms.Top(0), 2);

  // pop reduces size
  ms.Pop(0);
  EXPECT_EQ(ms.GetStackSize(0), 1u);
  EXPECT_EQ(ms.Top(0), 1);

  ms.Pop(0);
  EXPECT_EQ(ms.GetStackSize(0), 0u);
  EXPECT_THROW(ms.Top(0), std::underflow_error);

  // pop on empty should throw
  EXPECT_THROW(ms.Pop(0), std::underflow_error);
}

TEST(TMultiStack, invalid_index_throws)
{
  TMultiStack<int> ms(2, 2);
  EXPECT_THROW(ms.Push(5, 1), std::out_of_range);
  EXPECT_THROW(ms.Pop(3), std::out_of_range);
  EXPECT_THROW(ms.Top(10), std::out_of_range);
  EXPECT_THROW(ms.GetStackSize(7), std::out_of_range);
  EXPECT_THROW(ms.GetStackCapacity(7), std::out_of_range);
}

TEST(TMultiStack, reserve_stack_preserves_elements)
{
  TMultiStack<std::string> ms(1, 1);
  ms.Push(0, std::string("a"));
  ms.Push(0, std::string("b"));
  size_t beforeSize = ms.GetStackSize(0);
  ms.ReserveStack(0, 50);
  EXPECT_EQ(ms.GetStackSize(0), beforeSize);
  EXPECT_GE(ms.GetStackCapacity(0), 50u);
  EXPECT_EQ(ms.Top(0), "b");
}

TEST(TMultiStack, repack_all_shrink_to_fit_and_empty_free)
{
  TMultiStack<int> ms(3, 10);

  ms.Push(0, 1);
  ms.Push(0, 2);
  // stack 1 left empty
  ms.Push(2, 7);
  ms.Push(2, 8);
  ms.Push(2, 9);

  ms.RepackAll();

  // non-empty stacks capacity becomes equal to size
  EXPECT_EQ(ms.GetStackCapacity(0), ms.GetStackSize(0));
  EXPECT_EQ(ms.GetStackCapacity(2), ms.GetStackSize(2));
  // empty stack should have capacity 0
  EXPECT_EQ(ms.GetStackCapacity(1), 0u);
}

TEST(TMultiStack, repack_all_with_explicit_caps)
{
  TMultiStack<int> ms(2, 5);
  ms.Push(0, 1);
  ms.Push(0, 2);
  ms.Push(1, 3);

  // valid explicit capacities
  std::vector<size_t> caps = { 4, 10 };
  ASSERT_NO_THROW(ms.RepackAll(caps));
  EXPECT_EQ(ms.GetStackCapacity(0), 4u);
  EXPECT_EQ(ms.GetStackCapacity(1), 10u);

  // invalid: vector size mismatch
  std::vector<size_t> wrongCaps = { 1 };
  EXPECT_THROW(ms.RepackAll(wrongCaps), std::invalid_argument);

  // invalid: capacity smaller than current size
  std::vector<size_t> tooSmall = { 1, 0 }; // first < size 2
  EXPECT_THROW(ms.RepackAll(tooSmall), std::invalid_argument);
}

TEST(TMultiStack, add_and_remove_stack_behaviour)
{
  TMultiStack<int> ms(1, 1);
  EXPECT_EQ(ms.GetNumStacks(), 1u);

  ms.AddStack(3);
  EXPECT_EQ(ms.GetNumStacks(), 2u);
  EXPECT_EQ(ms.GetStackCapacity(1), 3u);

  // remove last when empty works
  ASSERT_NO_THROW(ms.RemoveLastStack());
  EXPECT_EQ(ms.GetNumStacks(), 1u);

  // remove shouldn't throw
  ms.Push(0, 10);
  ASSERT_NO_THROW(ms.RemoveLastStack());
}

TEST(TMultiStack, copy_constructor_deep_copy)
{
  TMultiStack<int> a(2, 2);
  a.Push(0, 1);
  a.Push(0, 2);
  a.Push(1, 10);

  TMultiStack<int> b(a); // copy
  EXPECT_EQ(b.GetNumStacks(), a.GetNumStacks());
  EXPECT_EQ(b.GetStackSize(0), a.GetStackSize(0));
  EXPECT_EQ(b.Top(0), a.Top(0));

  // modify copy and ensure original unchanged
  b.Pop(0);
  EXPECT_NE(b.GetStackSize(0), a.GetStackSize(0));
  EXPECT_EQ(a.GetStackSize(0), 2u);
  EXPECT_EQ(b.GetStackSize(0), 1u);
}

TEST(TMultiStack, move_constructor_leaves_source_cleared)
{
  TMultiStack<int> a(2, 2);
  a.Push(0, 42);
  a.Push(1, 99);

  TMultiStack<int> b(std::move(a));
  // moved object 'a' had its stacks moved-to b; our implementation clears source stacks vector
  EXPECT_EQ(b.GetNumStacks(), 2u);
  EXPECT_EQ(b.Top(0), 42);
  EXPECT_EQ(b.Top(1), 99);

  // source 'a' now should be empty (num stacks == 0)
  EXPECT_EQ(a.GetNumStacks(), 0u);
}

TEST(TMultiStack, clear_frees_all_and_getnumstacks_zero)
{
  TMultiStack<int> ms(2, 3);
  ms.Push(0, 1);
  ms.Push(1, 2);
  ms.Clear();
  EXPECT_EQ(ms.GetNumStacks(), 0u);
}

TEST(TMultiStack, top_returns_reference_and_allows_mutation)
{
  TMultiStack<int> ms(1, 2);
  ms.Push(0, 5);
  ms.Push(0, 6);
  int& ref = ms.Top(0);
  EXPECT_EQ(ref, 6);
  ref = 100;
  EXPECT_EQ(ms.Top(0), 100);
}

TEST(TMultiStack, push_rvalue_overload)
{
  TMultiStack<std::string> ms(1, 1);
  std::string s = "hello";
  ms.Push(0, std::move(s));
  EXPECT_EQ(ms.GetStackSize(0), 1u);
  EXPECT_EQ(ms.Top(0), "hello");
}

