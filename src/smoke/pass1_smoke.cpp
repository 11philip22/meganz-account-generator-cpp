#include <cstdlib>
#include <iostream>
#include <memory>

#include <guerrillamail_client.h>
#include <megaapi.h>

namespace
{
constexpr auto kSmokeAppKey = "9gETCbhB";
constexpr auto kSmokeUserAgent = "meganz-account-generator-cpp-pass1-smoke";
}

int main()
{
    gm_last_error_clear();
    const char* last_error = gm_last_error_message();

    auto api = std::make_unique<mega::MegaApi>(
        kSmokeAppKey,
        static_cast<const char*>(nullptr),
        kSmokeUserAgent,
        1u,
        mega::MegaApi::CLIENT_TYPE_DEFAULT
    );
    api->setLogLevel(mega::MegaApi::LOG_LEVEL_ERROR);

    std::cout << "pass1_smoke linked the MEGA SDK and guerrillamail-client-c";
    if(last_error && last_error[0] != '\0')
    {
        std::cout << " (GuerrillaMail last error buffer is non-empty)";
    }
    std::cout << '\n';

    return EXIT_SUCCESS;
}
