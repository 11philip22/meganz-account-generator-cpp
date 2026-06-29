#include "meganz_account_generator/account_generator.hpp"

#include "mega/mega_api_client.hpp"

#include <guerrillamail/client.hpp>
#include <guerrillamail/error.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

namespace
{

struct NameParts
{
    std::string first_name;
    std::string last_name;
};

[[nodiscard]] std::string trim_copy(std::string_view value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if(first == std::string_view::npos)
    {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(first, last - first + 1));
}

[[nodiscard]] NameParts split_display_name(std::string_view display_name)
{
    const auto trimmed_name = trim_copy(display_name);
    if(trimmed_name.empty())
    {
        throw std::invalid_argument("display_name must not be empty");
    }

    const auto separator = trimmed_name.find(' ');
    if(separator == std::string::npos)
    {
        return NameParts{
            .first_name = trimmed_name,
            .last_name = {},
        };
    }

    return NameParts{
        .first_name = trimmed_name.substr(0, separator),
        .last_name = trim_copy(trimmed_name.substr(separator + 1)),
    };
}

[[nodiscard]] meganz_account_generator::MailFailureStatus translate_mail_status(
    guerrillamail::ErrorCode code
) noexcept
{
    switch(code)
    {
    case guerrillamail::ErrorCode::invalid_argument:
        return meganz_account_generator::MailFailureStatus::InvalidArgument;
    case guerrillamail::ErrorCode::transport:
    case guerrillamail::ErrorCode::http_status:
        return meganz_account_generator::MailFailureStatus::Request;
    case guerrillamail::ErrorCode::token_parse:
        return meganz_account_generator::MailFailureStatus::TokenParse;
    case guerrillamail::ErrorCode::response_parse:
        return meganz_account_generator::MailFailureStatus::ResponseParse;
    case guerrillamail::ErrorCode::json_parse:
        return meganz_account_generator::MailFailureStatus::Json;
    case guerrillamail::ErrorCode::internal:
        return meganz_account_generator::MailFailureStatus::Internal;
    }

    return meganz_account_generator::MailFailureStatus::Unknown;
}

[[nodiscard]] std::string make_mega_operation_message(
    std::string_view operation,
    std::string_view detail
)
{
    std::ostringstream stream;
    stream << "MEGA " << operation << " failed";
    if(!detail.empty())
    {
        stream << ": " << detail;
    }

    return stream.str();
}

[[nodiscard]] std::string make_timeout_message(std::chrono::milliseconds timeout)
{
    std::ostringstream stream;
    stream << "Timed out after " << timeout.count()
           << "ms while waiting for a MEGA confirmation email";
    return stream.str();
}

[[nodiscard]] std::string copy_nullable_string(const char* value)
{
    if(value == nullptr)
    {
        return {};
    }

    return value;
}

void validate_config(const meganz_account_generator::AccountGeneratorConfig& config)
{
    if(config.app_key.empty())
    {
        throw std::invalid_argument("AccountGenerator requires a non-empty app_key");
    }

    if(trim_copy(config.email_alias).empty())
    {
        throw std::invalid_argument("AccountGenerator requires a non-empty email_alias");
    }

    if(config.password.empty())
    {
        throw std::invalid_argument("AccountGenerator requires a non-empty password");
    }

    if(trim_copy(config.display_name).empty())
    {
        throw std::invalid_argument("AccountGenerator requires a non-empty display_name");
    }

    if(config.timeout <= std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument("AccountGenerator timeout must be greater than zero");
    }

    if(config.poll_interval <= std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument("AccountGenerator poll_interval must be greater than zero");
    }

    if(config.request_timeout <= std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument("AccountGenerator request_timeout must be greater than zero");
    }
}

[[nodiscard]] meganz_account_generator::AccountGeneratorConfig validate_and_normalize_config(
    meganz_account_generator::AccountGeneratorConfig config
)
{
    validate_config(config);
    config.email_alias = trim_copy(config.email_alias);
    config.display_name = trim_copy(config.display_name);
    if(config.proxy && config.proxy->empty())
    {
        config.proxy.reset();
    }
    return config;
}

template <typename Operation>
decltype(auto) wrap_mail_operation(
    std::string_view operation,
    Operation&& operation_callback
)
{
    try
    {
        return std::forward<Operation>(operation_callback)();
    }
    catch(const guerrillamail::Error& error)
    {
        throw meganz_account_generator::MailFailureError(
            translate_mail_status(error.code()),
            "GuerrillaMail " + std::string(operation) + " failed: " + error.what()
        );
    }
}

template <typename Operation>
decltype(auto) wrap_mega_operation(
    std::string_view operation,
    Operation&& operation_callback
)
{
    try
    {
        return std::forward<Operation>(operation_callback)();
    }
    catch(const mega_integration::RequestTimeoutError& error)
    {
        throw meganz_account_generator::MegaSignupError(
            make_mega_operation_message(operation, error.what()),
            std::nullopt
        );
    }
    catch(const mega_integration::MegaRequestError& error)
    {
        throw meganz_account_generator::MegaSignupError(
            make_mega_operation_message(operation, error.what()),
            error.error_code()
        );
    }
}

class EmailCleanupGuard
{
public:
    explicit EmailCleanupGuard(guerrillamail::Client& mail_client) noexcept
        : mail_client_(mail_client)
    {
    }

