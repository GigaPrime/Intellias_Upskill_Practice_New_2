#include "../src/Optional.h"

#include "../packages/googletest/include/gtest/gtest.h"

namespace VT
{
	struct Tracker
	{
		int ctorCounter_ = 0;
		int dtorCounter_ = 0;
		int copyctorCounter_ = 0;
		int movectorCounter_ = 0;
		int copyAssignCounter_ = 0;
		int moveAssignCount_ = 0;
		int aliveCounter_ = 0;

		int value = 0;

		void reset()
		{
			ctorCounter_ = 0;
			dtorCounter_ = 0;
			copyctorCounter_ = 0;
			movectorCounter_ = 0;
			copyAssignCounter_ = 0;
			moveAssignCount_ = 0;
			aliveCounter_ = 0;
		}

		Tracker(int v = 0) : value(v)
		{
			++ctorCounter_;
			++aliveCounter_;
		}

		Tracker(const Tracker& other) : value(other.value)
		{
			++copyctorCounter_;
			++aliveCounter_;
		}

		Tracker(Tracker&& other) noexcept : value(other.value)
		{
			other.value = -1;
			++movectorCounter_;
			++aliveCounter_;
		}

		Tracker& operator=(const Tracker& other)
		{
			value = other.value;
			++copyAssignCounter_;
			return *this;
		}

		Tracker& operator=(Tracker&& other) noexcept
		{
			value = other.value;
			other.value = -1;
			++moveAssignCount_;
			return *this;
		}

		~Tracker()
		{
			++dtorCounter_;
			--aliveCounter_;
		}
	};

	TEST(OptionalTest, DefaultConstructedOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> opt;
			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, NullOptionalConstructorDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> opt(VT::NullOptionalType::getNullOptionalType());
			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyConstructEmptyOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst(src);

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.copyctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, MoveConstructEmptyOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst(std::move(src));

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.movectorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, ConvertingCopyConstructEmptyOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<int> src;
			VT::Optional<Tracker> dst(src);

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.copyctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, ConvertingMoveConstructEmptyOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<int> src;
			VT::Optional<Tracker> dst(std::move(src));

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.movectorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, AssignNullToEmptyOptionalDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> opt;
			opt = VT::NullOptionalType::getNullOptionalType();

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.ctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyAssignEmptyToEmptyDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst;

			dst = src;

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.copyAssignCounter_, 0);
			EXPECT_EQ(tracker.copyctorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, MoveAssignEmptyToEmptyDoesNothing)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> src;
			VT::Optional<Tracker> dst;

			dst = std::move(src);

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.moveAssignCount_, 0);
			EXPECT_EQ(tracker.movectorCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, SelfCopyAssignEmptyIsSafe)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> opt;
			opt = opt;

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.copyAssignCounter_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}

	TEST(OptionalTest, SelfMoveAssignEmptyIsSafe)
	{
		Tracker tracker;
		tracker.reset();
		{
			VT::Optional<Tracker> opt;
			opt = std::move(opt);

			EXPECT_EQ(tracker.aliveCounter_, 0);
			EXPECT_EQ(tracker.moveAssignCount_, 0);
			EXPECT_EQ(tracker.dtorCounter_, 0);
		}
		EXPECT_EQ(tracker.aliveCounter_, 0);
		EXPECT_EQ(tracker.dtorCounter_, 0);
	}
}