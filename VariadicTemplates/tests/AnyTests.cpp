#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>
#include <utility>

#include "../src/Any.h"

namespace VT 
{
	TEST(AnyTests, DefaultIsVoid)
	{
		Any a;
		EXPECT_EQ(a.type().hash_code(), typeid(void).hash_code());
	}
	
	TEST(AnyTests, StoreIntType)
	{
		Any a = 42;
		EXPECT_EQ(a.type().hash_code(), typeid(int).hash_code());
	}
	
	TEST(AnyTests, StoreStringType)
	{
		Any a = std::string("abc");
		EXPECT_EQ(a.type().hash_code(), typeid(std::string).hash_code());
	}
	
	TEST(AnyTests, ConstructStringByValue)
	{
		Any a = std::string("hello");
		EXPECT_EQ(a.type().hash_code(), typeid(std::string).hash_code());
	}
	
	TEST(AnyTests, CopyConstructPreservesType)
	{
		Any a = 5;
		Any b(a);
		EXPECT_EQ(b.type().hash_code(), typeid(int).hash_code());
	}
	
	TEST(AnyTests, CopyAssignmentSelf)
	{
		Any a = 3;
		// Self-assignment should be safe
		a = a; 
		EXPECT_EQ(a.type().hash_code(), typeid(int).hash_code());
	}
	
	TEST(AnyTests, AssignDifferentTypes)
	{
		Any a = 1;
		EXPECT_EQ(a.type().hash_code(), typeid(int).hash_code());
		a = std::string("x");
		EXPECT_EQ(a.type().hash_code(), typeid(std::string).hash_code());
	}
	
	TEST(AnyTests, MoveConstructorLeavesSourceEmpty)
	{
		Any a = 7;
		Any b(std::move(a));
		EXPECT_EQ(b.type().hash_code(), typeid(int).hash_code());
		EXPECT_EQ(a.type().hash_code(), typeid(void).hash_code());
	}
	
	TEST(AnyTests, MoveAssignmentLeavesSourceEmpty)
	{
		Any a = 10;
		Any b;
		b = std::move(a);
		EXPECT_EQ(b.type().hash_code(), typeid(int).hash_code());
		EXPECT_EQ(a.type().hash_code(), typeid(void).hash_code());
	}
	
	TEST(AnyTests, NonCopyableTypeTraits)
	{
		// unique_ptr is not copy constructible
		EXPECT_FALSE(std::is_copy_constructible_v<std::unique_ptr<int>>);
	}
	
	TEST(AnyTests, ConstructVectorByValue)
	{
		Any a = std::vector<int>{1,2,3};
		EXPECT_EQ(a.type().hash_code(), typeid(std::vector<int>).hash_code());
	}

	TEST(AnyTests, HasValueAndReset)
	{
		Any a;
		EXPECT_FALSE(a.hasValue());
		a = 11;
		EXPECT_TRUE(a.hasValue());
		a.reset();
		EXPECT_FALSE(a.hasValue());
		EXPECT_EQ(a.type(), typeid(void));
	}

	TEST(AnyTests, EmplaceAndInPlaceConstructor)
	{
		Any a;
		a.emplace<std::string>("emplaced");
		EXPECT_EQ(anyCast<std::string>(a), std::string("emplaced"));

		using Pair = std::pair<int,int>;
		Any b(std::in_place_type_t<Pair>{}, 2, 3);
		EXPECT_EQ(anyCast<Pair>(b).first, 2);
		EXPECT_EQ(anyCast<Pair>(b).second, 3);
	}

	TEST(AnyTests, SwapSwapsValues)
	{
		Any a = 1;
		Any b = std::string("s");
		a.swap(b);
		EXPECT_EQ(a.type(), typeid(std::string));
		EXPECT_EQ(b.type(), typeid(int));
		EXPECT_EQ(anyCast<std::string>(a), std::string("s"));
		EXPECT_EQ(anyCast<int>(b), 1);
	}

	TEST(AnyTests, AnyCastLvalueAndRvalueAndThrow)
	{
		Any a = 42;
		// lvalue anyCast
		int v1 = anyCast<int>(a);
		EXPECT_EQ(v1, 42);

		// Non-const reference anyCast
		int v2 = anyCast<int>(static_cast<Any&>(a));
		EXPECT_EQ(v2, 42);

		// Rvalue anyCast
		Any s = std::string("hello");
		std::string sres = anyCast<std::string>(std::move(s));
		EXPECT_EQ(sres, std::string("hello"));

		// Wrong type throws
		EXPECT_THROW(anyCast<std::string>(a), std::bad_any_cast);
	}

	TEST(AnyTests, AnyCastPointerOverloads)
	{
		Any a = 7;
		// Disambiguate overloaded pointer anyCast by taking function pointers to specific overloads
		using ConstFn = const int*(*)(const Any*);
		ConstFn constAnyCastInt = static_cast<ConstFn>(&VT::anyCast<int>);
		const int* pc = constAnyCastInt(&a);
		EXPECT_NE(pc, nullptr);
		EXPECT_EQ(*pc, 7);

		using NonConstFn = int*(*)(Any*);
		NonConstFn nonConstAnyCastInt = static_cast<NonConstFn>(&VT::anyCast<int>);
		int* p = nonConstAnyCastInt(&a);
		EXPECT_NE(p, nullptr);
		*p = 99;
		EXPECT_EQ(anyCast<int>(a), 99);

		// Pointer cast to wrong type returns nullptr
		using ConstStringFn = const std::string*(*)(const Any*);
		ConstStringFn constAnyCastString = static_cast<ConstStringFn>(&VT::anyCast<std::string>);
		EXPECT_EQ(constAnyCastString(&a), nullptr);
	}
} // namespace VT
