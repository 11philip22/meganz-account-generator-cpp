#include "mail/guerrillamail_client.hpp"

#include <guerrillamail/client.hpp>
#include <guerrillamail/error.hpp>

#include <memory>
#include <utility>

namespace
{

[[nodiscard]] mail::GuerrillaMailStatus translate_status(guerrillamail::ErrorCode code) noexcept
{
    switch(code)
    {
    case guerrillamail::ErrorCode::invalid_argument:
        return mail::GuerrillaMailStatus::InvalidArgument;
    case guerrillamail::ErrorCode::transport:
    case guerrillamail::ErrorCode::http_status:
        return mail::GuerrillaMailStatus::Request;
    case guerrillamail::ErrorCode::token_parse:
        return mail::GuerrillaMailStatus::TokenParse;
    case guerrillamail::ErrorCode::response_parse:
        return mail::GuerrillaMailStatus::ResponseParse;
    case guerrillamail::ErrorCode::json_parse:
        return mail::GuerrillaMailStatus::Json;
    case guerrillamail::ErrorCode::internal:
        return mail::GuerrillaMailStatus::Internal;
    }

    return mail::GuerrillaMailStatus::Unknown;
}

[[noreturn]] void throw_mail_error(const guerrillamail::Error& error)
{
    throw mail::GuerrillaMailError(translate_status(error.code()), error.what());
}

[[nodiscard]] guerrillamail::ClientOptions to_native_options(const mail::ClientOptions& options)
{
    guerrillamail::ClientOptions native_options;
    native_options.proxy = options.proxy;
    native_options.verify_tls = !options.danger_accept_invalid_certs;

    if(options.timeout)
    {
        native_options.timeout = *options.timeout;
    }

    return native_options;
}

[[nodiscard]] mail::MessageSummary to_message_summary(const guerrillamail::Message& message)
{
    return mail::MessageSummary{
        .mail_id = message.mail_id,
        .mail_from = message.mail_from,
        .mail_subject = message.mail_subject,
        .mail_excerpt = message.mail_excerpt,
        .mail_timestamp = message.mail_timestamp,
    };
}

[[nodiscard]] mail::EmailDetails to_email_details(const guerrillamail::EmailDetails& details)
{
    return mail::EmailDetails{
        .mail_id = details.mail_id,
        .mail_from = details.mail_from,
        .mail_subject = details.mail_subject,
        .mail_body = details.mail_body,
        .mail_timestamp = details.mail_timestamp,
        .attachment_count = details.attachment_count,
    };
}

} // namespace

namespace mail
{

struct GuerrillaMailClient::Impl
{
    explicit Impl(guerrillamail::Client native_client_in) noexcept
        : native_client(std::move(native_client_in))
    {
    }

    guerrillamail::Client native_client;
};

GuerrillaMailError::GuerrillaMailError(GuerrillaMailStatus status, std::string message)
    : std::runtime_error(std::move(message))
    , status_(status)
{
}

GuerrillaMailStatus GuerrillaMailError::status() const noexcept
{
    return status_;
}

GuerrillaMailClient::GuerrillaMailClient()
    : GuerrillaMailClient(ClientOptions{})
{
}

GuerrillaMailClient::GuerrillaMailClient(const ClientOptions& options)
    : impl_()
{
    try
    {
        impl_ = std::make_unique<Impl>(guerrillamail::Client::create(to_native_options(options)));
    }
    catch(const guerrillamail::Error& error)
    {
        throw_mail_error(error);
    }
}

GuerrillaMailClient::~GuerrillaMailClient() = default;

GuerrillaMailClient::GuerrillaMailClient(GuerrillaMailClient&& other) noexcept = default;

GuerrillaMailClient& GuerrillaMailClient::operator=(GuerrillaMailClient&& other) noexcept = default;

std::string GuerrillaMailClient::create_email(std::string_view alias) const
{
    try
    {
        return impl_->native_client.create_email(alias);
    }
    catch(const guerrillamail::Error& error)
    {
        throw_mail_error(error);
    }
}

std::vector<MessageSummary> GuerrillaMailClient::list_messages(std::string_view email) const
{
    try
    {
        const auto messages = impl_->native_client.get_messages(email);

        std::vector<MessageSummary> result;
        result.reserve(messages.size());

        for(const auto& message : messages)
        {
            result.push_back(to_message_summary(message));
        }

        return result;
    }
    catch(const guerrillamail::Error& error)
    {
        throw_mail_error(error);
    }
}

EmailDetails GuerrillaMailClient::fetch_email(
    std::string_view email,
    std::string_view mail_id
) const
{
    try
    {
        return to_email_details(impl_->native_client.fetch_email(email, mail_id));
    }
    catch(const guerrillamail::Error& error)
    {
        throw_mail_error(error);
    }
}

void GuerrillaMailClient::delete_email(std::string_view email) const
{
    try
    {
        impl_->native_client.delete_email(email);
    }
    catch(const guerrillamail::Error& error)
    {
        throw_mail_error(error);
    }
}

} // namespace mail
