//
// pch.h
//

#pragma once

#if defined(__has_include)
  #if __has_include(<gmock/gmock.h>)
    #include <gmock/gmock.h>
  #elif __has_include(<gtest/gtest.h>)
    #include <gtest/gtest.h>
  #endif
#else
  #include "gmock/gmock.h"
  #include "gtest/gtest.h"
#endif
