//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------------------
//
// This is the "spinning" version that simply polls for the interrupt status
// bit to be set.
//
#pragma once
#include "shared/concurrency.hpp"

#include <groov/groov.hpp>

#include <async/just.hpp>
#include <async/repeat.hpp>
#include <async/sequence.hpp>
#include <async/sync_wait.hpp>
#include <async/then.hpp>

#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

namespace button {
using namespace groov::literals;
namespace stm32 = caisselabs::stm32;

// clang-format off
template <stdx::ct_string BitName>
auto spin_for_flag() {
    using stdx::literals::operator""_cts;
    constexpr auto field = groov::path<"isr"_cts, BitName>{};
    return
          groov::read(stm32::i2c1 / field)
        | async::repeat_until([](auto v) { return v == true; })
        ;
}

auto set_brightness(std::uint8_t bright) -> async::sender auto {

    auto setup_controller =
        groov::write(
          stm32::i2c1(
            "cr2.ADD10"_f = groov::disable,
            "cr2.SADD7"_f = 0x6f,
            "cr2.RD_WRN"_f = stm32::i2c::rd_wrn::WRITE_XFER,
            "cr2.NBYTES"_f = 2,
            "cr2.RELOAD"_f = groov::disable,
            "cr2.AUTOEND"_f = groov::enable,
            "cr2.START"_f = true
          ))
        | seq(spin_for_flag<"TXIS">())
        ;

    auto write_address =
          groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = 0x19 // LED Brightness address
          ))
        | seq(spin_for_flag<"TXIS">())
        ;

    auto write_data = 
          groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = bright
          ))
        | seq(spin_for_flag<"STOPF">())
        | seq(groov::write(
            stm32::i2c1(
              "icr.STOPCF"_f = groov::set
          )))
        ;

    return
          setup_controller
        | seq(write_address)
        | seq(write_data)
        ;
}
// clang-format on
} // namespace button
