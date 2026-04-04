#include "cli/random_app_key.hpp"

#include <random>
#include <stdexcept>
#include <string_view>

namespace cli
{

std::string generate_random_app_key(std::size_t length)
{
    if(length == 0)
    {
        throw std::invalid_argument("app key length must be greater than zero");
    }

    static constexpr std::string_view kAlphabet = "abcdefghijklmnopqrstuvwxyz0123456789";

    std::random_device random_device;
    std::mt19937_64 engine(random_device());
    std::uniform_int_distribution<std::size_t> distribution(0, kAlphabet.size() - 1);

    std::string app_key;
    app_key.reserve(length);

    for(std::size_t index = 0; index < length; ++index)
    {
        app_key.push_back(kAlphabet[distribution(engine)]);
    }

    return app_key;
}

} // namespace cli
