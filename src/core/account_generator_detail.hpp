#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CORE_ACCOUNT_GENERATOR_DETAIL_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CORE_ACCOUNT_GENERATOR_DETAIL_HPP

#include "mail/guerrillamail_client.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace core::detail
{

struct NameParts
{
    std::string first_name;
    std::string last_name;
};

[[nodiscard]] std::string trim_copy(std::string_view value);
[[nodiscard]] std::optional<std::string> normalize_optional_string(
    std::optional<std::string> value
);
[[nodiscard]] NameParts split_display_name(std::string_view display_name);
[[nodiscard]] std::string generate_random_alias(std::size_t length = 12);
[[nodiscard]] bool is_probable_mega_message(const mail::MessageSummary& message);
[[nodiscard]] std::optional<std::string> extract_confirmation_link(std::string_view body);
void validate_confirmed_email_value(
    std::string_view confirmed_email,
    std::string_view expected_email
);

} // namespace core::detail

#endif
