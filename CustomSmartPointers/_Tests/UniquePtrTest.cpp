#include "pch.h"
#include <cstddef>

#include "UniquePtrTest.h"

namespace UniquePtrTests 
{
	namespace TypedTests
	{
		using baseTypes = ::testing::Types<char, short, size_t, int, float, double, long>;
		TYPED_TEST_CASE(UniquePtrTestFixtureBaseTypes, baseTypes);
		TYPED_TEST(UniquePtrTestFixtureBaseTypes, CtorWithoutParamsShouldNotThrow)
		{
			ASSERT_NO_THROW(SPTR::UniquePtr<TypeParam> ptr);
		}

		using STLTypes = ::testing::Types<uint8_t, int8_t, uint16_t, int16_t>; // add more
		TYPED_TEST_CASE(UniquePtrTestFixtureSTLTypes, STLTypes);
		TYPED_TEST(UniquePtrTestFixtureSTLTypes, CtorWithoutParamsShouldNotThrow)
		{
			ASSERT_NO_THROW(SPTR::UniquePtr<TypeParam> ptr);
		}

		using complexTypes = ::testing::Types<std::string, std::vector<int>, std::map<int, int>>;
		TYPED_TEST_CASE(UniquePtrTestFixtureComplexTypes, complexTypes);
		TYPED_TEST(UniquePtrTestFixtureComplexTypes, CtorWithoutParamsShouldNotThrow)
		{
			ASSERT_NO_THROW(SPTR::UniquePtr<TypeParam> ptr);
		}

		using pointerTypes = ::testing::Types<char*, int*, std::string*>;
		TYPED_TEST_CASE(UniquePtrTestFixturePointerTypes, pointerTypes);
		TYPED_TEST(UniquePtrTestFixturePointerTypes, CtorWithoutParamsShouldNotThrow)
		{
			ASSERT_NO_THROW(SPTR::UniquePtr<TypeParam> ptr);
		}

		using voidAndNullptrTypes = ::testing::Types<void*, nullptr_t>;
		TYPED_TEST_CASE(UniquePtrTestFixtureVoidAndNullptrTypes, voidAndNullptrTypes);
		TYPED_TEST(UniquePtrTestFixtureVoidAndNullptrTypes, CtorWithoutParamsShouldNotThrow)
		{
			ASSERT_NO_THROW(SPTR::UniquePtr<TypeParam> ptr);
		}
	} // end of TypedTest

	TEST(UniquePtrTestCtor, DefaultCtoReturnsNullptr)
	{
		SPTR::UniquePtr<int> uPtr;
		EXPECT_EQ(uPtr.get(), nullptr);
	}

	TEST(UniquePtrTestCtor, CtorReceivingNullptrReturnsNullptr)
	{
		SPTR::UniquePtr<int> uPtr(nullptr);
		EXPECT_EQ(uPtr.get(), nullptr);
	}

	TEST(UniquePtrTestCtor, UniquePtrRturnsAdoptedPtrValue)
	{
		int* valuePtr = new int(42);

		SPTR::UniquePtr<int> uPtr(valuePtr);
		EXPECT_EQ(*uPtr.get(), 42);
	}

	TEST(UniquePtrTestMoveCtor, CtorReturnsMovedPtrValue)
	{
		int* valuePtr = new int(42);
		SPTR::UniquePtr<int> uPtr1(valuePtr);
		EXPECT_EQ(*uPtr1.get(), 42);

		SPTR::UniquePtr<int> uPtr2(std::move(uPtr1));
		EXPECT_EQ(uPtr1.get(), nullptr);
		EXPECT_EQ(*uPtr2.get(), 42);
	}

	TEST(UniquePtrTestMoveCtor, CtorDoesNothingIfSelfMoved)
	{
		SPTR::UniquePtr<int> uPtr1 = SPTR::makeUnique<int>(42);
		auto addressBeforeMove = uPtr1.get();

		uPtr1 = std::move(uPtr1);
		auto addressAfterMove = uPtr1.get();
		EXPECT_EQ(addressBeforeMove, addressAfterMove);
		EXPECT_EQ(*uPtr1.get(), 42);
	}

	struct Beacon
	{
		// C++17 inline fields
		static inline int counter = 0;
		Beacon() { ++counter; }
		~Beacon() { --counter; }
	};

	TEST(UniquePtrReset, ResetDelesResourceAndSetsPtrToNullptr)
	{
		SPTR::UniquePtr<Beacon> uPtr = SPTR::makeUnique<Beacon>();
		EXPECT_EQ(Beacon::counter, 1);
		uPtr.reset();
		EXPECT_EQ(Beacon::counter, 0);
		EXPECT_EQ(uPtr.get(), nullptr);
	}

