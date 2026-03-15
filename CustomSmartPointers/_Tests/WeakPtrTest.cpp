#include "pch.h"

#include "../Pointers/WeakPtr.h"

namespace WeakPtrTests
{
    struct Beacon
    {
        static inline int counter = 0;
        Beacon() { ++counter; }
        ~Beacon() { --counter; }
    };

    TEST(WeakPtrTestCtor, DefaultCtorCreatesNullWeakPtr)
    {
        SPTR::WeakPtr<int> wPtr;
        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestCtor, DefaultCtorWithNullptrIsValid)
    {
        SPTR::WeakPtr<int> wPtr(nullptr);
        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestCtor, DefaultCtorDoesNotThrow)
    {
        EXPECT_NO_THROW(SPTR::WeakPtr<int> wPtr);
    }

    TEST(WeakPtrTestCtorFromWeakPtr, ConstructFromOtherValidWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(wPtr1);

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestCtorFromWeakPtr, ConstructFromOtherNullptrWeakPtr)
    {
        SPTR::WeakPtr<int> wPtr1(nullptr);
        SPTR::WeakPtr<int> wPtr2(wPtr1);

        EXPECT_TRUE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 0);
    }

    // std::weak_ptr<T> behavior allows creation pointers from pointers 
    // with different though compatible types, however, compatibility 
    // is meant in terms of inheritance because pointer-to-types
    // should be compartible not the value-types (pointees) themselves
    TEST(WeakPtrTestCtorFromWeakPtr, ConstructFromOtherWeakPtrWithCompatibleType)
    {
        struct Base { virtual ~Base() = default; };
        struct Derived : Base {};

        SPTR::SharedPtr<Derived> sPtr(new Derived);
        SPTR::WeakPtr<Derived> wPtr1(sPtr);
        SPTR::WeakPtr<Base> wPtr2(wPtr1);

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestCtorFromWeakPtr, ConstructFromOtherWeakPtrWithIncompatibleType)
    {
        SPTR::SharedPtr<std::byte> sPtr(new std::byte());
        SPTR::WeakPtr<std::byte> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(wPtr1);

        EXPECT_TRUE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 0);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructFromSharedPtrWithValidPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructFromSharedPtrWithNullptr)
    {
        SPTR::SharedPtr<int> sPtr;
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructFromSharedPtrWithCustomObject)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        SPTR::WeakPtr<Beacon> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);
        EXPECT_EQ(Beacon::counter, 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, MultipleWeakPtrsFromSameSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(sPtr);

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, SingleWeakPtrFromMultipleSharedPtrs)
    {
        SPTR::SharedPtr<int> sPtr1(new int(42));
        SPTR::SharedPtr<int> sPtr2 = sPtr1;
        SPTR::WeakPtr<int> wPtr(sPtr1);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 2);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructFromSharedPtrWithCompatibleType)
    {
        struct Base { virtual ~Base() = default; };
        struct Derived : Base {};

        SPTR::SharedPtr<Derived> sPtr(new Derived());
        SPTR::WeakPtr<Base> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructFromSharedPtrWithIncompatibleType)
    {
        SPTR::SharedPtr<std::byte> sPtr(new std::byte());
        SPTR::WeakPtr<Beacon> wPtr(sPtr);

        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructorWithValidSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(100));
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructorWithNullSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr;
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructorDoesNotIncreaseSharedRefCount)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        int refCountBefore = sPtr.refCount();

        SPTR::WeakPtr<int> wPtr(sPtr);
        int refCountAfter = sPtr.refCount();

        EXPECT_EQ(refCountBefore, refCountAfter);
        EXPECT_EQ(refCountAfter, 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructorSequentialCreation)
    {
        SPTR::SharedPtr<int> sPtr(new int(555));

        SPTR::WeakPtr<int> wPtr1(sPtr);
        EXPECT_FALSE(wPtr1.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);

        SPTR::WeakPtr<int> wPtr2(sPtr);
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);

        SPTR::WeakPtr<int> wPtr3(sPtr);
        EXPECT_FALSE(wPtr3.expired());
        EXPECT_EQ(wPtr3.useCount(), 1);
    }

    TEST(WeakPtrTestConstructionFromSharedPtr, ConstructorAfterSharedPtrCopy)
    {
        SPTR::SharedPtr<int> sPtr1(new int(777));
        SPTR::SharedPtr<int> sPtr2 = sPtr1;

        SPTR::WeakPtr<int> wPtr(sPtr1);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 2);
    }

    TEST(WeakPtrTestCopyCtor, CopyCtorFromAnotherWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(wPtr1);

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
        EXPECT_EQ(wPtr1.useCount(), 1);
    }

    TEST(WeakPtrTestCopyCtor, CopyCtorFromDefaultConstructedWeakPtr)
    {
        SPTR::WeakPtr<int> wPtr1;
        SPTR::WeakPtr<int> wPtr2(wPtr1);

        EXPECT_TRUE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 0);
    }

    TEST(WeakPtrTestCopyCtor, MultipleWeakPtrCopies)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(wPtr1);
        SPTR::WeakPtr<int> wPtr3(wPtr2);

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_FALSE(wPtr3.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);
        EXPECT_EQ(wPtr2.useCount(), 1);
        EXPECT_EQ(wPtr3.useCount(), 1);
    }

    TEST(WeakPtrTestMoveCtor, MoveCtorFromAnotherWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(std::move(wPtr1));

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestMoveCtor, MoveCtorFromDefaultConstructedWeakPtr)
    {
        SPTR::WeakPtr<int> wPtr1;
        SPTR::WeakPtr<int> wPtr2(std::move(wPtr1));

        EXPECT_TRUE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 0);
    }

    TEST(WeakPtrTestMoveCtor, MoveCtorFromCompatibleType)
    {
        struct Base { virtual ~Base() = default; };
        struct Derived : Base {};

        SPTR::SharedPtr<Derived> sPtr(new Derived);
        SPTR::WeakPtr<Derived> wPtr1(sPtr);

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);

        SPTR::WeakPtr<Base> wPtr2(std::move(wPtr1));
        
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestMoveCtor, MoveCtorFromIncompatibleType)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr1(sPtr);
    
        EXPECT_FALSE(wPtr1.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);
    
        SPTR::WeakPtr<short> wPtr2(std::move(wPtr1));
    
        EXPECT_FALSE(wPtr1.expired()); // wPtr1 was not moved
        EXPECT_EQ(wPtr1.useCount(), 1); // same here
        EXPECT_TRUE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 0);
    }

    TEST(WeakPtrTestCopyAssignment, CopyAssignmentFromSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(99));
        SPTR::WeakPtr<int> wPtr;

        wPtr = sPtr;

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestCopyAssignment, CopyAssignmentFromAnotherWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(33));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2;

        wPtr2 = wPtr1;

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
        EXPECT_EQ(wPtr1.useCount(), 1);
    }

    TEST(WeakPtrTestCopyAssignment, CopyAssignmentReplacesExistingWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr1(new int(11));
        SPTR::SharedPtr<int> sPtr2(new int(22));

        SPTR::WeakPtr<int> wPtr(sPtr1);
        EXPECT_EQ(wPtr.useCount(), 1);

        wPtr = sPtr2;
        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestMoveAssignment, MoveAssignmentFromAnotherWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(44));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2;

        wPtr2 = std::move(wPtr1);

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestMoveAssignment, MoveAssignmentFromDefaultConstructed)
    {
        SPTR::SharedPtr<int> sPtr(new int(88));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2;

        wPtr2 = std::move(wPtr1);

        EXPECT_FALSE(wPtr2.expired());
        EXPECT_EQ(wPtr2.useCount(), 1);
    }

    TEST(WeakPtrTestLock, LockReturnsSharedPtrWhenValidPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(66));
        SPTR::WeakPtr<int> wPtr(sPtr);

        SPTR::SharedPtr<int> lockedPtr = wPtr.lock();

        EXPECT_NE(lockedPtr.get(), nullptr);
        EXPECT_EQ(*lockedPtr, 66);
        EXPECT_EQ(lockedPtr.refCount(), 2);
    }

    TEST(WeakPtrTestLock, LockReturnsNullSharedPtrIfCreatedFromNullptr)
    {
        SPTR::WeakPtr<int> wPtr;

        SPTR::SharedPtr<int> lockedPtr = wPtr.lock();

        EXPECT_EQ(lockedPtr.get(), nullptr);
        EXPECT_EQ(lockedPtr.refCount(), 1);
    }

    TEST(WeakPtrTestLock, LockReturnsNullSharedPtrWhenOwnerDestroyed)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        SPTR::WeakPtr<Beacon> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(Beacon::counter, 1);
        EXPECT_EQ(wPtr.useCount(), 1);

        sPtr.reset();

        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(Beacon::counter, 0);

        SPTR::SharedPtr<Beacon> lockedPtr = wPtr.lock();
        EXPECT_EQ(lockedPtr.get(), nullptr);
    }

    TEST(WeakPtrTestLock, MultipleLockCallsOnSameWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(123));
        SPTR::WeakPtr<int> wPtr(sPtr);

        SPTR::SharedPtr<int> locked1 = wPtr.lock();
        SPTR::SharedPtr<int> locked2 = wPtr.lock();

        EXPECT_EQ(*locked1, *locked2);
        EXPECT_EQ(locked1.get(), locked2.get());
    }

    TEST(WeakPtrTestExpired, ExpiredReturnsTrueForDefaultConstructed)
    {
        SPTR::WeakPtr<int> wPtr;

        EXPECT_TRUE(wPtr.expired());
    }

    TEST(WeakPtrTestExpired, ExpiredReturnsFalseForValidPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
    }

    TEST(WeakPtrTestExpired, ExpiredReturnsTrueAfterOwnerDestroyed)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        SPTR::WeakPtr<Beacon> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());

        sPtr.reset();

        EXPECT_TRUE(wPtr.expired());
    }

    TEST(WeakPtrTestExpired, ExpiredReturnsTrueWhenAllOwnersCopiesDestroyed)
    {
        SPTR::SharedPtr<Beacon> sPtr1(new Beacon());
        SPTR::SharedPtr<Beacon> sPtr2(sPtr1);
        SPTR::WeakPtr<Beacon> wPtr(sPtr1);

        EXPECT_FALSE(wPtr.expired());

        sPtr1.reset();
        EXPECT_FALSE(wPtr.expired());

        sPtr2.reset();
        EXPECT_TRUE(wPtr.expired());
    }

    TEST(WeakPtrTestExpired, ExpiredReturnsFalseForMultipleWeakPtrs)
    {
        SPTR::SharedPtr<int> sPtr(new int(99));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(sPtr);
        SPTR::WeakPtr<int> wPtr3(sPtr);

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_FALSE(wPtr3.expired());
    }

    TEST(WeakPtrTestUseCount, UseCountReturnsZeroForDefaultConstructed)
    {
        SPTR::WeakPtr<int> wPtr;

        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestUseCount, UseCountReturnsOneForValidPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_EQ(wPtr.useCount(), 1);
    }

    TEST(WeakPtrTestUseCount, UseCountIncrementsWithSharedPtrCopies)
    {
        SPTR::SharedPtr<int> sPtr1(new int(55));
        SPTR::WeakPtr<int> wPtr(sPtr1);

        EXPECT_EQ(wPtr.useCount(), 1);

        SPTR::SharedPtr<int> sPtr2(sPtr1);
        EXPECT_EQ(wPtr.useCount(), 2);

        SPTR::SharedPtr<int> sPtr3(sPtr1);
        EXPECT_EQ(wPtr.useCount(), 3);
    }

    TEST(WeakPtrTestUseCount, UseCountDecrementsWhenSharedPtrDestroyed)
    {
        SPTR::SharedPtr<int> sPtr1(new int(77));
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        SPTR::WeakPtr<int> wPtr(sPtr1);

        EXPECT_EQ(wPtr.useCount(), 2);

        sPtr1.reset();
        EXPECT_EQ(wPtr.useCount(), 1);

        sPtr2.reset();
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestUseCount, UseCountIsIndependentOfWeakPtrCount)
    {
        SPTR::SharedPtr<int> sPtr(new int(88));
        SPTR::WeakPtr<int> wPtr1(sPtr);
        SPTR::WeakPtr<int> wPtr2(sPtr);
        SPTR::WeakPtr<int> wPtr3(sPtr);

        EXPECT_EQ(wPtr1.useCount(), 1);
        EXPECT_EQ(wPtr2.useCount(), 1);
        EXPECT_EQ(wPtr3.useCount(), 1);
    }

    TEST(WeakPtrTestUseCount, UseCountZeroAfterAllOwnersCopiesDestroyed)
    {
        SPTR::SharedPtr<int> sPtr1(new int(99));
        SPTR::SharedPtr<int> sPtr2(sPtr1);
        SPTR::WeakPtr<int> wPtr(sPtr1);

        EXPECT_EQ(wPtr.useCount(), 2);

        sPtr1.reset();
        EXPECT_EQ(wPtr.useCount(), 1);

        sPtr2.reset();
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestReset, ResetClearsWeakPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(42));
        SPTR::WeakPtr<int> wPtr(sPtr);

        EXPECT_FALSE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 1);

        wPtr.reset();

        EXPECT_TRUE(wPtr.expired());
        EXPECT_EQ(wPtr.useCount(), 0);
    }

    TEST(WeakPtrTestReset, ResetDoesNotAffectSharedPtr)
    {
        SPTR::SharedPtr<int> sPtr(new int(55));
        SPTR::WeakPtr<int> wPtr(sPtr);

        wPtr.reset();

        EXPECT_NE(sPtr.get(), nullptr);
        EXPECT_EQ(*sPtr, 55);
    }

    TEST(WeakPtrTestReset, ResetOnDefaultConstructedWeakPtr)
    {
        SPTR::WeakPtr<int> wPtr;

        EXPECT_NO_THROW(wPtr.reset());
        EXPECT_TRUE(wPtr.expired());
    }

    TEST(WeakPtrTestReset, ResetWithCustomObjectDoesNotCallDestructor)
    {
        SPTR::SharedPtr<Beacon> sPtr(new Beacon());
        SPTR::WeakPtr<Beacon> wPtr(sPtr);

        EXPECT_EQ(Beacon::counter, 1);

        wPtr.reset();

        EXPECT_EQ(Beacon::counter, 1);
    }

    TEST(WeakPtrTestDestructor, DestructorDoesNotDeleteObject)
    {
        {
            SPTR::SharedPtr<Beacon> sPtr(new Beacon());
            SPTR::WeakPtr<Beacon> wPtr(sPtr);
            EXPECT_EQ(Beacon::counter, 1);
        }

        EXPECT_EQ(Beacon::counter, 0);
    }

    TEST(WeakPtrTestDestructor, DestructorWithoutSharedPtr)
    {
        {
            SPTR::WeakPtr<int> wPtr;
        }

        EXPECT_NO_THROW(::testing::Test::RecordProperty("Test", "Passed"));
    }

    TEST(WeakPtrTestComplexScenarios, CyclicalReferencePrevention)
    {
        struct Node
        {
            int value;
            SPTR::SharedPtr<Node> next;
            SPTR::WeakPtr<Node> prev;

            Node(int val) : value(val) {}
        };

        SPTR::SharedPtr<Node> node1(new Node(1));
        SPTR::SharedPtr<Node> node2(new Node(2));

        node1->next = node2;
        node2->prev = node1;

        EXPECT_FALSE(node2->prev.expired());
        EXPECT_EQ(node2->prev.useCount(), 1);
    }

    TEST(WeakPtrTestComplexScenarios, MixedSharedAndWeakPtrOps)
    {
        SPTR::SharedPtr<int> sPtr1(new int(100));
        SPTR::WeakPtr<int> wPtr1(sPtr1);

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_EQ(wPtr1.useCount(), 1);

        SPTR::SharedPtr<int> sPtr2(sPtr1);
        EXPECT_EQ(wPtr1.useCount(), 2);

        SPTR::WeakPtr<int> wPtr2(sPtr1);
        EXPECT_EQ(wPtr1.useCount(), 2);

        sPtr1.reset();
        EXPECT_EQ(wPtr1.useCount(), 1);

        sPtr2.reset();
        EXPECT_TRUE(wPtr1.expired());
        EXPECT_TRUE(wPtr2.expired());
    }

    TEST(WeakPtrTestComplexScenarios, AssignmentChain)
    {
        SPTR::SharedPtr<int> sPtr(new int(200));
        SPTR::WeakPtr<int> wPtr1, wPtr2, wPtr3;

        wPtr1 = sPtr;
        wPtr2 = wPtr1;
        wPtr3 = wPtr2;

        EXPECT_FALSE(wPtr1.expired());
        EXPECT_FALSE(wPtr2.expired());
        EXPECT_FALSE(wPtr3.expired());

        EXPECT_EQ(wPtr1.useCount(), 1);
        EXPECT_EQ(wPtr2.useCount(), 1);
        EXPECT_EQ(wPtr3.useCount(), 1);
    }

} // end of WeakPtrTests