#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_REQUEST_WAITER_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MEGA_REQUEST_WAITER_HPP

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>

#include <megaapi.h>

namespace mega_integration
{

struct RequestResult
{
    std::unique_ptr<mega::MegaRequest> request;
    std::optional<std::unique_ptr<mega::MegaError>> temporary_error;
    int error_code{mega::MegaError::API_EINTERNAL};
    std::string error_string;

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] int request_type() const noexcept;
};

class RequestTimeoutError : public std::runtime_error
{
public:
    explicit RequestTimeoutError(std::chrono::milliseconds timeout);

    [[nodiscard]] std::chrono::milliseconds timeout() const noexcept;

private:
    std::chrono::milliseconds timeout_;
};

class MegaRequestError : public std::runtime_error
{
public:
    explicit MegaRequestError(RequestResult result);

    [[nodiscard]] int error_code() const noexcept;
    [[nodiscard]] const RequestResult& result() const noexcept;

private:
    RequestResult result_;
};

class RequestWaiter final : public mega::MegaRequestListener
                           , public std::enable_shared_from_this<RequestWaiter>
{
private:
    struct ConstructionToken
    {
    };

public:
    explicit RequestWaiter(ConstructionToken);

    [[nodiscard]] static std::shared_ptr<RequestWaiter> create();

    ~RequestWaiter() override = default;

    RequestWaiter(const RequestWaiter&) = delete;
    RequestWaiter& operator=(const RequestWaiter&) = delete;

    void onRequestFinish(
        mega::MegaApi* api,
        mega::MegaRequest* request,
        mega::MegaError* error
    ) override;

    void onRequestTemporaryError(
        mega::MegaApi* api,
        mega::MegaRequest* request,
        mega::MegaError* error
    ) override;

    void retain_until_finish();
    [[nodiscard]] RequestResult wait(std::chrono::milliseconds timeout);

private:
    void release_keepalive();

    mutable std::mutex mutex_;
    std::optional<std::unique_ptr<mega::MegaError>> temporary_error_;
    std::promise<RequestResult> promise_;
    std::future<RequestResult> future_;
    // When a wait times out, the SDK may still finish the request later on a callback thread.
    // This shared_ptr keeps the listener alive until onRequestFinish releases it.
    std::shared_ptr<RequestWaiter> self_keepalive_;
    bool finished_{false};
};

template <typename StartRequest>
[[nodiscard]] RequestResult execute_request(
    std::chrono::milliseconds timeout,
    StartRequest&& start_request
)
{
    auto waiter = RequestWaiter::create();
    std::forward<StartRequest>(start_request)(waiter.get());
    waiter->retain_until_finish();
    return waiter->wait(timeout);
}

} // namespace mega_integration

#endif
