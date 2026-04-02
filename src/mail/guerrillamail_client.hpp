#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MAIL_GUERRILLAMAIL_CLIENT_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MAIL_GUERRILLAMAIL_CLIENT_HPP

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace mail
{

enum class GuerrillaMailStatus
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

struct MessageSummary
{
    std::string mail_id;
    std::string mail_from;
    std::string mail_subject;
    std::string mail_excerpt;
    std::string mail_timestamp;
};

struct EmailDetails
{
    std::string mail_id;
    std::string mail_from;
    std::string mail_subject;
    std::string mail_body;
    std::string mail_timestamp;
    std::optional<std::uint32_t> attachment_count;
};

struct ClientOptions
{
    std::optional<std::string> proxy;
    std::optional<std::chrono::milliseconds> timeout;
    std::optional<std::string> user_agent;
    bool danger_accept_invalid_certs{false};
};

class GuerrillaMailError : public std::runtime_error
{
public:
    GuerrillaMailError(GuerrillaMailStatus status, std::string message);

    [[nodiscard]] GuerrillaMailStatus status() const noexcept;

private:
    GuerrillaMailStatus status_;
};

class GuerrillaMailClient
{
public:
    GuerrillaMailClient();
    explicit GuerrillaMailClient(const ClientOptions& options);
    ~GuerrillaMailClient();

    GuerrillaMailClient(const GuerrillaMailClient&) = delete;
    GuerrillaMailClient& operator=(const GuerrillaMailClient&) = delete;
    GuerrillaMailClient(GuerrillaMailClient&& other) noexcept;
    GuerrillaMailClient& operator=(GuerrillaMailClient&& other) noexcept;

    [[nodiscard]] std::string create_email(std::string_view alias) const;
    [[nodiscard]] std::vector<MessageSummary> list_messages(std::string_view email) const;
    [[nodiscard]] EmailDetails fetch_email(
        std::string_view email,
        std::string_view mail_id
    ) const;
    [[nodiscard]] bool delete_email(std::string_view email) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mail

#endif
