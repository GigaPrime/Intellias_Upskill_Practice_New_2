#include "pch.h"
#include <cstddef>

#include "UniquePtrTest.h"

namespace TypedTests
{
	using baseTypes = ::testing::Types<char, short, size_t, int, float, double, long>;
	TYPED_TEST_CASE(UniquePtrTestFixtureBaseTypes, baseTypes);
	TYPED_TEST(UniquePtrTestFixtureBaseTypes, CtorWithoutParamsShouldNotThrow)
	{
		ASSERT_NO_THROW(SPTR::UniquePtr<baseTypes> ptr);
	}

	using STLTypes = ::testing::Types<uint8_t, int8_t, uint16_t, int16_t>; // add more
	TYPED_TEST_CASE(UniquePtrTestFixtureSTLTypes, STLTypes);
	TYPED_TEST(UniquePtrTestFixtureSTLTypes, CtorWithoutParamsShouldNotThrow)
	{
		ASSERT_NO_THROW(SPTR::UniquePtr<STLTypes> ptr);
	}

	using complexTypes = ::testing::Types<std::string, std::vector<int>, std::map<int, int>>;
	TYPED_TEST_CASE(UniquePtrTestFixtureComplexTypes, complexTypes);
	TYPED_TEST(UniquePtrTestFixtureComplexTypes, CtorWithoutParamsShouldNotThrow)
	{
		ASSERT_NO_THROW(SPTR::UniquePtr<complexTypes> ptr);
	}

	using pointerTypes = ::testing::Types<char*, int*, std::string*>;
	TYPED_TEST_CASE(UniquePtrTestFixturePointerTypes, pointerTypes);
	TYPED_TEST(UniquePtrTestFixturePointerTypes, CtorWithoutParamsShouldNotThrow)
	{
		ASSERT_NO_THROW(SPTR::UniquePtr<pointerTypes> ptr);
	}

	using voidAndNullptrTypes = ::testing::Types<void*, nullptr_t>;
	TYPED_TEST_CASE(UniquePtrTestFixtureVoidAndNullptrTypes, voidAndNullptrTypes);
	TYPED_TEST(UniquePtrTestFixtureVoidAndNullptrTypes, CtorWithoutParamsShouldNotThrow)
	{
		ASSERT_NO_THROW(SPTR::UniquePtr<voidAndNullptrTypes> ptr);
	}
} // end of TypedTest

TEST(UniquePtrTestCtor, cTorTrivialHeapAllocatedPtrValue)
{
	int* valuePtr = new int(42);
	int value = *valuePtr;

	SPTR::UniquePtr<int> uPtr(valuePtr);
	EXPECT_EQ(valuePtr, nullptr);
	EXPECT_EQ(*uPtr.get(), value);
}

TEST(UniquePtrTestCtor, cTorTrivialStackAllocatedPtrValue)
{
	int value = 42;
	int* valuePtr = new int(value);

	SPTR::UniquePtr<int> uPtr(valuePtr);
	EXPECT_EQ(valuePtr, nullptr);
	EXPECT_EQ(*uPtr.get(), value);
}

TEST(UniquePtrTestCtor, cTorTrivialValue)
{
	int value = 42;
	SPTR::UniquePtr<int> uPtr(value);
	EXPECT_EQ(*uPtr.get(), value);
}

TEST(UniquePtrTestCtor, MoveCtorTrivialHeapAllocatedValue)
{
	int* valuePtr = new int(42);
	int value = *valuePtr;
	SPTR::UniquePtr<int> uPtr1(valuePtr);
	EXPECT_EQ(valuePtr, nullptr);
	EXPECT_EQ(*uPtr1.get(), value);

	SPTR::UniquePtr<int> uPtr2(std::move(uPtr1));
	EXPECT_EQ(uPtr1.get(), nullptr);
	EXPECT_EQ(*uPtr2.get(), value);
}

TEST(UniquePtrTestCtor, MoveCtorTrivialStackAllocatedValue)
{	
	int value = 42;
	int* valuePtr = new int(value);
	SPTR::UniquePtr<int> uPtr1(valuePtr);
	EXPECT_EQ(valuePtr, nullptr);
	EXPECT_EQ(*uPtr1.get(), value);

	SPTR::UniquePtr<int> uPtr2(std::move(uPtr1));
	EXPECT_EQ(uPtr1.get(), nullptr);
	EXPECT_EQ(*uPtr2.get(), value);
}

TEST(UniquePtrTestCtor, MoveAssignmentOperatorTest)
{
	int value = 42;
	SPTR::UniquePtr<int> uPtr1(value);
	SPTR::UniquePtr<int> uPtr2 = std::move(uPtr1);
	EXPECT_EQ(uPtr1.get(), nullptr);
	EXPECT_EQ(*uPtr2.get(), value);
}

