#include "mail/guerrillamail_client_detail.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace
{

void require(bool condition, std::string_view message)
{
    if(condition)
    {
        return;
    }

    std::cerr << "mail_wrapper_detail_test failed: " << message << '\n';
    std::exit(1);
}

gm_string_t borrow(std::string& value)
{
    return gm_string_t{
        .ptr = value.data(),
        .len = value.size(),
    };
}

void test_status_translation()
{
    require(
        mail::detail::translate_status(GM_ERR_INVALID_ARGUMENT)
            == mail::GuerrillaMailStatus::InvalidArgument,
        "invalid-argument status should map to the C++ enum"
    );
    require(
        mail::detail::translate_status(GM_ERR_RESPONSE_PARSE)
            == mail::GuerrillaMailStatus::ResponseParse,
        "response-parse status should map to the C++ enum"
    );
    require(
        mail::detail::translate_status(static_cast<gm_status_t>(999))
            == mail::GuerrillaMailStatus::Unknown,
        "unknown statuses should map to Unknown"
    );
}

void test_message_conversion()
{
    std::string mail_id = "mail-123";
    std::string mail_from = "mega@example.com";
    std::string mail_subject = "Confirm";
    std::string mail_timestamp = "1712055262";

    const gm_message_t message{
        .mail_id = borrow(mail_id),
        .mail_from = borrow(mail_from),
        .mail_subject = borrow(mail_subject),
        .mail_excerpt = {},
        .mail_timestamp = borrow(mail_timestamp),
    };

    const auto summary = mail::detail::to_message_summary(message);
    require(summary.mail_id == "mail-123", "message id should be copied");
    require(summary.mail_from == "mega@example.com", "sender should be copied");
    require(summary.mail_subject == "Confirm", "subject should be copied");
    require(summary.mail_excerpt.empty(), "empty excerpt should stay empty");
    require(summary.mail_timestamp == "1712055262", "timestamp should be copied");
}

void test_message_conversion_handles_null_fields()
{
    const gm_message_t message{
        .mail_id = {},
        .mail_from = {},
        .mail_subject = {},
        .mail_excerpt = {},
        .mail_timestamp = {},
    };

    const auto summary = mail::detail::to_message_summary(message);
    require(summary.mail_id.empty(), "null message ids should map to empty strings");
    require(summary.mail_from.empty(), "null senders should map to empty strings");
    require(summary.mail_subject.empty(), "null subjects should map to empty strings");
    require(summary.mail_excerpt.empty(), "null excerpts should map to empty strings");
    require(summary.mail_timestamp.empty(), "null timestamps should map to empty strings");
}

void test_email_details_conversion()
{
    std::string mail_id = "mail-456";
    std::string mail_from = "mega@example.com";
    std::string mail_subject = "Welcome";
    std::string mail_body = "body";
    std::string mail_timestamp = "1712055300";

    const gm_email_details_t with_attachment{
        .mail_id = borrow(mail_id),
        .mail_from = borrow(mail_from),
        .mail_subject = borrow(mail_subject),
        .mail_body = borrow(mail_body),
        .mail_timestamp = borrow(mail_timestamp),
        .attachment_count = 2,
        .has_attachment_count = true,
    };

    const auto detailed = mail::detail::to_email_details(with_attachment);
    require(detailed.mail_id == "mail-456", "detail mail id should be copied");
    require(detailed.mail_from == "mega@example.com", "detail sender should be copied");
    require(detailed.mail_subject == "Welcome", "detail subject should be copied");
    require(detailed.mail_body == "body", "body should be copied");
    require(detailed.mail_timestamp == "1712055300", "detail timestamp should be copied");
    require(
        detailed.attachment_count == std::optional<std::uint32_t>{2},
        "attachment count should be preserved when present"
    );

    const gm_email_details_t without_attachment{
        .mail_id = {},
        .mail_from = {},
        .mail_subject = {},
        .mail_body = {},
        .mail_timestamp = {},
        .attachment_count = 99,
        .has_attachment_count = false,
    };

    const auto missing = mail::detail::to_email_details(without_attachment);
    require(!missing.attachment_count.has_value(), "missing attachment count should map to nullopt");
    require(missing.mail_id.empty(), "null strings should map to empty strings");
}

} // namespace

int main()
{
    test_status_translation();
    test_message_conversion();
    test_message_conversion_handles_null_fields();
    test_email_details_conversion();
    return 0;
}
