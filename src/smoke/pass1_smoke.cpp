#include <iostream>
#include <utility>

#include "core/account_generator.hpp"
#include "mail/guerrillamail_client.hpp"
#include "mega/mega_api_client.hpp"

int main()
{
    const core::AccountGeneratorConfig generator_config{
        .app_key = "9gETCbhB",
        .password = "pass3-smoke-password",
        .display_name = "Automation Bot",
    };
    const mail::ClientOptions mail_options{};
    const mail::MessageSummary message_summary{};
    const mail::EmailDetails email_details{};
    const core::GeneratedAccount generated_account{};

    mega_integration::ClientOptions mega_options{
        .app_key = "9gETCbhB",
        .user_agent = "meganz-account-generator-cpp-pass2-smoke",
    };
    mega_integration::MegaApiClient mega_client(std::move(mega_options));
    mega_client.set_log_level(mega::MegaApi::LOG_LEVEL_ERROR);

    std::cout
        << "pass1_smoke linked the core layer, wrapper library, MEGA SDK, and guerrillamail-client-c "
        << "(types="
        << (
            sizeof(generator_config) +
            sizeof(generated_account) +
            sizeof(mail_options) +
            sizeof(message_summary) +
            sizeof(email_details)
        )
        << ")\n";
    return 0;
}
