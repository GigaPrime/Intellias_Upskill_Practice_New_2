#include "pch.h"
#include <cstddef>

#include "../Pointers/SharedPtr.h"

namespace SharedPtrTests
{   
    struct Beacon
    {
        static inline int counter = 0;
        Beacon() { ++counter; }
        ~Beacon() { --counter; }
    };

    template <typename T>
    class TestFixtureBaseTypes : public ::testing::Test
    {
    protected:
        SPTR::SharedPtr<T> sPtr;
    };

    template <typename T>
    class SharedPtrTestFixtureComplexTypes : public ::testing::Test
    {
    protected:
        SPTR::SharedPtr<T> sPtr;
    };

    namespace TypedTests
    {
        using baseTypes = ::testing::Types<char, short, size_t, int, float, double, long>;
        TYPED_TEST_CASE(TestFixtureBaseTypes, baseTypes);
        TYPED_TEST(TestFixtureBaseTypes, DefaultCtorShouldNotThrow)
        {
            ASSERT_NO_THROW(SPTR::SharedPtr<TypeParam> ptr);
        }

        using complexTypes = ::testing::Types<std::string, std::vector<int>, std::map<int, int>>;
        TYPED_TEST_CASE(SharedPtrTestFixtureComplexTypes, complexTypes);
        TYPED_TEST(SharedPtrTestFixtureComplexTypes, DefaultCtorShouldNotThrow)
        {
            ASSERT_NO_THROW(SPTR::SharedPtr<TypeParam> ptr);
        }
    } // end of TypedTests

    TEST(SharedPtrTestCtor, DefaultCtorReturnsNullptr)
    {
        SPTR::SharedPtr<int> sPtr;
        EXPECT_EQ(sPtr.get(), nullptr);
        EXPECT_EQ(sPtr.refCount(), 0);
    }

    TEST(SharedPtrTestCtor, CtorReceivingNullptrReturnsNullptr)
    {
        SPTR::SharedPtr<int> sPtr(nullptr);
        EXPECT_EQ(sPtr.get(), nullptr);
    }

