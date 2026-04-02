#include "meganz_account_generator/account_generator.hpp"

#include "core/account_generator.hpp"

#include <utility>

namespace
{

[[nodiscard]] meganz_account_generator::MailFailureStatus translate_mail_status(
    core::MailFailureStatus status
) noexcept
{
    switch(status)
    {
    case core::MailFailureStatus::Ok:
        return meganz_account_generator::MailFailureStatus::Ok;
    case core::MailFailureStatus::Null:
        return meganz_account_generator::MailFailureStatus::Null;
    case core::MailFailureStatus::InvalidArgument:
        return meganz_account_generator::MailFailureStatus::InvalidArgument;
    case core::MailFailureStatus::Request:
        return meganz_account_generator::MailFailureStatus::Request;
    case core::MailFailureStatus::ResponseParse:
        return meganz_account_generator::MailFailureStatus::ResponseParse;
    case core::MailFailureStatus::TokenParse:
        return meganz_account_generator::MailFailureStatus::TokenParse;
    case core::MailFailureStatus::Json:
        return meganz_account_generator::MailFailureStatus::Json;
    case core::MailFailureStatus::Internal:
        return meganz_account_generator::MailFailureStatus::Internal;
    case core::MailFailureStatus::Unknown:
        return meganz_account_generator::MailFailureStatus::Unknown;
    }

    return meganz_account_generator::MailFailureStatus::Unknown;
}

[[nodiscard]] core::AccountGeneratorConfig to_core_config(
    meganz_account_generator::AccountGeneratorConfig config
)
{
    return core::AccountGeneratorConfig{
        .app_key = std::move(config.app_key),
        .password = std::move(config.password),
        .display_name = std::move(config.display_name),
        .proxy = std::move(config.proxy),
        .base_path = std::move(config.base_path),
        .user_agent = std::move(config.user_agent),
        .timeout = config.timeout,
        .poll_interval = config.poll_interval,
        .request_timeout = config.request_timeout,
        .danger_accept_invalid_certs = config.danger_accept_invalid_certs,
        .worker_thread_count = config.worker_thread_count,
    };
}

[[nodiscard]] meganz_account_generator::GeneratedAccount to_public_account(
    core::GeneratedAccount account
)
{
    return meganz_account_generator::GeneratedAccount{
        .email = std::move(account.email),
        .password = std::move(account.password),
        .display_name = std::move(account.display_name),
    };
}

[[noreturn]] void throw_public_error()
{
    try
    {
        throw;
    }
    catch(const core::MailFailureError& error)
    {
        throw meganz_account_generator::MailFailureError(
            translate_mail_status(error.status()),
            error.what()
        );
    }
    catch(const core::MegaSignupError& error)
    {
        throw meganz_account_generator::MegaSignupError(error.what(), error.error_code());
    }
    catch(const core::ConfirmationEmailTimeoutError& error)
    {
        throw meganz_account_generator::ConfirmationEmailTimeoutError(error.timeout());
    }
    catch(const core::ConfirmationLinkParseError& error)
    {
        throw meganz_account_generator::ConfirmationLinkParseError(error.what());
    }
    catch(const core::AccountGenerationError& error)
    {
        throw meganz_account_generator::AccountGenerationError(error.what());
    }
}

} // namespace

namespace meganz_account_generator
{

AccountGenerationError::AccountGenerationError(std::string message)
    : std::runtime_error(std::move(message))
{
}

MailFailureError::MailFailureError(MailFailureStatus status, std::string message)
    : AccountGenerationError(std::move(message))
    , status_(status)
{
}

MailFailureStatus MailFailureError::status() const noexcept
{
    return status_;
}

MegaSignupError::MegaSignupError(std::string message, std::optional<int> error_code)
    : AccountGenerationError(std::move(message))
    , error_code_(std::move(error_code))
{
}

const std::optional<int>& MegaSignupError::error_code() const noexcept
{
    return error_code_;
}

ConfirmationEmailTimeoutError::ConfirmationEmailTimeoutError(std::chrono::milliseconds timeout)
    : AccountGenerationError("Timed out after " + std::to_string(timeout.count()) +
      "ms while waiting for a MEGA confirmation email")
    , timeout_(timeout)
{
}

std::chrono::milliseconds ConfirmationEmailTimeoutError::timeout() const noexcept
{
    return timeout_;
}

ConfirmationLinkParseError::ConfirmationLinkParseError(std::string message)
    : AccountGenerationError(std::move(message))
{
}

struct AccountGenerator::Impl
{
    explicit Impl(AccountGeneratorConfig config)
        : generator_(to_core_config(std::move(config)))
    {
    }

    [[nodiscard]] GeneratedAccount generate()
    {
        return to_public_account(generator_.generate());
    }

    core::AccountGenerator generator_;
};

AccountGenerator::AccountGenerator(AccountGeneratorConfig config)
{
    try
    {
        impl_ = std::make_unique<Impl>(std::move(config));
    }
    catch(...)
    {
        throw_public_error();
    }
}

AccountGenerator::~AccountGenerator() = default;

AccountGenerator::AccountGenerator(AccountGenerator&& other) noexcept = default;

AccountGenerator& AccountGenerator::operator=(AccountGenerator&& other) noexcept = default;

GeneratedAccount AccountGenerator::generate()
{
    if(impl_ == nullptr)
    {
        throw std::logic_error("AccountGenerator is not initialized");
    }

    try
    {
        return impl_->generate();
    }
    catch(...)
    {
        throw_public_error();
    }
}

} // namespace meganz_account_generator
