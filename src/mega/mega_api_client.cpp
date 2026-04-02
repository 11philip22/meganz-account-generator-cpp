#include "mega/mega_api_client.hpp"

#include <stdexcept>
#include <utility>

namespace
{

[[nodiscard]] const char* to_nullable_c_str(const std::optional<std::string>& value)
{
    if(!value)
    {
        return nullptr;
    }

    return value->c_str();
}

[[nodiscard]] std::string to_owned_string(std::string_view value)
{
    return std::string(value);
}

} // namespace

namespace mega_integration
{

MegaApiClient::MegaApiClient(ClientOptions options)
    : options_(std::move(options))
{
    if(options_.app_key.empty())
    {
        throw std::invalid_argument("MegaApiClient requires a non-empty app key");
    }

    api_ = std::make_unique<mega::MegaApi>(
        options_.app_key.c_str(),
        to_nullable_c_str(options_.base_path),
        to_nullable_c_str(options_.user_agent),
        options_.worker_thread_count,
        options_.client_type
    );
}

void MegaApiClient::set_log_level(int level)
{
    api_->setLogLevel(level);
}

RequestResult MegaApiClient::set_proxy(std::optional<std::string_view> proxy_url)
{
    mega::MegaProxy proxy = make_proxy(proxy_url);
    return ensure_success(execute_request(options_.request_timeout, [this, &proxy](auto* waiter)
    {
        api_->setProxySettings(&proxy, waiter);
    }));
}

RequestResult MegaApiClient::login(std::string_view email, std::string_view password)
{
    const auto owned_email = to_owned_string(email);
    const auto owned_password = to_owned_string(password);

    return ensure_success(execute_request(options_.request_timeout, [this, &owned_email, &owned_password](auto* waiter)
    {
        api_->login(owned_email.c_str(), owned_password.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::create_account(
    std::string_view email,
    std::string_view password,
    std::string_view first_name,
    std::string_view last_name
)
{
    const auto owned_email = to_owned_string(email);
    const auto owned_password = to_owned_string(password);
    const auto owned_first_name = to_owned_string(first_name);
    const auto owned_last_name = to_owned_string(last_name);

    return ensure_success(execute_request(
        options_.request_timeout,
        [this, &owned_email, &owned_password, &owned_first_name, &owned_last_name](auto* waiter)
        {
            api_->createAccount(
                owned_email.c_str(),
                owned_password.c_str(),
                owned_first_name.c_str(),
                owned_last_name.c_str(),
                waiter
            );
        }
    ));
}

RequestResult MegaApiClient::resume_create_account(std::string_view sid)
{
    const auto owned_sid = to_owned_string(sid);
    return ensure_success(execute_request(options_.request_timeout, [this, &owned_sid](auto* waiter)
    {
        api_->resumeCreateAccount(owned_sid.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::resend_signup_link(
    std::string_view email,
    std::string_view name
)
{
    const auto owned_email = to_owned_string(email);
    const auto owned_name = to_owned_string(name);

    return ensure_success(execute_request(options_.request_timeout, [this, &owned_email, &owned_name](auto* waiter)
    {
        api_->resendSignupLink(owned_email.c_str(), owned_name.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::query_signup_link(std::string_view link)
{
    const auto owned_link = to_owned_string(link);
    return ensure_success(execute_request(options_.request_timeout, [this, &owned_link](auto* waiter)
    {
        api_->querySignupLink(owned_link.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::confirm_account(std::string_view link)
{
    const auto owned_link = to_owned_string(link);
    return ensure_success(execute_request(options_.request_timeout, [this, &owned_link](auto* waiter)
    {
        api_->confirmAccount(owned_link.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::ensure_success(RequestResult result) const
{
    if(result.ok())
    {
        return result;
    }

    throw MegaRequestError(std::move(result));
}

mega::MegaProxy MegaApiClient::make_proxy(
    const std::optional<std::string_view>& proxy_url
) const
{
    mega::MegaProxy proxy;

    if(proxy_url && !proxy_url->empty())
    {
        const auto owned_proxy = to_owned_string(*proxy_url);
        proxy.setProxyType(mega::MegaProxy::PROXY_CUSTOM);
        proxy.setProxyURL(owned_proxy.c_str());
    }
    else
    {
        proxy.setProxyType(mega::MegaProxy::PROXY_NONE);
    }

    return proxy;
}

} // namespace mega_integration
