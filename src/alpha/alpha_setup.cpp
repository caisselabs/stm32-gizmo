//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "shared/concurrency.hpp"

#include <groov/groov.hpp>

#include <async/just.hpp>
#include <async/repeat.hpp>
#include <async/sequence.hpp>
#include <async/sync_wait.hpp>
#include <async/then.hpp>

#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

using namespace groov::literals;
namespace stm32 = caisselabs::stm32;

// clang-format off
void init_clock() {
    groov::write(stm32::rcc(
          "cr.HSION"_f = groov::enable,
          "cr.HSIKERON"_f = groov::enable
          ))
        | async::seq(groov::read(stm32::rcc / "cr.HSIRDY"_f))
        | async::repeat_until([](auto v) { return is_ready(v); })
        | async::sync_wait()
        ;

    groov::write(stm32::rcc("cfgr.SW"_f = stm32::rccx::sw::HSI16))
        | async::seq(groov::read(stm32::rcc / "cfgr.SW"_f))
        | async::repeat_until(
              [](auto v) { return v == stm32::rccx::sw::HSI16; })
        | async::sync_wait()
        ;

    groov::sync_write(stm32::rcc("apb2enr.SYSCFGEN"_f = groov::enable));
}

extern "C" {
// called by startup code prior to main
void SystemInit() {
    init_clock(); // get our clocks setup before doing other things
}
}


void i2c_reset() {
    // per docs, reset is
    //   - write PE = 0
    //   - check PE = 0
    //   - write PE = 1
    // This sequence ensures the correct APB clock cycles
    groov::write(stm32::i2c1("cr1.PE"_f = groov::disable))
        | async::seq(groov::read(stm32::i2c1 / "cr1.PE"_f))
        | async::seq(groov::write(stm32::i2c1("cr1.PE"_f = groov::enable)))
        | async::seq(groov::read(stm32::i2c1 / "cr1.PE"_f))
        | async::sync_wait();
}

// mapped currently to:
//        PA9  (CN3-1 / D1) - I2C1_SCL
//        PA10 (CN3-2 / D0) - I2C1_SDA
void setup_i2c() {

    // reset the i2c
    i2c_reset();
    
    // ensure it is disabled during configure
    groov::write(stm32::i2c1("cr1.PE"_f = groov::disable))
        | async::seq(groov::read(stm32::i2c1 / "cr1.PE"_f))
        | async::sync_wait();

    
    // enable clocks to i2c1
    groov::sync_write(
        stm32::rcc(
            "apb1enr1.I2C1EN"_f = groov::enable
        ));

    // ----------------------------------
    // 
    groov::write(
          stm32::rcc(
              "apb1rstr1.I2C1RST"_f = groov::set
          ))
        | async::seq(groov::write(
            stm32::rcc(
              "apb1rstr1.I2C1RST"_f = groov::clear
          )));

    // setup clock
    groov::sync_write(
        stm32::rcc(
            "ccipr.I2C1SEL"_f = stm32::rccx::i2cclk::HSI16
        ));

    // f(i2cclk) is 16MHz
    groov::sync_write(
        stm32::i2c1(
            "timingr.PRESC"_f = 0x1,
            "timingr.SCLL"_f = 0xc7,
            "timingr.SCLH"_f = 0xc3,
            "timingr.SDADEL"_f = 0x2,
            "timingr.SCLDEL"_f = 0x4
        ));

    // --------------------------------------------------------
    // route i2c1 to PB6 (scl) and PB7 (sda)
    //
    // The dev board shares some pins and since we are using
    // i2c on PB6 and PB7 we must:
    //
    // PA6 must be configured as input floating.
    // PA5 must be configured as input floating.
    groov::sync_write(
        stm32::rcc(
            "ahb2enr.GPIOAEN"_f = groov::enable,
            "ahb2enr.GPIOBEN"_f = groov::enable
        ));
    groov::sync_write(
        stm32::gpioa(
            "moder.5"_f = stm32::gpio::mode::input,
            "pupdr.5"_f = stm32::gpio::pupd::none,
            "moder.6"_f = stm32::gpio::mode::input,
            "pupdr.6"_f = stm32::gpio::pupd::none
        ));

    groov::sync_write(
        stm32::gpiob(
            "moder.6"_f = stm32::gpio::mode::alternate,
            "otyper.6"_f = stm32::gpio::outtype::open_drain,
            "ospeedr.6"_f = stm32::gpio::speed::high_speed,
            "pupdr.6"_f = stm32::gpio::pupd::pull_up,
            "afrl.6"_f = stm32::gpio::afsel::AF4,
            "moder.7"_f = stm32::gpio::mode::alternate,
            "otyper.7"_f = stm32::gpio::outtype::open_drain,
            "ospeedr.7"_f = stm32::gpio::speed::high_speed,
            "pupdr.7"_f = stm32::gpio::pupd::pull_up,
            "afrl.7"_f = stm32::gpio::afsel::AF4
        ));
    // --------------------------------------------------------

    // i2c_reset();
    groov::sync_write(stm32::i2c1("cr1.PE"_f = groov::enable));


    // [x] enable the i2c clock and gpio clock
    // [ ] configure the i2c pins for alternative functions
    //     - select alteernative function in moder reg
    //     - select open drain output
    //     - select high speed for the pins
    //     - select pull-up for both pins
    //     - configure the alternate function in afr registeer
    // [this is no longer cr1.swrst] reset the i2c
    // [ ] program the peripheral input clock in i2c_cr2 registeer to generate correct timings
    // [ ] configure the clock control registers
    // [ ] configure the rise time register
    // [ ] program the i2c_cr1 register to enable peripheral
    
    
}

void initialize_board() {

    groov::sync_write(
        stm32::rcc(
            "ahb2enr.GPIOAEN"_f = groov::enable,
            "ahb2enr.GPIOBEN"_f = groov::enable,
            "apb1enr1.I2C1EN"_f = groov::enable,
            "apb1enr1.TIM2EN"_f = groov::enable
        ));
    
    setup_i2c();

    // PB3 setup
    groov::sync_write(
        stm32::gpiob(
            "moder.3"_f = stm32::gpio::mode::output,
            "otyper.3"_f = stm32::gpio::outtype::push_pull,
            "ospeedr.3"_f = stm32::gpio::speed::low_speed,
            "pupdr.3"_f = stm32::gpio::pupd::none
        ));
}



void test_send(std::uint8_t bright) {

    auto wait_for_byte_write =
          groov::read(stm32::i2c1 / "isr.TXIS"_f)
        | async::repeat_until([](auto v) { return v == true; })
        ;
    
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
        | seq(wait_for_byte_write)
        | seq(groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = 0x19 // LED Brightness address
          )))
        | seq(wait_for_byte_write)
        | seq(groov::write(
            stm32::i2c1(
              "txdr.TXDATA"_f = bright
          )))
        | async::sync_wait();

}



