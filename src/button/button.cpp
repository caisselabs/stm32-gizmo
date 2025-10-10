//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------------------

// a concurrency policy is needed by the async library
// #include "button.hpp"  // spinning version
#include "button_bus.hpp"
#include "shared/concurrency.hpp"
#include "shared/stm32_interrupt.hpp"
#include "shared/stm32_timer.hpp"
#include "shared/timer_scheduler.hpp"
#include "sparkfun/qwiic_button/registers.hpp"

#include <async/let_value.hpp>
#include <async/repeat.hpp>
#include <async/schedulers/time_scheduler.hpp>
#include <async/sequence.hpp>
#include <async/start_detached.hpp>
#include <async/timeout_after.hpp>

#include <chrono>
#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

// method to initialize basic board functionality
void initialize_board();

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// fixed priority scheduler is configured with one priority level and
// is backed by one repurposed interrupt.
// DMA2_CH1 is interrupt 56 and will be used for our scheduler. This means
// we will trigger the interrupt via "software" with the schedule(priority)
// call.
//
#include <async/schedulers/priority_scheduler.hpp>
#include <async/schedulers/task_manager.hpp>

struct interrupt_scheduler {
    static auto schedule(async::priority_t p) -> void {
        shared::trigger_interrupt(56);
    }
};

using task_manager_t = async::priority_task_manager<interrupt_scheduler, 1>;
template <> inline auto async::injected_task_manager<> = task_manager_t{};

extern "C" {
// the repurposed ISR will service all tasks at priority 0
inline void DMA2_CH1_Handler(void) { async::task_mgr::service_tasks<0>(); }
}
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

using namespace std::chrono_literals;
using namespace groov::literals;

auto on_time = 500ms;
auto off_time = 500ms;

auto delay(auto v) { return async::time_scheduler{v}.schedule(); }

// ------------------------------------------------------------------
// Setup our button to use our new i2c bus implementation.
// ------------------------------------------------------------------
using my_button_t =                              //
    sparkfun::qwiic_button::reg::qwiic_button_t< //
        "my_button", button::bus<0x6f>           //
        >;
my_button_t my_button;

struct timeout_error {};

// clang-format off
int main() {

  initialize_board();
  setup_interrupts();
  initialize_timer();


  // ---------------------------------------------------------------
  // Blinky LED on development board
  // ---------------------------------------------------------------
  auto led_on  = groov::write(stm32::gpiob("odr.3"_f=true));
  auto led_off = groov::write(stm32::gpiob("odr.3"_f=false));
  auto on_cycle  = led_on  | async::let_value([](){ return delay(on_time); });
  auto off_cycle = led_off | async::let_value([](){ return delay(off_time); });
  
  async::sender auto blinky =
        on_cycle
      | async::seq(off_cycle)
      ;

  // start the on-board LED blinking as "task"
  auto s = blinky | async::repeat() | async::start_detached();
  // ---------------------------------------------------------------
  // ---------------------------------------------------------------

  std::uint8_t bright = 0x00;
  while(true) {
      groov::write(
          my_button("led_brightness"_r = ++bright)
        )
      | async::timeout_after(10ms, timeout_error{})
      | async::upon_error([](auto e){
          // rapid blinking to show we have an error
          on_time = 100ms;
          off_time = 100ms;
        })
      | async::sync_wait();

  }
}

