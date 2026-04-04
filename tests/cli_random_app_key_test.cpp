#include "cli/random_app_key.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace
{

void require(bool condition, std::string_view message)
{
    if(condition)
    {
        return;
    }

    std::cerr << "cli_random_app_key_test failed: " << message << '\n';
    std::exit(1);
}

void test_generate_random_app_key_shape()
{
    const auto app_key = cli::generate_random_app_key();

    require(app_key.size() == 8, "generated app key should have the default length");
    require(
        std::all_of(app_key.begin(), app_key.end(), [](char value)
        {
            return (value >= 'a' && value <= 'z') || (value >= '0' && value <= '9');
        }),
        "generated app key should contain only lowercase letters and digits"
    );
}

} // namespace

int main()
{
    test_generate_random_app_key_shape();
    return 0;
}