	TEST(UniquePtrTestDtor, DestructorDeletesAndDeallocatesCustomObject)
	{
		{
			SPTR::UniquePtr<Beacon> uPtr = SPTR::makeUnique<Beacon>();
			EXPECT_EQ(Beacon::counter, 1);
			//EXPECT_CALL(uPtr.~UniquePtr());
		}
		EXPECT_EQ(Beacon::counter, 0);
	}

	TEST(UniquePtrTestOperators, MoveAssignmentMovesOwnership)
	{
		int value = 42;
		int* valuePtr = new int(value);
		SPTR::UniquePtr<int> uPtr1(valuePtr);	// Why this causese a failure: SPTR::UniquePtr<int> uPtr1(new int(42)) ?
		SPTR::UniquePtr<int> uPtr2 = std::move(uPtr1);
		EXPECT_EQ(uPtr1.get(), nullptr);
		EXPECT_EQ(*uPtr2.get(), value);
	}

	TEST(UniquePtrTestOperators, MoveAssignmentDoesNothingIfSelfAssigned)
	{
		SPTR::UniquePtr<int> uPtr1 = SPTR::makeUnique<int>(42);
		auto addressBeforeMove = uPtr1.get();
		uPtr1 = std::move(uPtr1);
		auto addressAfterMove = uPtr1.get();
		EXPECT_EQ(addressBeforeMove, addressAfterMove);
		EXPECT_EQ(*uPtr1.get(), 42);
	}

	TEST(UniquePtrTestOperators, OperatorAsteriskThrowsIfNotSet)
	{
		SPTR::UniquePtr<int> uPtr;
		EXPECT_THROW(*uPtr, std::runtime_error);
	}

	TEST(UniquePtrTestOperators, OperatorAsteriskReturnsNewValueIfSet)
	{
		SPTR::UniquePtr<int> uPtr;
		uPtr = SPTR::makeUnique<int>(42);
		*uPtr = 43;
		EXPECT_EQ(*uPtr, 43);
	}

	TEST(UniquePtrTestOperators, OperatorAsteriskSetsValue)
	{
		SPTR::UniquePtr<int> uPtr;
		uPtr = SPTR::makeUnique<int>(42);
		*uPtr = 43;
		EXPECT_EQ(*uPtr, 43);
	}

	TEST(UniquePtrTestOperators, OperatorArrowReturnsValue)
	{
		SPTR::UniquePtr<std::vector<int>> uPtr
			= SPTR::makeUnique<std::vector<int>>(std::vector<int>{42, 43, 44, 45});
		EXPECT_EQ(uPtr->at(0), 42);
		/*
			SPTR::UniquePtr<std::vector<int>> uPtr2 = SPTR::makeUnique<std::vector<int>>(42, 43, 44, 45);
			EXPECT_EQ(uPtr2->at(3), 45)*/;
	}

	TEST(UniquePtrTestOperators, OperatorArrowSetsValue)
	{
		SPTR::UniquePtr<std::vector<int>> uPtr
			= SPTR::makeUnique<std::vector<int>>(std::vector<int>{42, 43, 44, 45});
		EXPECT_EQ(uPtr->at(0), 42);
		uPtr->at(0) = 43;
		EXPECT_EQ(uPtr->at(0), 43);
	}

	TEST(UniquePtrTestOperators, OperatorGetReturnsRawPtr)
	{
		SPTR::UniquePtr<int> uPtr = SPTR::makeUnique<int>(42);
		EXPECT_TRUE((std::is_same_v<decltype(uPtr.get()), int*>));
	}

	TEST(UniquePtrTestOperators, OperatorGetReturnsNullptrIfUptrIsNotAssigned)
	{
		SPTR::UniquePtr<int> uPtr;
		EXPECT_EQ(uPtr.get(), nullptr);
	}

	TEST(UniquePtrTestOperators, OperatorSubscriptReturnsValueFromSTLContainer)
	{
		SPTR::UniquePtr<std::vector<int>> uPtr =
			SPTR::makeUnique<std::vector<int>>(std::vector<int>{42, 43, 44, 45});

		size_t val = 42;
		for (size_t i = 0; i < uPtr->size(); ++i)
		{
			EXPECT_EQ(uPtr[i], val);
			++val;
		}
	}

	TEST(UniquePtrTestOperators, OperatorSubscriptReturnsValueFromPlainArray)
	{
		const size_t size = 4;
		SPTR::UniquePtr<int[]> uPtr =
			SPTR::makeUnique<int>({ 42, 43, 44, 45 });

		size_t val = 42;
		for (size_t i = 0; i < size; ++i)
		{
			EXPECT_EQ(uPtr[i], val);
			++val;
		}
	}

