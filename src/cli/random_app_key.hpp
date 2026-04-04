#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CLI_RANDOM_APP_KEY_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CLI_RANDOM_APP_KEY_HPP

#include <cstddef>
#include <string>

namespace cli
{

[[nodiscard]] std::string generate_random_app_key(std::size_t length = 8);

} // namespace cli

#endif
