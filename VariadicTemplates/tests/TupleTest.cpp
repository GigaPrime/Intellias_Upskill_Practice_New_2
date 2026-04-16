#include "../src/Tuple.h"

#include "gtest/gtest.h"

namespace VT
{
	// Construction tests
	TEST(TupleTest, DefaultConstruction)
	{
		Tuple<int, double, bool> t;
		// Default construction doesn't zero-initialize for primitive types
		// Just verify the tuple was constructed without error
		EXPECT_TRUE(true);
	}

	TEST(TupleTest, ParameterizedConstruction)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		EXPECT_EQ(get<0>(t), 42);
		EXPECT_EQ(get<1>(t), 3.14);
		EXPECT_EQ(get<2>(t), true);
	}

	TEST(TupleTest, SingleElementTuple)
	{
		Tuple<int> t(99);
		EXPECT_EQ(get<0>(t), 99);
	}

	TEST(TupleTest, EmptyTuple)
	{
		EXPECT_NO_THROW({
			Tuple<> t;
		});
	}

	TEST(TupleTest, CopyConstruction)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(t1);
		EXPECT_EQ(get<0>(t2), 42);
		EXPECT_EQ(get<1>(t2), 3.14);
		EXPECT_EQ(get<2>(t2), true);
	}

	TEST(TupleTest, MoveConstruction)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(std::move(t1));
		EXPECT_EQ(get<0>(t2), 42);
		EXPECT_EQ(get<1>(t2), 3.14);
		EXPECT_EQ(get<2>(t2), true);
	}

	// Assignment tests
	TEST(TupleTest, CopyAssignment)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(10, 2.71, false);
		t2 = t1;
		EXPECT_EQ(get<0>(t2), 42);
		EXPECT_EQ(get<1>(t2), 3.14);
		EXPECT_EQ(get<2>(t2), true);
	}

	TEST(TupleTest, MoveAssignment)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(10, 2.71, false);
		t2 = std::move(t1);
		EXPECT_EQ(get<0>(t2), 42);
		EXPECT_EQ(get<1>(t2), 3.14);
		EXPECT_EQ(get<2>(t2), true);
	}

	TEST(TupleTest, SelfAssignmentProtection)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		t = t;
		EXPECT_EQ(get<0>(t), 42);
		EXPECT_EQ(get<1>(t), 3.14);
		EXPECT_EQ(get<2>(t), true);
	}

	// Equality comparison tests
	TEST(TupleTest, EqualityOperator)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 == t2);
	}

	TEST(TupleTest, EqualityOperatorDifferentHead)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(99, 3.14, true);
		EXPECT_FALSE(t1 == t2);
	}

	TEST(TupleTest, EqualityOperatorDifferentTail)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 2.71, true);
		EXPECT_FALSE(t1 == t2);
	}

	TEST(TupleTest, InequalityOperator)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(99, 2.71, false);
		EXPECT_TRUE(t1 != t2);
	}

	TEST(TupleTest, InequalityOperatorSame)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_FALSE(t1 != t2);
	}

	// Less than operator tests
	TEST(TupleTest, LessThanHeadDifference)
	{
		Tuple<int, double, bool> t1(10, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 < t2);
		EXPECT_FALSE(t2 < t1);
	}

	TEST(TupleTest, LessThanTailDifference)
	{
		Tuple<int, double, bool> t1(42, 2.71, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 < t2);
		EXPECT_FALSE(t2 < t1);
	}

	TEST(TupleTest, LessThanSame)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_FALSE(t1 < t2);
	}

	// Greater than operator tests
	TEST(TupleTest, GreaterThanHeadDifference)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(10, 3.14, true);
		EXPECT_TRUE(t1 > t2);
		EXPECT_FALSE(t2 > t1);
	}

	TEST(TupleTest, GreaterThanTailDifference)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 2.71, true);
		EXPECT_TRUE(t1 > t2);
		EXPECT_FALSE(t2 > t1);
	}

	TEST(TupleTest, GreaterThanSame)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_FALSE(t1 > t2);
	}

	// Less than or equal operator tests
	TEST(TupleTest, LessThanOrEqualLesser)
	{
		Tuple<int, double, bool> t1(10, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 <= t2);
	}

	TEST(TupleTest, LessThanOrEqualEqual)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 <= t2);
	}

	TEST(TupleTest, LessThanOrEqualGreater)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(10, 3.14, true);
		EXPECT_FALSE(t1 <= t2);
	}

	// Greater than or equal operator tests
	TEST(TupleTest, GreaterThanOrEqualGreater)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(10, 3.14, true);
		EXPECT_TRUE(t1 >= t2);
	}

	TEST(TupleTest, GreaterThanOrEqualEqual)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_TRUE(t1 >= t2);
	}

	TEST(TupleTest, GreaterThanOrEqualLesser)
	{
		Tuple<int, double, bool> t1(10, 3.14, true);
		Tuple<int, double, bool> t2(42, 3.14, true);
		EXPECT_FALSE(t1 >= t2);
	}

	// Swap tests
	TEST(TupleTest, Swap)
	{
		Tuple<int, double, bool> t1(42, 3.14, true);
		Tuple<int, double, bool> t2(99, 2.71, false);
		t1.swap(t2);
		EXPECT_EQ(get<0>(t1), 99);
		EXPECT_EQ(get<1>(t1), 2.71);
		EXPECT_EQ(get<2>(t1), false);
		EXPECT_EQ(get<0>(t2), 42);
		EXPECT_EQ(get<1>(t2), 3.14);
		EXPECT_EQ(get<2>(t2), true);
	}

	// Get function tests
	TEST(TupleTest, GetFirstElement)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		EXPECT_EQ(get<0>(t), 42);
	}

	TEST(TupleTest, GetSecondElement)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		EXPECT_EQ(get<1>(t), 3.14);
	}

	TEST(TupleTest, GetThirdElement)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		EXPECT_EQ(get<2>(t), true);
	}

	TEST(TupleTest, GetConstTuple)
	{
		const Tuple<int, double, bool> t(42, 3.14, true);
		EXPECT_EQ(get<0>(t), 42);
		EXPECT_EQ(get<1>(t), 3.14);
		EXPECT_EQ(get<2>(t), true);
	}

	TEST(TupleTest, GetModifyElement)
	{
		Tuple<int, double, bool> t(42, 3.14, true);
		get<0>(t) = 99;
		get<1>(t) = 2.71;
		get<2>(t) = false;
		EXPECT_EQ(get<0>(t), 99);
		EXPECT_EQ(get<1>(t), 2.71);
		EXPECT_EQ(get<2>(t), false);
	}

	// More complex tuple tests
	TEST(TupleTest, LargeTuple)
	{
		Tuple<int, int, int, int, int> t(1, 2, 3, 4, 5);
		EXPECT_EQ(get<0>(t), 1);
		EXPECT_EQ(get<1>(t), 2);
		EXPECT_EQ(get<2>(t), 3);
		EXPECT_EQ(get<3>(t), 4);
		EXPECT_EQ(get<4>(t), 5);
	}

	TEST(TupleTest, TupleWithStrings)
	{
		Tuple<std::string, int> t("hello", 42);
		EXPECT_EQ(get<0>(t), "hello");
		EXPECT_EQ(get<1>(t), 42);
	}

	TEST(TupleTest, ComparisonChain)
	{
		Tuple<int, int> t1(1, 2);
		Tuple<int, int> t2(1, 3);
		Tuple<int, int> t3(2, 1);
		EXPECT_TRUE(t1 < t2);
		EXPECT_TRUE(t2 < t3);
		EXPECT_TRUE(t1 < t3);
	}

} // namespace VT
