#include "../src/Variant.h"

#include <string>
#include <sstream>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace VT
{
    TEST(VariantCostructorTest, DefaultConstructorCreatesFirstType)
    {
		std::size_t index = 0;
        Variant<int, double, std::string> variant;
        EXPECT_EQ(variant.index(), index);
    }

    TEST(VariantCostructorTest, InPlaceConstructorCreatesTypeAtGivenIndex)
    {
		std::size_t index = 2;
		std::string str = "123";
        Variant<int, double, std::string> variant(std::in_place_index<2>, str);
		EXPECT_EQ(variant.index(), index);
        EXPECT_EQ(holdsAlternative<std::string>(variant), true);
        //EXPECT_EQ(get<2>(variant), str);
    }

    TEST(VariantCostructorTest, InPlaceConstructorFailsIfIndexOutOfRange)
    {
        std::size_t index = 4;
        // This should fail to compile
        // Variant<int, double, std::string> variant(std::in_place_index<4>, "123");
    }



} // namespace VT