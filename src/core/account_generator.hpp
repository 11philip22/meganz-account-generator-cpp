#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CORE_ACCOUNT_GENERATOR_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_CORE_ACCOUNT_GENERATOR_HPP

#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace core
{

class AccountGenerationError : public std::runtime_error
{
public:
    explicit AccountGenerationError(std::string message);
};

enum class MailFailureStatus
{
    Ok,
    Null,
    InvalidArgument,
    Request,
    ResponseParse,
    TokenParse,
    Json,
    Internal,
    Unknown,
};

class MailFailureError : public AccountGenerationError
{
public:
    MailFailureError(MailFailureStatus status, std::string message);

    [[nodiscard]] MailFailureStatus status() const noexcept;

private:
    MailFailureStatus status_;
};

class MegaSignupError : public AccountGenerationError
{
public:
    MegaSignupError(std::string message, std::optional<int> error_code);

    [[nodiscard]] const std::optional<int>& error_code() const noexcept;

private:
    std::optional<int> error_code_;
};

class ConfirmationEmailTimeoutError : public AccountGenerationError
{
public:
    explicit ConfirmationEmailTimeoutError(std::chrono::milliseconds timeout);

    [[nodiscard]] std::chrono::milliseconds timeout() const noexcept;

private:
    std::chrono::milliseconds timeout_;
};

class ConfirmationLinkParseError : public AccountGenerationError
{
public:
    explicit ConfirmationLinkParseError(std::string message);
};

struct AccountGeneratorConfig
{
    std::string app_key;
    std::string password;
    std::string display_name;
    std::optional<std::string> proxy;
    std::optional<std::string> base_path;
    std::optional<std::string> user_agent;
    std::chrono::milliseconds timeout{std::chrono::minutes{5}};
    std::chrono::milliseconds poll_interval{std::chrono::seconds{5}};
    std::chrono::milliseconds request_timeout{std::chrono::seconds{30}};
    bool danger_accept_invalid_certs{false};
    unsigned worker_thread_count{1};
    int mega_client_type{0};
};

struct GeneratedAccount
{
    std::string email;
    std::string password;
    std::string display_name;
};

class AccountGenerator
{
public:
    explicit AccountGenerator(AccountGeneratorConfig config);
    ~AccountGenerator();

    AccountGenerator(const AccountGenerator&) = delete;
    AccountGenerator& operator=(const AccountGenerator&) = delete;
    AccountGenerator(AccountGenerator&& other) noexcept;
    AccountGenerator& operator=(AccountGenerator&& other) noexcept;

    [[nodiscard]] GeneratedAccount generate();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core

#endif
