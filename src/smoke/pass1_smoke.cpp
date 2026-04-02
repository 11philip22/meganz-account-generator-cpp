#include <iostream>
#include <utility>

#include "mail/guerrillamail_client.hpp"
#include "mega/mega_api_client.hpp"

int main()
{
    const mail::ClientOptions mail_options{};
    const mail::MessageSummary message_summary{};
    const mail::EmailDetails email_details{};

    mega_integration::ClientOptions mega_options{
        .app_key = "9gETCbhB",
        .user_agent = "meganz-account-generator-cpp-pass2-smoke",
    };
    mega_integration::MegaApiClient mega_client(std::move(mega_options));
    mega_client.set_log_level(mega::MegaApi::LOG_LEVEL_ERROR);

    std::cout
        << "pass1_smoke linked the wrapper library, MEGA SDK, and guerrillamail-client-c "
        << "(mail wrapper types="
        << (sizeof(mail_options) + sizeof(message_summary) + sizeof(email_details))
        << ")\n";
    return 0;
}
