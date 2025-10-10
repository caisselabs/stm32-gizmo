//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------------------
#pragma once

#include "shared/stm32_interrupt.hpp"

#include <stdx/concepts.hpp>
#include <conc/concurrency.hpp>


// ----------------------------------------------------------------------------
// concurrency policy
namespace shared {
struct concurrency_policy {
    template <typename = void, stdx::invocable F, stdx::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, auto &&...pred) -> decltype(auto) {
        while (true) {
            shared::disable_interrupts_lock lock{};
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
        }
    }
};
} // namespace shared


template <> inline auto conc::injected_policy<> = shared::concurrency_policy{};
// ----------------------------------------------------------------------------