	TEST(UniquePtrTestOperators, OperatorSubscriptThrowsIfTypeIsNotIterable)
	{
		SPTR::UniquePtr<int> uPtr = SPTR::makeUnique<int>(42);
		EXPECT_THROW(uPtr[0], std::runtime_error);
	}

	TEST(UniquePtrTestOperators, OperatorSubscriptThrowsIfPtrIsNotInitialised)
	{
		SPTR::UniquePtr<std::vector<int>> uPtr;
		EXPECT_THROW(uPtr[0], std::runtime_error);
	}

	TEST(UniquePtrOperators, OperatorSubscriptFailsIAtCompileTimeIfVoid)
	{
		SPTR::UniquePtr<void> uPtr = SPTR::makeUnique<void>();
		//EXPECT_THROW(uPtr[0], std::runtime_error); -> fails at compile time. Expected, since void is not Subscriptable.
	}

	TEST(MakeUniqueTest, TrivialRValue)
	{
		SPTR::UniquePtr<int> uPtr = SPTR::makeUnique<int>(42);
		EXPECT_EQ(*uPtr.get(), 42);
	}

	TEST(MakeUniqueTest, WithParamsForCtor)
	{
		SPTR::UniquePtr<std::vector<int>> uPtr =
			SPTR::makeUnique<std::vector<int>>(std::vector<int>{42, 43, 44, 45});

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

	struct ResourceByMalloc
	{
		int value = 42;
		ResourceByMalloc() { std::cout << "ResourceByMalloc created"; }
		~ResourceByMalloc() { std::cout << "ResourceByMalloc destroyed"; }
	};

	struct ResourceByNew
	{
		int value = 43;
		ResourceByNew() { std::cout << "ResourceByNew created"; }
		~ResourceByNew() { std::cout << "ResourceByNew destroyed"; }
	};

	template <typename T>
	struct Deleter
	{
		void operator()(T* ptr) const
		{
			if constexpr (std::is_same_v<T, ResourceByMalloc>)
			{
				std::cout << "Custom deleter for ResourceByMalloc called\n";
				std::destroy_at(ptr);
				std::free(ptr);
			}
			else if constexpr (std::is_same_v<T, ResourceByNew>)
			{
				std::cout << "Custom deleter for ResourceByNew called\n";
				delete ptr;
			}
			else
			{
				throw std::runtime_error("Unsupported resource type");
			}
		}
	};

	TEST(CustomDeleter, UniquePtrWithCustomDeleterDeletesResource)
	{
		auto resource = std::construct_at(static_cast<ResourceByMalloc*>(
			std::malloc(sizeof(ResourceByMalloc))), ResourceByMalloc());
		Deleter<ResourceByMalloc> deleterForMalloc;
		{
			SPTR::UniquePtr<ResourceByMalloc, Deleter<ResourceByMalloc>> uPtr(resource);
			EXPECT_EQ(uPtr->value, 42);
			EXPECT_NO_THROW(uPtr.~UniquePtr()); // Explicitly calling destructor to test the custom deleter
			EXPECT_EQ(uPtr.get(), nullptr); // After destructor, the pointer should be null
		}
	}

	TEST(CustomDeleter, UniquePtrWithDefaultDeleterDeletesResource)
	{
		auto resource = new ResourceByNew();
		SPTR::UniquePtr<ResourceByNew, Deleter<ResourceByNew>> uPtr(resource);
		EXPECT_EQ(uPtr->value, 43);
		EXPECT_NO_THROW(uPtr.~UniquePtr()); // Explicitly calling destructor to test the custom deleter
		EXPECT_EQ(uPtr.get(), nullptr); // After destructor, the pointer should be null
	}

	TEST(ComparisonOperators, ComparisonReturnsTrueIfEqualAndFalseIfNot)
	{
		SPTR::UniquePtr<int> uPtr1 = SPTR::makeUnique<int>(42);
		SPTR::UniquePtr<int> uPtr2 = SPTR::makeUnique<int>(42);
		EXPECT_TRUE(uPtr1 == uPtr1); // comparing with itself
		EXPECT_FALSE(uPtr1 == uPtr2); // comparing with another unique_ptr
		EXPECT_FALSE(uPtr1 != uPtr1); // comparing with itself
		EXPECT_TRUE(uPtr1 != uPtr2); // comparing with another unique_ptr
		EXPECT_TRUE(uPtr1 < uPtr2 || uPtr2 < uPtr1); // comparing addresses, one should be less than the other
		EXPECT_TRUE(uPtr1 > uPtr2 || uPtr2 > uPtr1);
		EXPECT_TRUE(uPtr1 <= uPtr2 || uPtr2 <= uPtr1);
		EXPECT_TRUE(uPtr1 >= uPtr2 || uPtr2 >= uPtr1);
	}
} // end of UniquePtrTests
