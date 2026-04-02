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
    std::optional<std::string> base_path;
    std::optional<std::string> user_agent;
    std::chrono::milliseconds request_timeout{std::chrono::seconds{30}};
    unsigned worker_thread_count{1};
    int client_type{mega::MegaApi::CLIENT_TYPE_DEFAULT};
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

    void set_log_level(int level);

    [[nodiscard]] RequestResult set_proxy(std::optional<std::string_view> proxy_url);
    [[nodiscard]] RequestResult login(std::string_view email, std::string_view password);
    [[nodiscard]] RequestResult create_account(
        std::string_view email,
        std::string_view password,
        std::string_view first_name,
        std::string_view last_name
    );
    [[nodiscard]] RequestResult resume_create_account(std::string_view sid);
    [[nodiscard]] RequestResult resend_signup_link(
        std::string_view email,
        std::string_view name
    );
    [[nodiscard]] RequestResult query_signup_link(std::string_view link);
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
