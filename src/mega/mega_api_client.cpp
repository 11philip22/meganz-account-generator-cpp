#include "mega/mega_api_client.hpp"

#include <stdexcept>
#include <utility>

namespace mega_integration
{

MegaApiClient::MegaApiClient(std::string app_key, std::chrono::milliseconds request_timeout)
    : app_key_(std::move(app_key))
    , request_timeout_(request_timeout)
{
    if(app_key_.empty())
    {
        throw std::invalid_argument("MegaApiClient requires a non-empty app key");
    }

    api_ = std::make_unique<mega::MegaApi>(
        app_key_.c_str(),
        nullptr,
        nullptr,
        1,
        mega::MegaApi::CLIENT_TYPE_DEFAULT
    );
}

RequestResult MegaApiClient::set_proxy(std::optional<std::string_view> proxy_url)
{
    mega::MegaProxy proxy = make_proxy(proxy_url);
    return ensure_success(execute_request(request_timeout_, [this, &proxy](auto* waiter)
    {
        api_->setProxySettings(&proxy, waiter);
    }));
}

RequestResult MegaApiClient::create_account(
    std::string_view email,
    std::string_view password,
    std::string_view first_name,
    std::string_view last_name
)
{
    const auto owned_email = std::string(email);
    const auto owned_password = std::string(password);
    const auto owned_first_name = std::string(first_name);
    const auto owned_last_name = std::string(last_name);

    return ensure_success(execute_request(
        request_timeout_,
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
    const auto owned_sid = std::string(sid);
    return ensure_success(execute_request(request_timeout_, [this, &owned_sid](auto* waiter)
    {
        api_->resumeCreateAccount(owned_sid.c_str(), waiter);
    }));
}

RequestResult MegaApiClient::confirm_account(std::string_view link)
{
    const auto owned_link = std::string(link);
    return ensure_success(execute_request(request_timeout_, [this, &owned_link](auto* waiter)
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

    throw MegaRequestError(result);
}

mega::MegaProxy MegaApiClient::make_proxy(
    const std::optional<std::string_view>& proxy_url
) const
{
    mega::MegaProxy proxy;

    if(proxy_url && !proxy_url->empty())
    {
        const auto owned_proxy = std::string(*proxy_url);
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
