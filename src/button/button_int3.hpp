#pragma once
#include "shared/concurrency.hpp"

#include <groov/groov.hpp>

#include <async/continue_on.hpp>
#include <async/just.hpp>
#include <async/let_value.hpp>
#include <async/repeat.hpp>
#include <async/schedulers/trigger_scheduler.hpp>
#include <async/sequence.hpp>
#include <async/sync_wait.hpp>
#include <async/then.hpp>
#include <async/when_all.hpp>
#include <async/when_any.hpp>

#include <stdx/bit.hpp>

#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

namespace button {
using namespace groov::literals;
namespace stm32 = caisselabs::stm32;

// clang-format off
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

auto wait_for_error() {
    using write_spec_t = decltype(groov::sync_read(stm32::i2c1 / "isr"_r));

    return
        async::trigger_scheduler<"i2c1_err", write_spec_t>{}.schedule()
        | async::let_value([](auto r) { return async::just_error(r); })
        ;
}

auto xfer_done_or_error =
    async::when_any(
       wait_for_flag<"TXIS">(),
       wait_for_error()
    );

auto xfer_stop_or_error =
    async::when_any(
       wait_for_flag<"STOPF">(),
       wait_for_error()
    );

auto write_byte = [](std::uint8_t b) {
    return
        async::when_all(
           xfer_done_or_error,
           groov::write(stm32::i2c1("txdr.TXDATA"_f = b))
       );
};

auto write_last_byte = [](std::uint8_t b) {
    return
        async::when_all(
           xfer_stop_or_error,
           groov::write(stm32::i2c1("txdr.TXDATA"_f = b))
          )
        | async::seq(
             groov::write(stm32::i2c1("icr.STOPCF"_f = groov::set))
          );
};

template <typename Data>
auto send_command(std::uint8_t addr, Data data) -> async::sender auto {

    constexpr std::uint8_t bytes_to_write = sizeof(data) + 1;

    auto setup_controller =
        async::when_all(
          xfer_done_or_error,
          groov::write(
            stm32::i2c1(
              "cr2.ADD10"_f = groov::disable,
              "cr2.SADD7"_f = 0x6f,
              "cr2.RD_WRN"_f = stm32::i2c::rd_wrn::WRITE_XFER,
              "cr2.NBYTES"_f = bytes_to_write,
              "cr2.RELOAD"_f = groov::disable,
              "cr2.AUTOEND"_f = groov::enable,
              "cr2.START"_f = true
            ))
        );

    auto control_addr =
        setup_controller | async::seq(write_byte(addr));

    auto values = stdx::bit_unpack<std::uint8_t>(data);
    auto write_data =
        std::apply([](auto ... v, auto last) {
            return
                (async::seq(write_byte(v))
                 | ...
                 |async::seq(write_last_byte(last) ));
        }, values);

    return control_addr | write_data;
}
// clang-format on
} // namespace button
