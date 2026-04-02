#include "mail/guerrillamail_client.hpp"

#include "mail/guerrillamail_client_detail.hpp"

#include <limits>
#include <memory>
#include <utility>

namespace
{

using BuilderHandle = std::unique_ptr<gm_builder_t, decltype(&gm_builder_free)>;
using ClientHandle = std::unique_ptr<gm_client_t, decltype(&gm_client_free)>;
using EmailDetailsHandle = std::unique_ptr<gm_email_details_t, decltype(&gm_email_details_free)>;

class StringHandle
{
public:
    StringHandle() = default;
    ~StringHandle()
    {
        gm_string_free(&value_);
    }

    StringHandle(const StringHandle&) = delete;
    StringHandle& operator=(const StringHandle&) = delete;

    [[nodiscard]] gm_string_t* get() noexcept
    {
        return &value_;
    }

    [[nodiscard]] std::string to_string() const
    {
        if(value_.ptr == nullptr || value_.len == 0)
        {
            return {};
        }

        return {value_.ptr, value_.len};
    }

private:
    gm_string_t value_{};
};

class MessageListHandle
{
public:
    MessageListHandle() = default;
    ~MessageListHandle()
    {
        gm_message_list_free(&value_);
    }

    MessageListHandle(const MessageListHandle&) = delete;
    MessageListHandle& operator=(const MessageListHandle&) = delete;

    [[nodiscard]] gm_message_list_t* get() noexcept
    {
        return &value_;
    }

    [[nodiscard]] const gm_message_list_t& value() const noexcept
    {
        return value_;
    }

private:
    gm_message_list_t value_{};
};

[[nodiscard]] std::string copy_string(const gm_string_t& value)
{
    if(value.ptr == nullptr || value.len == 0)
    {
        return {};
    }

    return {value.ptr, value.len};
}

[[nodiscard]] std::string make_status_message(gm_status_t status)
{
    switch(status)
    {
    case GM_OK:
        return "operation succeeded";
    case GM_ERR_NULL:
        return "unexpected null pointer";
    case GM_ERR_INVALID_ARGUMENT:
        return "invalid argument";
    case GM_ERR_REQUEST:
        return "request failed";
    case GM_ERR_RESPONSE_PARSE:
        return "response parse failed";
    case GM_ERR_TOKEN_PARSE:
        return "token parse failed";
    case GM_ERR_JSON:
        return "json error";
    case GM_ERR_INTERNAL:
        return "internal error";
    }

    return "unknown GuerrillaMail error";
}

[[nodiscard]] std::string current_error_message(gm_status_t status)
{
    const char* message = gm_last_error_message();
    if(message != nullptr && message[0] != '\0')
    {
        return message;
    }

    return make_status_message(status);
}

void throw_on_error(gm_status_t status)
{
    if(status == GM_OK)
    {
        return;
    }

    throw mail::GuerrillaMailError(
        mail::detail::translate_status(status),
        current_error_message(status)
    );
}

[[nodiscard]] std::uint64_t to_timeout_ms(std::chrono::milliseconds timeout)
{
    if(timeout.count() < 0)
    {
        throw std::invalid_argument("timeout must not be negative");
    }

    using UnsignedRep = std::uint64_t;
    constexpr auto kMax = std::numeric_limits<UnsignedRep>::max();
    const auto count = timeout.count();
    if(static_cast<unsigned long long>(count) > kMax)
    {
        throw std::invalid_argument("timeout is too large");
    }

    return static_cast<UnsignedRep>(count);
}

void apply_builder_options(gm_builder_t* builder, const mail::ClientOptions& options)
{
    if(options.proxy)
    {
        throw_on_error(gm_builder_set_proxy(builder, options.proxy->c_str()));
    }

    if(options.user_agent)
    {
        throw_on_error(gm_builder_set_user_agent(builder, options.user_agent->c_str()));
    }

    if(options.timeout)
    {
        throw_on_error(gm_builder_set_timeout_ms(builder, to_timeout_ms(*options.timeout)));
    }

    if(options.danger_accept_invalid_certs)
    {
        throw_on_error(gm_builder_set_danger_accept_invalid_certs(builder, true));
    }
}

} // namespace

namespace mail
{

struct GuerrillaMailClient::Impl
{
    gm_client_t* client{nullptr};

    explicit Impl(gm_client_t* raw_client) noexcept
        : client(raw_client)
    {
    }

