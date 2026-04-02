#include "meganz_account_generator/account_generator.hpp"

#include <string_view>
#include <type_traits>

int main()
{
    using meganz_account_generator::AccountGenerator;
    using meganz_account_generator::AccountGeneratorConfig;
    using meganz_account_generator::GeneratedAccount;

    static_assert(std::is_move_constructible_v<AccountGenerator>);
    static_assert(std::is_move_assignable_v<AccountGenerator>);
    static_assert(!std::is_copy_constructible_v<AccountGenerator>);
    static_assert(!std::is_copy_assignable_v<AccountGenerator>);

    const AccountGeneratorConfig config{
        .app_key = "public-api-app-key",
        .password = "public-api-password",
    };
    const GeneratedAccount account{
        .email = "person@example.com",
        .password = config.password,
        .display_name = config.display_name,
    };
    const meganz_account_generator::ConfirmationLinkParseError error("link parse failed");

    return config.display_name == "Automation Bot" &&
            account.email == "person@example.com" &&
            std::string_view(error.what()) == "link parse failed"
        ? 0
        : 1;
}
