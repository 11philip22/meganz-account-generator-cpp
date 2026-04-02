#include "mega/request_waiter.hpp"

#include <sstream>
#include <utility>

namespace
{

[[nodiscard]] std::string describe_error_code(int error_code)
{
    const char* description = mega::MegaError::getErrorString(error_code);
    if(description == nullptr || description[0] == '\0')
    {
        return "unknown MEGA error";
    }

    return description;
}

[[nodiscard]] std::string copy_error_string(const mega::MegaError& error)
{
    const char* value = error.getErrorString();
    if(value == nullptr || value[0] == '\0')
    {
        return describe_error_code(error.getErrorCode());
    }

    return value;
}

} // namespace

namespace mega_integration
{

bool RequestResult::ok() const noexcept
{
    return error_code == mega::MegaError::API_OK;
}

int RequestResult::request_type() const noexcept
{
    if(request == nullptr)
    {
        return -1;
    }

    return request->getType();
}

RequestTimeoutError::RequestTimeoutError(std::chrono::milliseconds timeout)
    : std::runtime_error(
        [timeout]
        {
            std::ostringstream stream;
            stream << "MEGA request timed out after " << timeout.count() << "ms";
            return stream.str();
        }()
    )
    , timeout_(timeout)
{
}

std::chrono::milliseconds RequestTimeoutError::timeout() const noexcept
{
    return timeout_;
}

MegaRequestError::MegaRequestError(RequestResult result)
    : std::runtime_error(result.error_string)
    , result_(std::move(result))
{
}

int MegaRequestError::error_code() const noexcept
{
    return result_.error_code;
}

const RequestResult& MegaRequestError::result() const noexcept
{
    return result_;
}

std::shared_ptr<RequestWaiter> RequestWaiter::create()
{
    return std::make_shared<RequestWaiter>(ConstructionToken{});
}

RequestWaiter::RequestWaiter(ConstructionToken)
    : future_(promise_.get_future())
{
}

void RequestWaiter::onRequestFinish(
    mega::MegaApi*,
    mega::MegaRequest* request,
    mega::MegaError* error
)
{
    const auto keepalive = shared_from_this();

    RequestResult result;
    if(request != nullptr)
    {
        result.request.reset(request->copy());
    }

    if(error != nullptr)
    {
        result.error_code = error->getErrorCode();
        result.error_string = copy_error_string(*error);
    }
    else
    {
        result.error_code = mega::MegaError::API_EINTERNAL;
        result.error_string = "request finished without MegaError";
    }

    {
        std::lock_guard lock(mutex_);
        if(finished_)
        {
            return;
        }

        if(temporary_error_)
        {
            result.temporary_error = std::make_optional(
                std::unique_ptr<mega::MegaError>(temporary_error_.value()->copy())
            );
        }

        finished_ = true;
    }

    promise_.set_value(std::move(result));
    release_keepalive();
}

void RequestWaiter::onRequestTemporaryError(
    mega::MegaApi*,
    mega::MegaRequest*,
    mega::MegaError* error
)
{
    if(error == nullptr)
    {
        return;
    }

    std::lock_guard lock(mutex_);
    if(finished_)
    {
        return;
    }

    temporary_error_ = std::make_optional(std::unique_ptr<mega::MegaError>(error->copy()));
}

void RequestWaiter::retain_until_finish()
{
    std::lock_guard lock(mutex_);
    if(finished_)
    {
        return;
    }

    self_keepalive_ = shared_from_this();
}

RequestResult RequestWaiter::wait(std::chrono::milliseconds timeout)
{
    if(future_.wait_for(timeout) != std::future_status::ready)
    {
        throw RequestTimeoutError(timeout);
    }

    return future_.get();
}

void RequestWaiter::release_keepalive()
{
    std::lock_guard lock(mutex_);
    self_keepalive_.reset();
}

} // namespace mega_integration