TEST(UniquePtrTestDtor, DestructorDeletesAndDeallocatesComplexObject)
{
	// A 'side-handler' for the underlying vector
	std::vector<int>* ptrToVector;
	{
		SPTR::UniquePtr<std::vector<int>> uPtr(std::initializer_list<int>{42, 43, 44, 45});
		uPtr->shrink_to_fit();
		EXPECT_EQ(uPtr->size(), 4);
		EXPECT_EQ(uPtr->capacity(), uPtr->size()); // shrink_to_fit guarantees that
		EXPECT_EQ(uPtr->at(0), 42);
		ptrToVector = uPtr.get();
		// Destructor is called here
	}
	
	// ptrToVector becomes a dangling pointer, hovewer the memory is not physically wiped
	// data is still there, but the allocated memory is free
	EXPECT_EQ(ptrToVector->capacity(), 0);
}

struct Beacon
{
	// C++17 inline fields
	static inline int counter = 0;
	Beacon() { ++counter; }
	~Beacon() { --counter; }
};

TEST(UniquePtrTestDtor, DestructorDeletesAndDeallocatesCustomObject)
{
	{
		SPTR::UniquePtr<Beacon> uPtr = SPTR::makeUnique<Beacon>();
		EXPECT_EQ(Beacon::counter, 1);
	}
	EXPECT_EQ(Beacon::counter, 0);
}

// TEST uncompartible types while assigning or moving!!!

// Though I dont imagine how to test operator->

TEST(UniquePtrTestOperators, OperatorAsteriskReturnsValue)
{
	SPTR::UniquePtr<int> uPtr(42);
	EXPECT_EQ(*uPtr, 42);
}

TEST(UniquePtrTestOperators, OperatorAsteriskThrowsIfPtrIsNotInitialised)
{
	SPTR::UniquePtr<int> uPtr;
	EXPECT_THROW(*uPtr, std::runtime_error);
}

TEST(UniquePtrTestOperators, OperatorGetReturnsRawPtr)
{
	SPTR::UniquePtr<int> uPtr(42);
	EXPECT_TRUE((std::is_same_v<decltype(uPtr.get()), int*>));
}

TEST(UniquePtrTestOperators, OperatorGetReturnsNullptrIfUptrIsNotAssigned)
{
	SPTR::UniquePtr<int> uPtr;
	EXPECT_EQ(uPtr.get(), nullptr);
}

TEST(UniquePtrTestOperators, OperatorIndexReturnsValueFromSTLContainer)
{
	SPTR::UniquePtr<std::vector<int>> uPtr =
		SPTR::makeUnique<std::vector<int>>(std::initializer_list<int>{42, 43, 44, 45});
	
	size_t val = 42;
	for (size_t i = 0; i < uPtr->size(); ++i)
	{
		EXPECT_EQ(uPtr[i], val);
		++val;
	}
}

TEST(UniquePtrTestOperators, OperatorIndexReturnsValueFromPlainArray)
{
	const size_t size = 4;
	SPTR::UniquePtr<int[]> uPtr =
		SPTR::makeUnique<int>({42, 43, 44, 45});
	
	size_t val = 42;
	for (size_t i = 0; i < size; ++i)
	{
		EXPECT_EQ(uPtr[i], val);
		++val;
	}
}

TEST(UniquePtrTestOperators, OperatorIndexFailIfOutOfBounds)
{
	SPTR::UniquePtr<std::vector<int>> uPtr =
		SPTR::makeUnique<std::vector<int>>(std::initializer_list<int>{42, 43, 44, 45});
	
	//EXPECT_THROW(uPtr[4], std::out_of_range);
	EXPECT_THROW(uPtr->at(4), std::out_of_range);
}

TEST(UniquePtrTestOperators, OperatorIndexThrowsIfTypeIsNotIterable)
{
	SPTR::UniquePtr<int> uPtr(42);
	EXPECT_THROW(uPtr[0], std::runtime_error);
}

TEST(UniquePtrTestOperators, OperatorIndexThrowsIfPtrIsNotInitialised)
{
	SPTR::UniquePtr<std::vector<int>> uPtr;
	EXPECT_THROW(uPtr[0], std::runtime_error);
}

TEST(MakeUniqueTest, TrivialRValue)
{
	SPTR::UniquePtr<int> uPtr = SPTR::makeUnique<int>(42);
	EXPECT_EQ(*uPtr.get(), 42);
}

TEST(MakeUniqueTest, WithParamsForCtor)
{
	SPTR::UniquePtr<std::vector<int>> uPtr =
		SPTR::makeUnique<std::vector<int>>(std::initializer_list<int>{42, 43, 44, 45});
	
	size_t val = 42;
	for (const auto& elem : *uPtr.get())
	{
		EXPECT_EQ(elem, val);
		++val;
	}
}

TEST(MakeUniqueTest, WithoutParamsForCtorForCustomObject)
{
	struct Foo 
	{
		int val = 42;
	};

	SPTR::UniquePtr<Foo> uPtr = SPTR::makeUnique<Foo>();
	EXPECT_NE(uPtr.get(), nullptr);
	EXPECT_EQ(uPtr.get()->val, 42);
}

TEST(MakeUniqueTest, ShouldThrowIfCtorThrows)
{
	EXPECT_THROW(
		SPTR::UniquePtr<std::vector<std::uint64_t>> uPtr = 
		SPTR::makeUnique<std::vector<std::uint64_t>>(1'000'000'000'000'000), 
		std::bad_alloc);
}
