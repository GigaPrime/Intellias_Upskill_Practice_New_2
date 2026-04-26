#include "../src/Variant.h"

#include <string>
#include <sstream>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace VT
{
	TEST(VariantConstructorTest, DefaultConstructorCreatesFirstType)
	{
		Variant<int, double, std::string> variant;
		EXPECT_EQ(variant.index(), 0);
	}

	TEST(VariantConstructorTest, InPlaceConstructorCreatesTypeAtGivenIndex)
	{
		std::string str = "123";
		Variant<int, double, std::string> variant(std::in_place_index<2>, str);
		EXPECT_EQ(variant.index(), 2);
		EXPECT_TRUE(holdsAlternative<std::string>(variant));
	}

	TEST(VariantConstructorTest, InPlaceConstructorCreatesTypeAtGivenIndexIfDuplicatedTypes)
	{
		std::string str = "123";
		Variant<int, double, std::string, std::string> variant(std::in_place_index<3>, str);
		EXPECT_EQ(variant.index(), 3);
		EXPECT_TRUE(holdsAlternative<std::string>(variant));
	}

	TEST(VariantConstructorTest, CopyConstructorCopiesValue)
	{
		Variant<int, double> original(std::in_place_index<0>, 42);
		EXPECT_EQ(original.index(), 0);

		Variant<int, double> copy(original);
		EXPECT_EQ(copy.index(), 0);
		EXPECT_TRUE(holdsAlternative<int>(copy));
		EXPECT_TRUE(holdsAlternative<int>(original));
	}

	TEST(VariantConstructorTest, CopyConstructorCopiesValueIfDuplicatedTypes)
	{
		Variant<int, double, int> original(std::in_place_index<2>, 42);
		EXPECT_EQ(original.index(), 2);

		Variant<int, double, int> copy(original);	
		EXPECT_EQ(copy.index(), 2);
		EXPECT_TRUE(holdsAlternative<int>(copy));
		EXPECT_TRUE(holdsAlternative<int>(original));
	}

	TEST(VariantConstructorTest, MoveConstructorMovesValue)
	{
		Variant<int, double> original(std::in_place_index<1>, 42.42);
		EXPECT_EQ(original.index(), 1);
		Variant<int, double> moved(std::move(original));

		EXPECT_EQ(moved.index(), 1);
		EXPECT_TRUE(holdsAlternative<double>(moved));
		EXPECT_FALSE(holdsAlternative<double>(original));
	}

	TEST(VariantConstructorTest, MoveConstructorMovesValueIfDuplicatedTypes)
	{
		Variant<int, double, int> original(std::in_place_index<2>, 42);
		EXPECT_EQ(original.index(), 2);
		Variant<int, double, int> moved(std::move(original));
		EXPECT_EQ(moved.index(), 2);
		EXPECT_TRUE(holdsAlternative<int>(moved));
		EXPECT_FALSE(holdsAlternative<int>(original));
	}

	TEST(VariantAssignmentTest, CopyAssignmentHandlesValue)
	{
		Variant<int, double> original(std::in_place_index<0>, 42);
		Variant<int, double> target(std::in_place_index<1>, 1.5);

		target = original;

		EXPECT_EQ(target.index(), 0);
		EXPECT_TRUE(holdsAlternative<int>(target));
	}

	TEST(VariantAssignmentTest, CopyAssignmentToSameType)
	{
		Variant<int, double> original(std::in_place_index<0>, 100);
		Variant<int, double> target(std::in_place_index<0>, 50);

		target = original;

		EXPECT_EQ(target.index(), 0);
		EXPECT_TRUE(holdsAlternative<int>(target));
	}

	TEST(VariantAssignmentTest, MoveAssignmentHandlesValue)
	{
		Variant<int, double> original(std::in_place_index<0>, 999);
		Variant<int, double> target(std::in_place_index<1>, 1.1);

		target = std::move(original);

		EXPECT_EQ(target.index(), 0);
		EXPECT_TRUE(holdsAlternative<int>(target));
	}

	TEST(VariantIndexTest, IndexReturnsCorrectIndexForRandomType)
	{
		Variant<int, double, std::string> variant(std::in_place_index<1>, 2.5);
		EXPECT_EQ(variant.index(), 1);
	}

	TEST(VariantIndexTest, DefaultConstructorHasIndex0)
	{
		Variant<int, std::string> variant;
		EXPECT_EQ(variant.index(), 0);
	}

	TEST(VariantSwapTest, SwapExchangesContentsWithIntAndDouble)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<1>, 3.14);

		var1.swap(var2);

		EXPECT_EQ(var1.index(), 1);
		EXPECT_TRUE(holdsAlternative<double>(var1));
		EXPECT_EQ(var2.index(), 0);
		EXPECT_TRUE(holdsAlternative<int>(var2));
	}

	TEST(VariantSwapTest, SwapSameTypes)
	{
		Variant<int, double> var1(std::in_place_index<0>, 10);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		var1.swap(var2);

		EXPECT_EQ(var1.index(), 0);
		EXPECT_EQ(var2.index(), 0);
	}

	TEST(VariantEqualityTest, EqualityTrueForSameTypeAndValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<0>, 42);

		EXPECT_TRUE(var1 == var2);
	}

	TEST(VariantEqualityTest, EqualityFalseForDifferentValues)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<0>, 43);

		EXPECT_FALSE(var1 == var2);
	}

	TEST(VariantEqualityTest, EqualityFalseForDifferentTypes)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<1>, 42.0);

		EXPECT_FALSE(var1 == var2);
	}

	TEST(VariantInequalityTest, InequalityTrueForDifferentValues)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<0>, 43);

		EXPECT_TRUE(var1 != var2);
	}

	TEST(VariantInequalityTest, InequalityFalseForSameValues)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<0>, 42);

		EXPECT_FALSE(var1 != var2);
	}

	TEST(VariantInequalityTest, InequalityTrueForDifferentTypes)
	{
		Variant<int, double> var1(std::in_place_index<0>, 42);
		Variant<int, double> var2(std::in_place_index<1>, 42.0);

		EXPECT_TRUE(var1 != var2);
	}

	TEST(VariantLessThanTest, LessThanTrueForLessValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 10);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_TRUE(var1 < var2);
	}

	TEST(VariantLessThanTest, LessThanFalseForGreaterValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 20);
		Variant<int, double> var2(std::in_place_index<0>, 10);

		EXPECT_FALSE(var1 < var2);
	}

	TEST(VariantLessThanTest, LessThanFalseForEqualValues)
	{
		Variant<int, double> var1(std::in_place_index<0>, 15);
		Variant<int, double> var2(std::in_place_index<0>, 15);

		EXPECT_FALSE(var1 < var2);
	}

	TEST(VariantLessThanTest, LessThanComparesTypeIndexWhenDifferent)
	{
		Variant<int, double> var1(std::in_place_index<0>, 100);
		Variant<int, double> var2(std::in_place_index<1>, 1.0);

		EXPECT_TRUE(var1 < var2);
	}

	TEST(VariantGreaterThanTest, GreaterThanTrueForGreaterValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 30);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_TRUE(var1 > var2);
	}

	TEST(VariantGreaterThanTest, GreaterThanFalseForLessValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 10);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_FALSE(var1 > var2);
	}

	TEST(VariantGreaterThanTest, GreaterThanFalseForEqualValues)
	{
		Variant<int, double> var1(std::in_place_index<0>, 15);
		Variant<int, double> var2(std::in_place_index<0>, 15);

		EXPECT_FALSE(var1 > var2);
	}

	TEST(VariantLessThanOrEqualTest, LessThanOrEqualTrueForLessValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 10);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_TRUE(var1 <= var2);
	}

	TEST(VariantLessThanOrEqualTest, LessThanOrEqualTrueForEqualValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 15);
		Variant<int, double> var2(std::in_place_index<0>, 15);

		EXPECT_TRUE(var1 <= var2);
	}

	TEST(VariantLessThanOrEqualTest, LessThanOrEqualFalseForGreaterValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 30);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_FALSE(var1 <= var2);
	}

	TEST(VariantGreaterThanOrEqualTest, GreaterThanOrEqualTrueForGreaterValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 30);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_TRUE(var1 >= var2);
	}

	TEST(VariantGreaterThanOrEqualTest, GreaterThanOrEqualTrueForEqualValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 15);
		Variant<int, double> var2(std::in_place_index<0>, 15);

		EXPECT_TRUE(var1 >= var2);
	}

	TEST(VariantGreaterThanOrEqualTest, GreaterThanOrEqualFalseForLesserValue)
	{
		Variant<int, double> var1(std::in_place_index<0>, 10);
		Variant<int, double> var2(std::in_place_index<0>, 20);

		EXPECT_FALSE(var1 >= var2);
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeTrueForCorrectType)
	{
		Variant<int, double, std::string> variant(std::in_place_index<0>, 42);
		EXPECT_TRUE(holdsAlternative<int>(variant));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeTrueForCorrectDuplicatedType)
	{
		Variant<int, double, int, std::string> variant(std::in_place_index<2>, 42);
		EXPECT_TRUE(holdsAlternative<int>(variant));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeFalseForIncorrectType)
	{
		Variant<int, double, std::string> variant(std::in_place_index<0>, 42);
		EXPECT_FALSE(holdsAlternative<double>(variant));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeFalseForIncorrectDuplicatedType)
	{
		Variant<int, double, double, std::string> variant(std::in_place_index<0>, 42.42);
		EXPECT_FALSE(holdsAlternative<std::string>(variant));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeAfterCopyConstruction)
	{
		Variant<int, double> original(std::in_place_index<1>, 2.71);
		Variant<int, double> copy(original);

		EXPECT_TRUE(holdsAlternative<double>(copy));
		EXPECT_FALSE(holdsAlternative<int>(copy));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeAfterMoveConstruction)
	{
		Variant<int, double> original(std::in_place_index<0>, 100);
		Variant<int, double> moved(std::move(original));

		EXPECT_TRUE(holdsAlternative<int>(moved));
		EXPECT_FALSE(holdsAlternative<double>(moved));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeAfterCopyAssignment)
	{
		Variant<int, double> source(std::in_place_index<1>, 1.5);
		Variant<int, double> target(std::in_place_index<0>, 5);

		target = source;

		EXPECT_TRUE(holdsAlternative<double>(target));
		EXPECT_FALSE(holdsAlternative<int>(target));
	}

	TEST(HoldsAlternativeTest, HoldsAlternativeAfterMoveAssignment)
	{
		Variant<int, double> source(std::in_place_index<0>, 100);
		Variant<int, double> target(std::in_place_index<1>, 2.5);

		target = std::move(source);

		EXPECT_TRUE(holdsAlternative<int>(target));
		EXPECT_FALSE(holdsAlternative<double>(target));
	}

} // namespace VT