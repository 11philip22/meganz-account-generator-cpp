#include "mega/request_waiter.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace
{

class FakeRequest final : public mega::MegaRequest
{
public:
    explicit FakeRequest(int type)
        : type_(type)
    {
    }

    [[nodiscard]] mega::MegaRequest* copy() override
    {
        return new FakeRequest(*this);
    }

    [[nodiscard]] int getType() const override
    {
        return type_;
    }

private:
    int type_{-1};
};

class FakeError final : public mega::MegaError
{
public:
    FakeError(int error_code, std::string error_string)
        : mega::MegaError(error_code)
        , error_code_(error_code)
        , error_string_(std::move(error_string))
    {
    }

    [[nodiscard]] mega::MegaError* copy() const override
    {
        return new FakeError(*this);
    }

    [[nodiscard]] int getErrorCode() const override
    {
        return error_code_;
    }

    [[nodiscard]] const char* getErrorString() const override
    {
        return error_string_.c_str();
    }

private:
    int error_code_{mega::MegaError::API_EINTERNAL};
    std::string error_string_;
};

void require(bool condition, std::string_view message)
{
    if(condition)
    {
        return;
    }

    std::cerr << "request_waiter_test failed: " << message << '\n';
    std::exit(1);
}

void test_timeout_keeps_listener_alive_until_finish()
{
    using namespace std::chrono_literals;

    std::weak_ptr<mega_integration::RequestWaiter> weak_waiter;
    {
        const auto waiter = mega_integration::RequestWaiter::create();
        weak_waiter = waiter;
        waiter->retain_until_finish();

        bool timed_out = false;
        try
        {
            static_cast<void>(waiter->wait(0ms));
        }
        catch(const mega_integration::RequestTimeoutError& error)
        {
            timed_out = true;
            require(error.timeout() == 0ms, "timeout duration should round-trip");
        }

        require(timed_out, "wait() should time out before the request finishes");
    }

    require(!weak_waiter.expired(), "listener should survive a timed-out wait");

    auto waiter = weak_waiter.lock();
    require(static_cast<bool>(waiter), "timed-out waiter should still be reachable");

    FakeRequest request(mega::MegaRequest::TYPE_LOGIN);
    FakeError error(mega::MegaError::API_OK, "ok");
    waiter->onRequestFinish(nullptr, &request, &error);
    waiter.reset();

    require(weak_waiter.expired(), "listener should release itself after finish");
}

void test_finish_copies_request_and_temporary_error()
{
    using namespace std::chrono_literals;

    const auto waiter = mega_integration::RequestWaiter::create();
    waiter->retain_until_finish();

    FakeError temporary_error(mega::MegaError::API_EAGAIN, "retry");
    FakeRequest request(mega::MegaRequest::TYPE_CREATE_ACCOUNT);
    FakeError final_error(mega::MegaError::API_OK, "ok");

    waiter->onRequestTemporaryError(nullptr, nullptr, &temporary_error);
    waiter->onRequestFinish(nullptr, &request, &final_error);

    const auto result = waiter->wait(10ms);
    require(result.ok(), "final result should report success");
    require(
        result.request_type() == mega::MegaRequest::TYPE_CREATE_ACCOUNT,
        "request type should be copied from the SDK request"
    );
    require(result.temporary_error.has_value(), "temporary error should be captured");
    require(
        result.temporary_error.value()->getErrorCode() == mega::MegaError::API_EAGAIN,
        "temporary error code should be copied"
    );
}

void test_execute_request_captures_error_result()
{
    using namespace std::chrono_literals;

    FakeRequest request(mega::MegaRequest::TYPE_CONFIRM_ACCOUNT);
    FakeError temporary_error(mega::MegaError::API_EAGAIN, "retry");
    FakeError final_error(mega::MegaError::API_EACCESS, "access denied");

    auto result = mega_integration::execute_request(10ms, [&](auto* waiter)
    {
        waiter->onRequestTemporaryError(nullptr, nullptr, &temporary_error);
        waiter->onRequestFinish(nullptr, &request, &final_error);
    });

    require(!result.ok(), "error result should report failure");
    require(
        result.request_type() == mega::MegaRequest::TYPE_CONFIRM_ACCOUNT,
        "error result should still copy the request type"
    );
    require(
        result.error_code == mega::MegaError::API_EACCESS,
        "error result should preserve the final error code"
    );
    require(
        result.error_string == "access denied",
        "error result should preserve the final error string"
    );

    const mega_integration::MegaRequestError error(std::move(result));
    require(error.error_code() == mega::MegaError::API_EACCESS, "wrapped error should preserve code");
    const std::string expected_code_fragment =
        "code " + std::to_string(mega::MegaError::API_EACCESS);
    require(
        std::string_view(error.what()).find(expected_code_fragment) != std::string_view::npos,
        "wrapped error message should include the numeric error code"
    );
    require(
        std::string_view(error.what()).find("last temporary error: retry") != std::string_view::npos,
        "wrapped error message should include the latest temporary error"
    );
}

void test_finish_without_error_object_reports_internal_error()
{
    using namespace std::chrono_literals;

    const auto waiter = mega_integration::RequestWaiter::create();
    waiter->retain_until_finish();

    FakeRequest request(mega::MegaRequest::TYPE_LOGIN);
    waiter->onRequestFinish(nullptr, &request, nullptr);

    const auto result = waiter->wait(10ms);
    require(!result.ok(), "missing MegaError should be treated as failure");
    require(
        result.error_code == mega::MegaError::API_EINTERNAL,
        "missing MegaError should map to API_EINTERNAL"
    );
    require(
        result.error_string == "request finished without MegaError",
        "missing MegaError should report an actionable message"
    );
}

} // namespace

int main()
{
    test_timeout_keeps_listener_alive_until_finish();
    test_finish_copies_request_and_temporary_error();
    test_execute_request_captures_error_result();
    test_finish_without_error_object_reports_internal_error();
    return 0;
}