    ~Impl()
    {
        if(client != nullptr)
        {
            gm_client_free(client);
        }
    }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
};

namespace detail
{

GuerrillaMailStatus translate_status(gm_status_t status) noexcept
{
    switch(status)
    {
    case GM_OK:
        return GuerrillaMailStatus::Ok;
    case GM_ERR_NULL:
        return GuerrillaMailStatus::Null;
    case GM_ERR_INVALID_ARGUMENT:
        return GuerrillaMailStatus::InvalidArgument;
    case GM_ERR_REQUEST:
        return GuerrillaMailStatus::Request;
    case GM_ERR_RESPONSE_PARSE:
        return GuerrillaMailStatus::ResponseParse;
    case GM_ERR_TOKEN_PARSE:
        return GuerrillaMailStatus::TokenParse;
    case GM_ERR_JSON:
        return GuerrillaMailStatus::Json;
    case GM_ERR_INTERNAL:
        return GuerrillaMailStatus::Internal;
    }

    return GuerrillaMailStatus::Unknown;
}

MessageSummary to_message_summary(const gm_message_t& message)
{
    return MessageSummary{
        .mail_id = copy_string(message.mail_id),
        .mail_from = copy_string(message.mail_from),
        .mail_subject = copy_string(message.mail_subject),
        .mail_excerpt = copy_string(message.mail_excerpt),
        .mail_timestamp = copy_string(message.mail_timestamp),
    };
}

EmailDetails to_email_details(const gm_email_details_t& details)
{
    return EmailDetails{
        .mail_id = copy_string(details.mail_id),
        .mail_from = copy_string(details.mail_from),
        .mail_subject = copy_string(details.mail_subject),
        .mail_body = copy_string(details.mail_body),
        .mail_timestamp = copy_string(details.mail_timestamp),
        .attachment_count = details.has_attachment_count
            ? std::optional<std::uint32_t>{details.attachment_count}
            : std::nullopt,
    };
}

} // namespace detail

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
    : impl_()
{
    gm_last_error_clear();
    gm_client_t* client = nullptr;
    throw_on_error(gm_client_new_default(&client));
    ClientHandle client_handle(client, &gm_client_free);
    impl_ = std::make_unique<Impl>(client_handle.release());
}

GuerrillaMailClient::GuerrillaMailClient(const ClientOptions& options)
    : impl_()
{
    gm_last_error_clear();

    gm_builder_t* raw_builder = nullptr;
    throw_on_error(gm_builder_new(&raw_builder));

    BuilderHandle builder(raw_builder, &gm_builder_free);
    apply_builder_options(builder.get(), options);

    gm_client_t* raw_client = nullptr;
    throw_on_error(gm_builder_build(builder.get(), &raw_client));
    ClientHandle client_handle(raw_client, &gm_client_free);
    impl_ = std::make_unique<Impl>(client_handle.release());
}

GuerrillaMailClient::~GuerrillaMailClient() = default;

GuerrillaMailClient::GuerrillaMailClient(GuerrillaMailClient&& other) noexcept
    : impl_(std::move(other.impl_))
{
}

GuerrillaMailClient& GuerrillaMailClient::operator=(GuerrillaMailClient&& other) noexcept
{
    if(this == &other)
    {
        return *this;
    }

    impl_ = std::move(other.impl_);
    return *this;
}

std::string GuerrillaMailClient::create_email(std::string_view alias) const
{
    const std::string owned_alias(alias);
    StringHandle email;
    throw_on_error(gm_client_create_email(impl_->client, owned_alias.c_str(), email.get()));
    return email.to_string();
}

std::vector<MessageSummary> GuerrillaMailClient::list_messages(std::string_view email) const
{
    const std::string owned_email(email);
    MessageListHandle messages;
    throw_on_error(gm_client_get_messages(impl_->client, owned_email.c_str(), messages.get()));

    std::vector<MessageSummary> result;
    result.reserve(messages.value().len);

    for(std::size_t index = 0; index < messages.value().len; ++index)
    {
        result.push_back(detail::to_message_summary(messages.value().ptr[index]));
    }

    return result;
}

EmailDetails GuerrillaMailClient::fetch_email(
    std::string_view email,
    std::string_view mail_id
) const
{
    const std::string owned_email(email);
    const std::string owned_mail_id(mail_id);
    gm_email_details_t* details = nullptr;
    throw_on_error(
        gm_client_fetch_email(
            impl_->client,
            owned_email.c_str(),
            owned_mail_id.c_str(),
            &details
        )
    );

    EmailDetailsHandle handle(details, &gm_email_details_free);
    return detail::to_email_details(*handle);
}

bool GuerrillaMailClient::delete_email(std::string_view email) const
{
    const std::string owned_email(email);
    bool deleted = false;
    throw_on_error(gm_client_delete_email(impl_->client, owned_email.c_str(), &deleted));
    return deleted;
}

} // namespace mail
