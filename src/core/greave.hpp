// core/greave.hpp -- Universal header which includes very commonly-used system libraries and #defines.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once

// C library definitions.
#define __STDC_LIMIT_MACROS
#define _USE_MATH_DEFINES
#ifdef GREAVE_TARGET_WINDOWS
#define WIN32_LEAN_AND_MEAN
#if not(defined _WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif  // GREAVE_TARGET_WINDOWS
#endif  // _WIN32_WINNT

// Very commonly-used system libraries.
#include <climits>  // for handy definitions like UINT_MAX
#include <cmath>    // basic math functions
#include <cstdint>  // for integer typedefs
#include <cstdlib>  // more handy basic functions
#include <cstring>  // many useful string/array functions
#include <map>      // std::map comes in handy often
#include <memory>   // for std::shared_ptr
#include <set>      // for std::set, of course
#include <string>   // good old std::string
#include <vector>   // std::vector is also our friend
