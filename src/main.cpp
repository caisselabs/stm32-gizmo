//----------------------------------------------------------------
// User includes this
#include <groov/test.hpp>
//----------------------------------------------------------------
//----------------------------------------------------------------
struct my_bus;
namespace groov::test {

template <> struct storage_type<"test_group"> {
    using type = make_store_type<std::uint32_t, std::uint32_t>;
};

using test_bus_list = make_test_bus_list<default_test_bus<"test_group">,
                                         test_bus<"my_group", my_bus>>;
} // namespace groov::test
//----------------------------------------------------------------

#include <groov/groov.hpp>
#include <groov/read.hpp>
#include <groov/value_path.hpp>
#include <groov/write.hpp>

#include <async/concepts.hpp>
#include <async/just.hpp>
#include <async/just_result_of.hpp>
#include <async/sync_wait.hpp>
#include <async/then.hpp>

#include <cstdint>
#include <iostream>
#include <map>

#include <sparkfun/qwiic_button/registers.hpp>

using address_t = std::uint32_t;
using value_t = std::uint32_t;

using testing_store_t = std::map<address_t, value_t>;
testing_store_t testing_store;

using button1_t =
    sparkfun::qwiic_button::reg::qwiic_button_t<"button1", my_bus_t>;

constexpr auto button1 = button1_t{};

using test_reg_t = groov::reg<"reg", std::uint32_t, 99, groov::w::replace,
                              groov::field<"t0", std::uint8_t, 7, 1>,
                              groov::field<"t1", std::uint32_t, 9, 0>>;
using test_group_t = groov::group<"test_group", my_bus_t, test_reg_t>;

auto test_g = test_group_t{};

using namespace groov::literals;

int main() {
    // auto path = "int_cfg.click_en"_f;
    // groov::sync_write( button1(path = true)); //button1("int_cfg.click_en"_f
    // = true));
    //    auto path = "i2c_address"_r;

    // groov::sync_write( button1("i2c_address"_r = 0xa5));
    // auto v = groov::sync_read( button1 / "i2c_address"_r );

    /*
    // annoying that it is [[nodiscard]]
    groov::sync_write(
        button1(
            "debounce_time"_r = 0xa55a,
            "i2c_address"_r = 0x11
        ));

    //auto v = groov::sync_read( button1 / "debounce_time"_r );
    auto v = groov::sync_read(
                 button1("debounce_time"_r, "i2c_address"_r));


    std::cout << "debounce_time: " << v["debounce_time"_r]
              << " i2c_address: " << (int)v["i2c_address"_r] << "\n";
    */
}