    ~EmailCleanupGuard()
    {
        if(email_.empty())
        {
            return;
        }

        try
        {
            mail_client_.delete_email(email_);
        }
        catch(...)
        {
        }
    }

    void set_email(std::string email)
    {
        email_ = std::move(email);
    }

private:
    guerrillamail::Client& mail_client_;
    std::string email_;
};

[[nodiscard]] bool is_probable_mega_message(const guerrillamail::Message& message)
{
    const auto contains_case_insensitive = [](std::string_view haystack, std::string_view needle)
    {
        return std::search(
            haystack.begin(),
            haystack.end(),
            needle.begin(),
            needle.end(),
            [](char left, char right)
            {
                return std::tolower(static_cast<unsigned char>(left)) ==
                    std::tolower(static_cast<unsigned char>(right));
            }
        ) != haystack.end();
    };

    return contains_case_insensitive(message.mail_from, "mega") ||
        contains_case_insensitive(message.mail_subject, "mega");
}

[[nodiscard]] std::optional<std::string> extract_confirmation_link(std::string_view body)
{
    static constexpr std::array kPrefixes{
        std::string_view{"https://mega.nz/#confirm"},
        std::string_view{"https://mega.nz/confirm"},
    };

    const auto is_key_character = [](char value)
    {
        const auto unsigned_value = static_cast<unsigned char>(value);
        return std::isalnum(unsigned_value) != 0 || value == '_' || value == '-';
    };

    for(const auto prefix : kPrefixes)
    {
        std::size_t search_offset = 0;
        while(true)
        {
            const auto position = body.find(prefix, search_offset);
            if(position == std::string_view::npos)
            {
                break;
            }

            auto end = position + prefix.size();
            while(end < body.size() && is_key_character(body[end]))
            {
                ++end;
            }

            if(end > position + prefix.size())
            {
                return std::string(body.substr(position, end - position));
            }

            search_offset = position + prefix.size();
        }
    }

    return std::nullopt;
}

void validate_confirmed_email_value(
    std::string_view confirmed_email,
    std::string_view expected_email
)
{
    if(confirmed_email.empty())
    {
        throw meganz_account_generator::MegaSignupError(
            "MEGA confirm account finished without a confirmed email",
            std::nullopt
        );
    }

    if(confirmed_email != expected_email)
    {
        throw meganz_account_generator::MegaSignupError(
            "MEGA confirmation returned a different email than the generated account",
            std::nullopt
        );
    }
}

[[nodiscard]] guerrillamail::Client make_mail_client(
    const meganz_account_generator::AccountGeneratorConfig& config
)
{
    return wrap_mail_operation("initialize client", [&config]
    {
        guerrillamail::ClientOptions options;
        options.proxy = config.proxy;
        options.timeout = config.request_timeout;
        options.verify_tls = !config.danger_accept_invalid_certs;
        return guerrillamail::Client::create(options);
    });
}

[[nodiscard]] std::string extract_session_key(const mega_integration::RequestResult& result)
{
    if(result.request == nullptr)
    {
        throw meganz_account_generator::MegaSignupError(
            "MEGA create account finished without request data",
            std::nullopt
        );
    }

    const auto session_key = copy_nullable_string(result.request->getSessionKey());
    if(session_key.empty())
    {
        throw meganz_account_generator::MegaSignupError(
            "MEGA create account finished without a session key",
            std::nullopt
        );
    }

    return session_key;
}

void validate_confirmed_email(
    const mega_integration::RequestResult& result,
    std::string_view expected_email
)
{
    if(result.request == nullptr)
    {
        throw meganz_account_generator::MegaSignupError(
            "MEGA confirm account finished without request data",
            std::nullopt
        );
    }

    validate_confirmed_email_value(
        copy_nullable_string(result.request->getEmail()),
        expected_email
    );
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
    : AccountGenerationError(make_timeout_message(timeout))
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
        : config_(validate_and_normalize_config(std::move(config)))
        , mail_client_(make_mail_client(config_))
        , mega_client_(config_.app_key, config_.request_timeout)
    {
        if(config_.proxy)
        {
            static_cast<void>(wrap_mega_operation("set proxy", [this]
            {
                return mega_client_.set_proxy(*config_.proxy);
            }));
        }
    }

    [[nodiscard]] GeneratedAccount generate()
    {
        const auto email = wrap_mail_operation("create_email", [this]
        {
            return mail_client_.create_email(config_.email_alias);
        });

        EmailCleanupGuard cleanup(mail_client_);
        cleanup.set_email(email);

        const auto name_parts = split_display_name(config_.display_name);

        const auto create_result = wrap_mega_operation("create account", [this, &email, &name_parts]
        {
            return mega_client_.create_account(
                email,
                config_.password,
                name_parts.first_name,
                name_parts.last_name
            );
        });

        const auto session_key = extract_session_key(create_result);
        static_cast<void>(wrap_mega_operation("resume create account", [this, &session_key]
        {
            return mega_client_.resume_create_account(session_key);
        }));

        const auto confirmation_link = wait_for_confirmation_link(email);
        const auto confirm_result = wrap_mega_operation("confirm account", [this, &confirmation_link]
        {
            return mega_client_.confirm_account(confirmation_link);
        });

        validate_confirmed_email(confirm_result, email);
        return GeneratedAccount{
            .email = email,
            .password = config_.password,
            .display_name = config_.display_name,
        };
    }

private:
    [[nodiscard]] std::string wait_for_confirmation_link(std::string_view email)
    {
        const auto start = std::chrono::steady_clock::now();
        bool saw_probable_mega_mail = false;

        while(true)
        {
            if(std::chrono::steady_clock::now() - start >= config_.timeout)
            {
                if(saw_probable_mega_mail)
                {
                    throw ConfirmationLinkParseError(
                        "Timed out while waiting for a MEGA confirmation link in a likely MEGA email"
                    );
                }

                throw ConfirmationEmailTimeoutError(config_.timeout);
            }

            const auto messages = wrap_mail_operation("list_messages", [this, email]
            {
                return mail_client_.get_messages(email);
            });

            for(const auto& message : messages)
            {
                if(!is_probable_mega_message(message))
                {
                    continue;
                }

                saw_probable_mega_mail = true;

                const auto details = wrap_mail_operation("fetch_email", [this, email, &message]
                {
                    return mail_client_.fetch_email(email, message.mail_id);
                });

                const auto confirmation_link = extract_confirmation_link(details.mail_body);
                if(confirmation_link)
                {
                    return *confirmation_link;
                }
            }

            std::this_thread::sleep_for(config_.poll_interval);
        }
    }

    AccountGeneratorConfig config_;
    guerrillamail::Client mail_client_;
    mega_integration::MegaApiClient mega_client_;
};

AccountGenerator::AccountGenerator(AccountGeneratorConfig config)
    : impl_(std::make_unique<Impl>(std::move(config)))
{
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

    return impl_->generate();
}

} // namespace meganz_account_generator
