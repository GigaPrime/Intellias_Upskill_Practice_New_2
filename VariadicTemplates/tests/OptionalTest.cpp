#include "../src/Optional.h"

#include "../packages/googletest/include/gtest/gtest.h"

namespace VT
{
	struct Tracker
	{
		static inline int ctorCounter_ = 0;
		static inline int dtorCounter_ = 0;
		static inline int copyctorCounter_ = 0;
		static inline int movectorCounter_ = 0;
		static inline int copyAssignCounter_ = 0;
		static inline int moveAssignCount_ = 0;
		static inline int aliveCounter_ = 0;

		int value = 0;

		static void Reset()
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
		Tracker::Reset();
		{
			VT::Optional<Tracker> opt;
			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, NullOptionalConstructorDoesNothing)
	{
		Tracker::Reset();
		{
			VT::Optional<Tracker> opt(VT::NullOptionalType::getNullOptionalType());
			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyConstructEmptyOptionalDoesNothing)
	{
		Tracker::Reset();
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
		Tracker::Reset();
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
		Tracker::Reset();
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
		Tracker::Reset();
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
		Tracker::Reset();
		{
			VT::Optional<Tracker> opt;
			opt = VT::NullOptionalType::getNullOptionalType();

			EXPECT_EQ(Tracker::aliveCounter_, 0);
			EXPECT_EQ(Tracker::ctorCounter_, 0);
			EXPECT_EQ(Tracker::dtorCounter_, 0);
		}
		EXPECT_EQ(Tracker::aliveCounter_, 0);
		EXPECT_EQ(Tracker::dtorCounter_, 0);
	}

	TEST(OptionalTest, CopyAssignEmptyToEmptyDoesNothing)
	{
		Tracker::Reset();
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
		Tracker::Reset();
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
		Tracker::Reset();
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
		Tracker::Reset();
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
}