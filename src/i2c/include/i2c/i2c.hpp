//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace i2c {

struct device {
    auto send(msg) -> sender{};

    auto receive(msg) -> sender{};
};
} // namespace i2c
