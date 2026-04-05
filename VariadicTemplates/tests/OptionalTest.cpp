#include "../src/Optional.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace VT
{
	struct Tracker
	{
		static int ctorCounter_;
		static int dtorCounter_;
		static int copyctorCounter_;
		static int movectorCounter_;
		static int copyAssignCounter_;
		static int moveAssignCount_;
		static int aliveCounter_;

		static void reset()
		{
			ctorCounter_ = 0;
			dtorCounter_ = 0;
			copyctorCounter_ = 0;
			movectorCounter_ = 0;
			copyAssignCounter_ = 0;
			moveAssignCount_ = 0;
			aliveCounter_ = 0;
		}

		int value_ = 0;

		Tracker(int value = 0) : value_(value)
		{
			++ctorCounter_;
			++aliveCounter_;
		}

		Tracker(const Tracker& other) : value_(other.value_)
		{
			++copyctorCounter_;
			++aliveCounter_;
		}

		Tracker(Tracker&& other) noexcept : value_(other.value_)
		{
			other.value_ = -1;
			++movectorCounter_;
			++aliveCounter_;
		}

		Tracker& operator=(const Tracker& other)
		{
			value_ = other.value_;
			++copyAssignCounter_;
			return *this;
		}

		Tracker& operator=(Tracker&& other) noexcept
		{
			value_ = other.value_;
			other.value_ = -1;
			++moveAssignCount_;
			return *this;
		}

		~Tracker()
		{
			++dtorCounter_;
			--aliveCounter_;
		}
	};

	int Tracker::ctorCounter_ = 0;
	int Tracker::dtorCounter_ = 0;
	int Tracker::copyctorCounter_ = 0;
	int Tracker::movectorCounter_ = 0;
	int Tracker::copyAssignCounter_ = 0;
	int Tracker::moveAssignCount_ = 0;
	int Tracker::aliveCounter_ = 0;

	TEST(OptionalTest, DefaultConstructedOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt;
			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, NullOptionalConstructorDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt(VT::NullOptionalType::getNullOptionalType());
			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyConstructEmptyOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst(src);

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, MoveConstructEmptyOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst(std::move(src));

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, ConvertingCopyConstructEmptyOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<int> src;
			VT::Optional<Tracker> dst(src);

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, ConvertingMoveConstructEmptyOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<int> src;
			VT::Optional<Tracker> dst(std::move(src));

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, AssignNullToEmptyOptionalDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt;
			opt = VT::NullOptionalType::getNullOptionalType();

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyAssignEmptyToEmptyDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst;

			dst = src;

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::copyAssignCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, MoveAssignEmptyToEmptyDoesNothing)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst;

			dst = std::move(src);

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::moveAssignCount_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, SelfCopyAssignEmptyIsSafe)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt;
			opt = opt;

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::copyAssignCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, SelfMoveAssignEmptyIsSafe)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt;
			opt = std::move(opt);

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::moveAssignCount_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, ForwardingConstructorConstructsContainedValueFromDirectArgs)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt(123);

			EXPECT_EQ(Tracker::ctorCounter_, 1);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::aliveCounter_, 1);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 1);
	}

	TEST(OptionalTest, ForwardingConstructorCopiesFromLvalueArgument)
	{
		Tracker source(55);
		Tracker::ctorCounter_ = 0;
		Tracker::dtorCounter_ = 0;
		Tracker::copyctorCounter_ = 0;
		Tracker::movectorCounter_ = 0;
		Tracker::copyAssignCounter_ = 0;
		Tracker::moveAssignCount_ = 0;
		Tracker::aliveCounter_ = 1; // source is alive

		{
			VT::Optional<Tracker> opt(source);

			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::copyctorCounter_, 1);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::aliveCounter_, 2);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}

		EXPECT_EQ(Tracker::aliveCounter_, 1);
		EXPECT_EQ(Tracker::dtorCounter_, 1);
	}

	TEST(OptionalTest, ForwardingConstructorMovesFromRvalueArgument)
	{
		Tracker::reset();
		{
			VT::Optional<Tracker> opt(Tracker(77));

			EXPECT_EQ(Tracker::ctorCounter_, 1);      // temporary Tracker(77)
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 1);  // moved into Optional storage
			EXPECT_EQ(Tracker::aliveCounter_, 1);     // temporary destroyed, Optional value alive
			EXPECT_EQ(Tracker::dtorCounter_, 1);      // temporary already destroyed
		}

		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 2);          // temporary + contained value
	}

	struct MultiArgTracker
	{
		static int ctorCounter_;
		static int dtorCounter_;
		static int aliveCounter_;

		static void reset()
		{
			ctorCounter_ = 0;
			dtorCounter_ = 0;
			aliveCounter_ = 0;
		}

		int first_ = 0;
		int second_ = 0;

		MultiArgTracker(int first, int second)
			: first_(first), second_(second)
		{
			++ctorCounter_;
			++aliveCounter_;
		}

		~MultiArgTracker()
		{
			++dtorCounter_;
			--aliveCounter_;
		}
	};

	int MultiArgTracker::ctorCounter_ = 0;
	int MultiArgTracker::dtorCounter_ = 0;
	int MultiArgTracker::aliveCounter_ = 0;

	TEST(OptionalTest, ForwardingConstructorSupportsMultipleArguments)
	{
		MultiArgTracker::reset();
		{
			VT::Optional<MultiArgTracker> opt(10, 20);

			EXPECT_EQ(MultiArgTracker::ctorCounter_, 1);
			EXPECT_EQ(MultiArgTracker::aliveCounter_, 1);
			EXPECT_EQ(MultiArgTracker::dtorCounter_, 0);
		}

		EXPECT_EQ(MultiArgTracker::aliveCounter_, 0);
		EXPECT_EQ(MultiArgTracker::dtorCounter_, 1);
	}

	TEST(OptionalTest, VariadicConstructorCopiesFromLvalueTracker)
	{
		Tracker::reset();
		Tracker source(42);

		EXPECT_EQ(Tracker::ctorCounter_, 1);
		EXPECT_EQ(Tracker::aliveCounter_, 1);

		{
			VT::Optional<Tracker> opt(source);

			EXPECT_EQ(Tracker::ctorCounter_, 1);
			EXPECT_EQ(Tracker::copyctorCounter_, 1);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::aliveCounter_, 2);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}

		EXPECT_EQ(Tracker::aliveCounter_, 1);
		EXPECT_EQ(Tracker::dtorCounter_, 1);
	}

	TEST(OptionalTest, VariadicConstructorMovesFromRvalueTracker)
	{
		Tracker::reset();

		{
			VT::Optional<Tracker> opt(Tracker(77));

			EXPECT_EQ(Tracker::ctorCounter_, 1);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 1);
			EXPECT_EQ(Tracker::dtorCounter_, 1);   // temporary destroyed
			EXPECT_EQ(Tracker::aliveCounter_, 1);  // only value in Optional remains alive
		}

		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 2);
	}

	TEST(OptionalTest, VariadicConstructorBuildsTrackerFromInt)
	{
		Tracker::reset();

		{
			VT::Optional<Tracker> opt(123);

			EXPECT_EQ(Tracker::ctorCounter_, 1);
			EXPECT_EQ(Tracker::copyctorCounter_, 0);
			EXPECT_EQ(Tracker::movectorCounter_, 0);
			EXPECT_EQ(Tracker::aliveCounter_, 1);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}

		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 1);
	}

	struct PairTracker
	{
		static int ctorCounter_;
		static int dtorCounter_;
		static int aliveCounter_;

		static void reset()
		{
			ctorCounter_ = 0;
			dtorCounter_ = 0;
			aliveCounter_ = 0;
		}

		int first_ = 0;
		int second_ = 0;

		PairTracker(int first, int second)
			: first_(first), second_(second)
		{
			++ctorCounter_;
			++aliveCounter_;
		}

		~PairTracker()
		{
			++dtorCounter_;
			--aliveCounter_;
		}
	};

	int PairTracker::ctorCounter_ = 0;
	int PairTracker::dtorCounter_ = 0;
	int PairTracker::aliveCounter_ = 0;

	TEST(OptionalTest, VariadicConstructorForwardsMultipleArguments)
	{
		PairTracker::reset();

		{
			VT::Optional<PairTracker> opt(10, 20);

			EXPECT_EQ(PairTracker::ctorCounter_, 1);
			EXPECT_EQ(PairTracker::aliveCounter_, 1);
			EXPECT_EQ(PairTracker::dtorCounter_, 0);
		}

		EXPECT_EQ(PairTracker::aliveCounter_, 0);
		EXPECT_EQ(PairTracker::dtorCounter_, 1);
	}
}