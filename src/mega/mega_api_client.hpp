#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_MEGA_API_CLIENT_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_MEGA_API_CLIENT_HPP

#include "mega/request_waiter.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <megaapi.h>

namespace mega_integration
{

struct ClientOptions
{
    std::string app_key;
    std::chrono::milliseconds request_timeout{std::chrono::seconds{30}};
};

class MegaApiClient
{
public:
    explicit MegaApiClient(ClientOptions options);
    ~MegaApiClient() = default;

    MegaApiClient(const MegaApiClient&) = delete;
    MegaApiClient& operator=(const MegaApiClient&) = delete;
    MegaApiClient(MegaApiClient&&) noexcept = default;
    MegaApiClient& operator=(MegaApiClient&&) noexcept = default;

    [[nodiscard]] RequestResult set_proxy(std::optional<std::string_view> proxy_url);
    [[nodiscard]] RequestResult create_account(
        std::string_view email,
        std::string_view password,
        std::string_view first_name,
        std::string_view last_name
    );
    [[nodiscard]] RequestResult resume_create_account(std::string_view sid);
    [[nodiscard]] RequestResult confirm_account(std::string_view link);

private:
    ClientOptions options_;
    std::unique_ptr<mega::MegaApi> api_;

    [[nodiscard]] RequestResult ensure_success(RequestResult result) const;
    [[nodiscard]] mega::MegaProxy make_proxy(
        const std::optional<std::string_view>& proxy_url
    ) const;
};

} // namespace mega_integration

#endif
