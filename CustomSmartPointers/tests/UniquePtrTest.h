#pragma once

#include "pch.h"

#include "../Pointers/UniquePtr.h"

template <typename T>
class UniquePtrTestFixtureBaseTypes : public ::testing::Test
{
protected:
	SPTR::UniquePtr<T> uPtr;
};

template <typename T>
class UniquePtrTestFixtureSTLTypes : public ::testing::Test
{
protected:
    SPTR::UniquePtr<T> uPtr;
};

template <typename T>
class UniquePtrTestFixtureComplexTypes : public ::testing::Test
{
protected:
    SPTR::UniquePtr<T> uPtr;
};

template <typename T>
class UniquePtrTestFixturePointerTypes : public ::testing::Test
{
protected:
    SPTR::UniquePtr<T> uPtr;
};

template <typename T>
class UniquePtrTestFixtureVoidAndNullptrTypes : public ::testing::Test
{
protected:
    SPTR::UniquePtr<T> uPtr;
};