    TEST(SharedPtrTestCtor, CtorAdoptsRawPointerAndInitializesRefCount)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr(valuePtr);
        EXPECT_EQ(*sPtr.get(), 42);
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestCtor, CtorAdoptsPointerToCustomObject)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        EXPECT_NE(sPtr.get(), nullptr);
        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestCopyCtor, CopyCtorIncrementsRefCount)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        EXPECT_EQ(sPtr1.refCount(), 1);

        SPTR::SharedPtr<int> sPtr2(sPtr1);
        EXPECT_EQ(sPtr2.refCount(), 2);
        EXPECT_EQ(sPtr1.refCount(), 2);
        EXPECT_EQ(*sPtr1.get(), *sPtr2.get());
    }

    TEST(SharedPtrTestCopyCtor, CopyCtorFromDefaultConstructedReturnsNullptr)
    {
        SPTR::SharedPtr<int> sPtr1;
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        EXPECT_EQ(sPtr2.get(), nullptr);
    }

    TEST(SharedPtrTestCopyCtor, MultipleCopiesShareOwnership)
    {
        int* valuePtr = new int(100);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        SPTR::SharedPtr<int> sPtr3(sPtr2);
        SPTR::SharedPtr<int> sPtr4(sPtr3);

        EXPECT_EQ(sPtr1.refCount(), 4);
        EXPECT_EQ(sPtr2.refCount(), 4);
        EXPECT_EQ(sPtr3.refCount(), 4);
        EXPECT_EQ(sPtr4.refCount(), 4);
        EXPECT_EQ(*sPtr4.get(), 100);
    }

    TEST(SharedPtrTestCopyCtor, CopyCtorIncrementsRefCountForCustomObject)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 1);

        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);
        EXPECT_EQ(Beacon::counter, 1); // Still only one Beacon object
        EXPECT_EQ(sPtr2.refCount(), 2);
    }

    TEST(SharedPtrTestMoveCtor, MoveCtorTransfersOwnership)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        EXPECT_EQ(sPtr1.refCount(), 1);

        SPTR::SharedPtr<int> sPtr2(std::move(sPtr1));
        EXPECT_EQ(sPtr1.get(), nullptr);
        EXPECT_EQ(*sPtr2.get(), 42);
        EXPECT_EQ(sPtr2.refCount(), 1);
    }

    TEST(SharedPtrTestMoveCtor, MoveCtorDoesNothingIfSelfMoved)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        auto addressBeforeMove = sPtr1.get();

        sPtr1 = std::move(sPtr1);
        auto addressAfterMove = sPtr1.get();
        EXPECT_EQ(addressBeforeMove, addressAfterMove);
        EXPECT_EQ(*sPtr1.get(), 42);
    }

    TEST(SharedPtrTestMoveCtor, MoveCtorFromMultipleOwners)
    {
        int* valuePtr = new int(99);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        SPTR::SharedPtr<int> sPtr3(sPtr1);

        EXPECT_EQ(sPtr1.refCount(), 3);
        SPTR::SharedPtr<int> sPtr4(std::move(sPtr1));

        EXPECT_EQ(sPtr1.get(), nullptr);
        EXPECT_EQ(*sPtr4.get(), 99);
        EXPECT_EQ(sPtr2.refCount(), 3);
        EXPECT_EQ(sPtr3.refCount(), 3);
        EXPECT_EQ(sPtr4.refCount(), 3);
    }

    TEST(SharedPtrTestMoveCtor, MoveCtorCustomObject)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 1);

        SPTR::SharedPtr<Beacon> sPtr2(std::move(sPtr1));
        EXPECT_EQ(sPtr1.get(), nullptr);
        EXPECT_EQ(Beacon::counter, 1); // Still one Beacon
        EXPECT_EQ(sPtr2.refCount(), 1);
    }

    TEST(SharedPtrTestCopyAssignment, CopyAssignmentIncrementsRefCount)
    {
        int* valuePtr1 = new int(42);
        int* valuePtr2 = new int(99);

        SPTR::SharedPtr<int> sPtr1(valuePtr1);
        SPTR::SharedPtr<int> sPtr2(valuePtr2);
        EXPECT_EQ(sPtr1.refCount(), 1);
        EXPECT_EQ(sPtr2.refCount(), 1);

        sPtr1 = sPtr2;
        EXPECT_EQ(sPtr1.refCount(), 2);
        EXPECT_EQ(sPtr2.refCount(), 2);
        EXPECT_EQ(*sPtr1.get(), 99);
    }

    TEST(SharedPtrTestCopyAssignment, CopyAssignmentDeletesOldOwnership)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);

        SPTR::SharedPtr<Beacon> sPtr2(new Beacon());
        EXPECT_EQ(Beacon::counter, 2);

        sPtr1 = sPtr2;
        EXPECT_EQ(Beacon::counter, 1); // First Beacon deleted
        EXPECT_EQ(sPtr1.refCount(), 2);
        EXPECT_EQ(sPtr2.refCount(), 2);
    }

    TEST(SharedPtrTestCopyAssignment, CopyAssignmentIgnoresSelfAssignment)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr(valuePtr);
        auto addressBefore = sPtr.get();

        sPtr = sPtr;
        auto addressAfter = sPtr.get();
        EXPECT_EQ(addressBefore, addressAfter);
        EXPECT_EQ(*sPtr.get(), 42);
    }

    TEST(SharedPtrTestCopyAssignment, CopyAssignmentFromNullptr)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr1(valuePtr);
        SPTR::SharedPtr<int> sPtr2;

        sPtr1 = sPtr2;
        EXPECT_EQ(sPtr1.get(), nullptr);
    }

    TEST(SharedPtrTestMoveAssignment, MoveAssignmentTransfersOwnership)
    {
        int* valuePtr1 = new int(42);
        int* valuePtr2 = new int(99);

        SPTR::SharedPtr<int> sPtr1(valuePtr1);
        SPTR::SharedPtr<int> sPtr2(valuePtr2);

        sPtr1 = std::move(sPtr2);
        EXPECT_EQ(sPtr1.get(), valuePtr2);
        EXPECT_EQ(sPtr2.get(), nullptr);
        EXPECT_EQ(*sPtr1.get(), 99);
    }

    TEST(SharedPtrTestMoveAssignment, MoveAssignmentDeletesOldResource)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(new Beacon());
        EXPECT_EQ(Beacon::counter, 2);

        sPtr1 = std::move(sPtr2);
        EXPECT_EQ(Beacon::counter, 1); // First Beacon deleted
        EXPECT_EQ(sPtr1.refCount(), 1);
        EXPECT_EQ(sPtr2.get(), nullptr);
    }

    TEST(SharedPtrTestMoveAssignment, MoveAssignmentIgnoresSelfAssignment)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr(valuePtr);
        auto addressBefore = sPtr.get();

        sPtr = std::move(sPtr);
        auto addressAfter = sPtr.get();
        EXPECT_EQ(addressBefore, addressAfter);
        EXPECT_EQ(*sPtr.get(), 42);
    }

    TEST(SharedPtrTestMoveAssignment, MoveAssignmentFromMultipleOwners)
    {
        int* valuePtr1 = new int(42);

        SPTR::SharedPtr<int> sPtr1(valuePtr1);
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        SPTR::SharedPtr<int> sPtr3(new int(43));

        EXPECT_EQ(sPtr1.refCount(), 2);
        sPtr3 = std::move(sPtr1);

        EXPECT_EQ(sPtr1.get(), nullptr);
        EXPECT_EQ(*sPtr3.get(), 42);
        EXPECT_EQ(sPtr2.refCount(), 2);
        EXPECT_EQ(sPtr3.refCount(), 2);
    }

    TEST(SharedPtrTestDtor, DestructorDeletesResourceWhenLastOwner)
    {
        {
            SPTR::SharedPtr<Beacon> sPtr(new Beacon());
            EXPECT_EQ(Beacon::counter, 1);
        }
        EXPECT_EQ(Beacon::counter, 0);
    }

    TEST(SharedPtrTestDtor, DestructorDoesNotDeleteIfOtherOwnersExist)
    {
        SPTR::SharedPtr<int> sPtr1;
        {
            SPTR::SharedPtr<int> sPtr2(new int(42));
            sPtr1 = sPtr2;
            EXPECT_EQ(sPtr1.refCount(), 2);
        }
        EXPECT_EQ(*sPtr1.get(), 42);
        EXPECT_EQ(sPtr1.refCount(), 1);
    }

    TEST(SharedPtrTestDtor, DestructorDecrementRefCountCorrectly)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);
        SPTR::SharedPtr<Beacon> sPtr3(sPtr1);

        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 3);

        {
            SPTR::SharedPtr<Beacon> tempPtr(sPtr1);
            EXPECT_EQ(sPtr1.refCount(), 4);
        }

        EXPECT_EQ(sPtr1.refCount(), 3);
        EXPECT_EQ(Beacon::counter, 1);
    }

    TEST(SharedPtrTestDtor, DestructorDeletesRefCounterMemory)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2(sPtr1);

        EXPECT_EQ(sPtr1.refCount(), 2);
        sPtr1.reset();
        EXPECT_EQ(sPtr2.refCount(), 1);
    }

    TEST(SharedPtrTestOperatorAsterisk, OperatorAsteriskReturnsValue)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        EXPECT_EQ(*sPtr, 42);
    }

    TEST(SharedPtrTestOperatorAsterisk, OperatorAsteriskThrowsIfNullptr)
    {
        SPTR::SharedPtr<int> sPtr;
        EXPECT_THROW(*sPtr, std::runtime_error);
    }

    TEST(SharedPtrTestOperatorAsterisk, OperatorAsteriskCanModifyValue)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        *sPtr = 100;
        EXPECT_EQ(*sPtr, 100);
    }

    TEST(SharedPtrTestOperatorAsterisk, OperatorAsteriskSharesValues)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2(sPtr1);

        *sPtr1 = 100;
        EXPECT_EQ(*sPtr2, 100);
    }

    TEST(SharedPtrTestOperatorAsterisk, OperatorAsteriskCustomObject)
    {
        struct Point { int x = 0; int y = 0; };
        SPTR::SharedPtr<Point> sPtr(new Point{ 5, 10 });
        EXPECT_EQ((*sPtr).x, 5);
        EXPECT_EQ((*sPtr).y, 10);
    }

    TEST(SharedPtrTestOperatorArrow, OperatorArrowReturnsPointer)
    {
        struct Point { int x = 0; int y = 0; };
        SPTR::SharedPtr<Point> sPtr(new Point{ 5, 10 });
        EXPECT_EQ(sPtr->x, 5);
        EXPECT_EQ(sPtr->y, 10);
    }

    TEST(SharedPtrTestOperatorArrow, OperatorArrowThrowsIfNullptr)
    {
        struct Point { int x = 0; int y = 0; };
        SPTR::SharedPtr<Point> sPtr;
        EXPECT_THROW(sPtr->x, std::runtime_error);
    }

    TEST(SharedPtrTestOperatorArrow, OperatorArrowCanModifyObject)
    {
        struct Point { int x = 0; int y = 0; };
        SPTR::SharedPtr<Point> sPtr(new Point{ 5, 10 });
        sPtr->x = 100;
        sPtr->y = 200;
        EXPECT_EQ(sPtr->x, 100);
        EXPECT_EQ(sPtr->y, 200);
    }

    TEST(SharedPtrTestOperatorArrow, OperatorArrowCallsMemberFunctions)
    {
        class Calculator
        {
        public:
            int add(int a, int b) { return a + b; }
            int multiply(int a, int b) { return a * b; }
        };

        SPTR::SharedPtr<Calculator> sPtr(new Calculator());
        EXPECT_EQ(sPtr->add(5, 3), 8);
        EXPECT_EQ(sPtr->multiply(5, 3), 15);
    }

    TEST(SharedPtrTestOperatorSubscript, OperatorSubscriptAccessesArray)
    {
        SPTR::SharedPtr<int[]> sPtr(new int[5]{1, 2, 3, 4, 5});
        EXPECT_EQ(sPtr[0], 1);
        EXPECT_EQ(sPtr[2], 3);
        EXPECT_EQ(sPtr[4], 5);
    }
    
    TEST(SharedPtrTestOperatorSubscript, OperatorSubscriptCanModifyElements)
    {
        SPTR::SharedPtr<int[]> sPtr(new int[5]{1, 2, 3, 4, 5});
        sPtr[0] = 10;
        sPtr[2] = 30;
        EXPECT_EQ(sPtr[0], 10);
        EXPECT_EQ(sPtr[2], 30);
    }

    TEST(SharedPtrTestReset, ResetDeletesResourceAndSetsNullptr)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);

        sPtr.reset();
        EXPECT_EQ(sPtr.get(), nullptr);
        EXPECT_EQ(Beacon::counter, 0);
    }

    TEST(SharedPtrTestReset, ResetDoesNothingOnNullptr)
    {
        SPTR::SharedPtr<int> sPtr;
        EXPECT_NO_THROW(sPtr.reset());
        EXPECT_EQ(sPtr.get(), nullptr);
    }

    TEST(SharedPtrTestReset, ResetDecrementRefCount)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);

        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 2);

        sPtr1.reset();
        EXPECT_EQ(sPtr1.get(), nullptr);
        EXPECT_EQ(sPtr2.refCount(), 1);
        EXPECT_EQ(Beacon::counter, 1); // Still alive because sPtr2 owns it
    }

    TEST(SharedPtrTestReset, ResetWithNewPointer)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);

        sPtr.reset(new Beacon());
        EXPECT_EQ(Beacon::counter, 1); // Old deleted, new created
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestReset, ResetWithNewPointerDeletesOldResource)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);

        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 2);

        sPtr1.reset(new Beacon());
        EXPECT_EQ(Beacon::counter, 2); // New Beacon created
        EXPECT_EQ(sPtr2.refCount(), 1);
        EXPECT_EQ(sPtr1.refCount(), 1);
    }

    TEST(SharedPtrTestReset, ResetWithNullptrDeletesResource)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);

        sPtr.reset(nullptr);
        EXPECT_EQ(sPtr.get(), nullptr);
        EXPECT_EQ(Beacon::counter, 0);
    }

    TEST(SharedPtrTestRefCount, RefCountReturnsCorrectValue)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        EXPECT_EQ(sPtr1.refCount(), 1);

        SPTR::SharedPtr<int> sPtr2(sPtr1);
        EXPECT_EQ(sPtr1.refCount(), 2);
        EXPECT_EQ(sPtr2.refCount(), 2);

        SPTR::SharedPtr<int> sPtr3(sPtr1);
        EXPECT_EQ(sPtr1.refCount(), 3);
        EXPECT_EQ(sPtr2.refCount(), 3);
        EXPECT_EQ(sPtr3.refCount(), 3);
    }

    TEST(SharedPtrTestRefCount, RefCountAfterReset)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2(sPtr1);

        EXPECT_EQ(sPtr1.refCount(), 2);
        sPtr1.reset();
        EXPECT_EQ(sPtr2.refCount(), 1);
    }

    TEST(SharedPtrTestGet, GetReturnsRawPointer)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr(valuePtr);
        EXPECT_EQ(sPtr.get(), valuePtr);
    }

    TEST(SharedPtrTestGet, GetReturnsNullptrForDefaultConstructed)
    {
        SPTR::SharedPtr<int> sPtr;
        EXPECT_EQ(sPtr.get(), nullptr);
    }

    TEST(SharedPtrTestGet, GetReturnsNullptrAfterReset)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        sPtr.reset();
        EXPECT_EQ(sPtr.get(), nullptr);
    }

    TEST(SharedPtrTestEqualityOperator, EqualityReturnsTrueForSamePointer)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2(sPtr1);

        EXPECT_TRUE(sPtr1 == sPtr2);
        EXPECT_FALSE(sPtr1 != sPtr2);
    }

    TEST(SharedPtrTestEqualityOperator, EqualityReturnsFalseForDifferentPointers)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2(new int(42));

        EXPECT_FALSE(sPtr1 == sPtr2);
        EXPECT_TRUE(sPtr1 != sPtr2);
    }

    TEST(SharedPtrTestEqualityOperator, EqualityReturnsTrueForBothNullptr)
    {
        SPTR::SharedPtr<int> sPtr1;
        SPTR::SharedPtr<int> sPtr2;

        EXPECT_TRUE(sPtr1 == sPtr2);
        EXPECT_FALSE(sPtr1 != sPtr2);
    }

    TEST(SharedPtrComparisonOperators, ComparisonReturnsTrueIfEqualAndFalseIfNot)
    {
        SPTR::SharedPtr<int> uPtr1 = SPTR::makeShared<int>(42);
        SPTR::SharedPtr<int> uPtr2 = SPTR::makeShared<int>(42);
        EXPECT_TRUE(uPtr1 == uPtr1); // comparing with itself
        EXPECT_FALSE(uPtr1 == uPtr2); // comparing with another unique_ptr
        EXPECT_FALSE(uPtr1 != uPtr1); // comparing with itself
        EXPECT_TRUE(uPtr1 != uPtr2); // comparing with another unique_ptr
        EXPECT_TRUE(uPtr1 < uPtr2 || uPtr2 < uPtr1); // comparing addresses, one should be less than the other
        EXPECT_TRUE(uPtr1 > uPtr2 || uPtr2 > uPtr1);
        EXPECT_TRUE(uPtr1 <= uPtr2 || uPtr2 <= uPtr1);
        EXPECT_TRUE(uPtr1 >= uPtr2 || uPtr2 >= uPtr1);
    }

    TEST(SharedPtrTestMakeShared, MakeSharedCreatesAndReturnsSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr = SPTR::makeShared<int>(42);
        EXPECT_NE(sPtr.get(), nullptr);
        EXPECT_EQ(*sPtr, 42);
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestMakeShared, MakeSharedWithCustomObject)
    {
        struct Point { int x; int y; };
        SPTR::SharedPtr<Point> sPtr = SPTR::makeShared<Point>(5, 10);
        EXPECT_EQ(sPtr->x, 5);
        EXPECT_EQ(sPtr->y, 10);
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestMakeShared, MakeSharedWithString)
    {
        SPTR::SharedPtr<std::string> sPtr = SPTR::makeShared<std::string>("Hello, World!");
        EXPECT_EQ(*sPtr, "Hello, World!");
        EXPECT_EQ(sPtr.refCount(), 1);
    }

    TEST(SharedPtrTestMakeShared, MakeSharedCustomObjectLifetime)
    {
        {
            SPTR::SharedPtr<Beacon> sPtr = SPTR::makeShared<Beacon>();
            EXPECT_EQ(Beacon::counter, 1);
        }
        EXPECT_EQ(Beacon::counter, 0);
    }

    TEST(SharedPtrComplexScenarios, MultipleOwnersComplexLifetime)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 1);

        {
            SPTR::SharedPtr<Beacon> sPtr2(sPtr1);
            EXPECT_EQ(sPtr1.refCount(), 2);
            EXPECT_EQ(Beacon::counter, 1);

            {
                SPTR::SharedPtr<Beacon> sPtr3(sPtr2);
                EXPECT_EQ(sPtr1.refCount(), 3);
            }
            EXPECT_EQ(sPtr1.refCount(), 2);
            EXPECT_EQ(Beacon::counter, 1);
        }
        EXPECT_EQ(sPtr1.refCount(), 1);
        EXPECT_EQ(Beacon::counter, 1);
    }

    TEST(SharedPtrComplexScenarios, ReassignmentWithMultipleOwners)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);

        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 2);

        sPtr1 = SPTR::makeShared<Beacon>();
        EXPECT_EQ(Beacon::counter, 2); // Two Beacons now
        EXPECT_EQ(sPtr1.refCount(), 1);
        EXPECT_EQ(sPtr2.refCount(), 1);
    }

    TEST(SharedPtrComplexScenarios, CycleOfCopyAndMove)
    {
        int* valuePtr = new int(42);
        SPTR::SharedPtr<int> sPtr1(valuePtr);

        SPTR::SharedPtr<int> sPtr2 = sPtr1;
        EXPECT_EQ(sPtr1.refCount(), 2);

        SPTR::SharedPtr<int> sPtr3 = std::move(sPtr2);
        EXPECT_EQ(sPtr2.get(), nullptr);
        EXPECT_EQ(sPtr1.refCount(), 2);
        EXPECT_EQ(sPtr3.refCount(), 2);

        sPtr1 = sPtr3;
        EXPECT_EQ(sPtr1.refCount(), 2);
    }

    TEST(SharedPtrComplexScenarios, VectorOfSharedPtrs)
    {
        std::vector<SPTR::SharedPtr<Beacon>> ptrs;

        SPTR::SharedPtr<Beacon> sPtr1 = SPTR::makeShared<Beacon>();
        ptrs.push_back(sPtr1);
        ptrs.push_back(sPtr1);
        ptrs.push_back(sPtr1);

        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(sPtr1.refCount(), 4);

        ptrs.clear();
        EXPECT_EQ(Beacon::counter, 1); // vector cleared but sPtr1 still owns the Beacon
    }

    TEST(SharedPtrComplexScenarios, SharedPtrInContainer)
    {
        {
            std::vector<SPTR::SharedPtr<int>> vec;
            vec.push_back(SPTR::makeShared<int>(1));
            vec.push_back(SPTR::makeShared<int>(2));
            vec.push_back(SPTR::makeShared<int>(3));

            EXPECT_EQ(*vec[0], 1);
            EXPECT_EQ(*vec[1], 2);
            EXPECT_EQ(*vec[2], 3);
        }
    }
} // end of SharedPtrTests
