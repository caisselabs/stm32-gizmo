//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace i2c {
namespace detail {
struct driver {
    auto write(span) {}
    auto receive(span) {}
};
} // namespace detail
} // namespace i2c
