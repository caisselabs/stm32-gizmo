#pragma once
#include "shared/concurrency.hpp"

#include <groov/groov.hpp>

#include <async/continue_on.hpp>
#include <async/just.hpp>
#include <async/repeat.hpp>
#include <async/schedulers/trigger_scheduler.hpp>
#include <async/sequence.hpp>
#include <async/sync_wait.hpp>
#include <async/then.hpp>

#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

namespace button {
using namespace groov::literals;
namespace stm32 = caisselabs::stm32;

// clang-format off
    /*
template <stdx::ct_string BitName>
auto wait_for_flag() {
    using stdx::literals::operator""_cts;
    constexpr auto field = groov::path<"isr"_cts, BitName>{};
    return
          async::trigger_scheduler<"i2c1_ev">{}.schedule()
        | seq(groov::read(stm32::i2c1 / field))
        | async::repeat_until([](auto v) { return v == true; })
        ;
}
    */

template <stdx::ct_string BitName>
auto wait_for_flag() {
    using stdx::literals::operator""_cts;
    constexpr auto field = groov::path<"isr"_cts, BitName>{};
    using write_spec_t = decltype(groov::sync_read(stm32::i2c1 / "isr"_r));

    return
        async::trigger_scheduler<"i2c1_ev", write_spec_t>{}.schedule()
        | async::repeat_until([field](auto r) { return r[field] == true; })
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
        | async::seq(wait_for_flag<"TXIS">())
        ;

    auto write_address =
        groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = 0x19 // LED Brightness address
          ))
        | async::seq(wait_for_flag<"TXIS">())
        ;

    auto write_data = 
          groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = bright
          ))
        | async::seq(wait_for_flag<"STOPF">())
        | async::seq(groov::write(
            stm32::i2c1(
              "icr.STOPCF"_f = groov::set
          )))
        ;

    
    return
          setup_controller
        | async::seq(write_address)
        | async::seq(write_data)
        ;
}
// clang-format on
} // namespace button
