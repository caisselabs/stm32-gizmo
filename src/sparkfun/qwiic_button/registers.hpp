//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <groov/config.hpp>
#include <groov/identity.hpp>

#include <stdx/ct_string.hpp>

#include <cstdint>

namespace sparkfun::qwiic_button::reg {

namespace access {
using rw = groov::w::replace;
using ro = groov::read_only<groov::w::ignore>;
} // namespace access

// clang-format off
using id =
    groov::reg<
        "id", std::uint8_t,
        0x00, access::ro
    >;

using firmware_lsb =
    groov::reg<
        "firmware_lsb", std::uint8_t,
        0x01, access::rw
    >;

using firmware_msb =
    groov::reg<
        "firmware_msb", std::uint8_t,
        0x02, access::rw
    >;

using button_status =
    groov::reg<
        "button_status", std::uint8_t,
        0x03, access::rw,
        groov::field<"eventAvailable", bool, 0, 0>,
        groov::field<"hasBeenClicked", bool, 1, 1>,
        groov::field<"isPressed", bool, 2, 2>
    >;

using int_cfg =
    groov::reg<
        "int_cfg", std::uint8_t,
        0x04, access::rw,
        groov::field<"click_en", bool, 0, 0>,
        groov::field<"press_en", bool, 1, 1>,
        groov::field<"reserved", std::uint8_t, 7, 2, access::ro>
    >;

using debounce_time =
    groov::reg<
        "debounce_time", std::uint16_t,
        0x05, access::rw
    >;

using pressed_queue_status =
    groov::reg<
        "pressed_queue_status", std::uint8_t,
        0x07, access::rw,
        groov::field<"full", bool, 0, 0, access::ro>,
        groov::field<"empty", bool, 1, 1, access::ro>,
        groov::field<"popRequest", bool, 2, 2>,
        groov::field<"reserved", std::uint8_t, 7, 3, access::ro>
    >;

using pressed_queue_front =
    groov::reg<
        "pressed_queue_front", std::uint64_t,
        0x08, access::ro
    >;

using pressed_queue_back =
    groov::reg<
        "pressed_queue_back", std::uint64_t,
        0x0c, access::ro
    >;

using clicked_queue_status =
    groov::reg<
        "clicked_queue_status", std::uint8_t,
        0x10, access::rw,
        groov::field<"full", bool, 0, 0, access::ro>,
        groov::field<"empty", bool, 1, 1, access::ro>,
        groov::field<"popRequest", bool, 2, 2>,
        groov::field<"reserved", std::uint8_t, 7, 3, access::ro>
    >;

using clicked_queue_front =
    groov::reg<
        "clicked_queue_front", std::uint64_t,
        0x11, access::ro
    >;

using clicked_queue_back =
    groov::reg<
        "clicked_queue_back", std::uint64_t,
        0x15, access::ro
    >;

using led_brightness =
    groov::reg<
        "led_brightness", std::uint8_t,
        0x19, access::rw,
        groov::field<"v", std::uint8_t, 7, 0>
    >;

using led_pulse_granularity =
    groov::reg<
        "led_pulse_granularity", std::uint8_t,
        0x1a, access::rw
    >;

using led_pulse_on_time =
    groov::reg<
        "led_pulse_on_time", std::uint16_t,
        0x1b, access::rw
    >;

using led_pulse_off_time =
    groov::reg<
        "led_pulse_off_time", std::uint16_t,
        0x1d, access::rw
    >;

using i2c_address =
    groov::reg<
        "i2c_address", std::uint8_t,
        0x1f, access::rw
    >;

template <stdx::ct_string Name, typename Bus>  //std::uint8_t I2CAddr>
using qwiic_button_t =
    groov::group<
        Name, Bus,
        // id,
        // firmware_lsb,
        // firmware_msb,
        // button_status,
        // debounce_time,
        // int_cfg,
        // pressed_queue_status,
        // pressed_queue_front,
        // pressed_queue_back,
        // clicked_queue_status,
        // clicked_queue_front,
        // clicked_queue_back,
        led_brightness
        // led_pulse_granularity,
        // led_pulse_on_time,
        // led_pulse_off_time,
        //i2c_address
    >;

// clang-format on
} // namespace sparkfun::qwiic_button::reg
