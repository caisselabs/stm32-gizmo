//
// Copyright (c) 2025 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------------------
//
// Timer interrupt hooked to a priority to a timer scheduler
//
//
// ----------------------------------------------------------------------------

// a concurrency policy is needed by the async library
// #include "button.hpp"  // spinning version
#include "button_int2.hpp"
#include "shared/concurrency.hpp"
#include "shared/stm32_interrupt.hpp"
#include "shared/stm32_timer.hpp"
#include "shared/timer_scheduler.hpp"

#include <async/continue_on.hpp>
#include <async/repeat.hpp>
#include <async/schedulers/priority_scheduler.hpp>
#include <async/schedulers/task_manager.hpp>
#include <async/schedulers/time_scheduler.hpp>
#include <async/sequence.hpp>
#include <async/start_detached.hpp>

#include <chrono>
#include <cstdint>

#include <caisselabs/stm32/stm32l432.hpp>

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// fixed priority scheduler
//
#include <async/schedulers/priority_scheduler.hpp>
#include <async/schedulers/task_manager.hpp>

namespace blinky_sched {
struct interrupt_scheduler {
    static auto schedule(async::priority_t p) -> void {
        shared::trigger_interrupt(56);
    }
};

using task_manager_t = async::priority_task_manager<interrupt_scheduler, 1>;
} // namespace blinky_sched

template <>
inline auto async::injected_task_manager<> = blinky_sched::task_manager_t{};

extern "C" {
// taking this interrupt over for our scheduler
// DMA2_CH1 is interrupt 56 for our target
inline void DMA2_CH1_Handler(void) { async::task_mgr::service_tasks<0>(); }
}
// ----------------------------------------------------------------------------

// method to initialize basic board functionality
void initialize_board();

using namespace std::chrono_literals;
using namespace groov::literals;

// clang-format off
int main() {

  initialize_board();
  setup_interrupts();
  initialize_timer();


  auto delay = [](auto v) {
      return
          async::continue_on(async::time_scheduler{v});
  };

  auto led_on  = groov::write(stm32::gpiob("odr.3"_f=true));
  auto led_off = groov::write(stm32::gpiob("odr.3"_f=false));
  auto on_cycle  = led_on  | delay(300ms);
  auto off_cycle = led_off | delay(1s);

  
  async::sender auto blinky =
        on_cycle
      | async::seq(off_cycle)
      ;

  auto s = blinky | async::repeat() | async::start_detached();


  std::uint8_t bright = 0x00;
  while(true) {
      // spin little heater, spin
        button::set_brightness(++bright)
      | async::sync_wait();
  }
}

