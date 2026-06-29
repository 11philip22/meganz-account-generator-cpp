#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_MEGA_API_CLIENT_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_MEGA_API_CLIENT_HPP

#include "mega/request_waiter.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <string_view>

#include <megaapi.h>

namespace mega_integration
{

class MegaApiClient
{
public:
    MegaApiClient(std::string app_key, std::chrono::milliseconds request_timeout);
    ~MegaApiClient() = default;

    MegaApiClient(const MegaApiClient&) = delete;
    MegaApiClient& operator=(const MegaApiClient&) = delete;
    MegaApiClient(MegaApiClient&&) noexcept = default;
    MegaApiClient& operator=(MegaApiClient&&) noexcept = default;

    [[nodiscard]] RequestResult set_proxy(std::string_view proxy_url);
    [[nodiscard]] RequestResult create_account(
        std::string_view email,
        std::string_view password,
        std::string_view first_name,
        std::string_view last_name
    );
    [[nodiscard]] RequestResult resume_create_account(std::string_view sid);
    [[nodiscard]] RequestResult confirm_account(std::string_view link);

private:
    std::string app_key_;
    std::chrono::milliseconds request_timeout_;
    std::unique_ptr<mega::MegaApi> api_;

    [[nodiscard]] RequestResult ensure_success(RequestResult result) const;
};

} // namespace mega_integration

#endif
