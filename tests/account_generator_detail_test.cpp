#include "core/account_generator.hpp"
#include "core/account_generator_detail.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

namespace
{

void require(bool condition, std::string_view message)
{
    if(condition)
    {
        return;
    }

    std::cerr << "account_generator_detail_test failed: " << message << '\n';
    std::exit(1);
}

template <typename Callback>
void require_mega_signup_error(Callback&& callback, std::string_view message)
{
    try
    {
        std::forward<Callback>(callback)();
    }
    catch(const core::MegaSignupError&)
    {
        return;
    }

    std::cerr << "account_generator_detail_test failed: " << message << '\n';
    std::exit(1);
}

void test_extract_confirmation_link_from_plain_text_body()
{
    const auto link = core::detail::extract_confirmation_link(
        "Confirm your account: https://mega.nz/#confirmAbC123_-"
    );

    require(link.has_value(), "plain-text confirmation link should be extracted");
    require(
        *link == "https://mega.nz/#confirmAbC123_-",
        "plain-text confirmation link should round-trip"
    );
}

void test_extract_confirmation_link_from_html_body()
{
    const auto link = core::detail::extract_confirmation_link(
        R"(<a href="https://mega.nz/confirmZXcv_123">Confirm account</a>)"
    );

    require(link.has_value(), "HTML confirmation link should be extracted");
    require(
        *link == "https://mega.nz/confirmZXcv_123",
        "HTML confirmation link should round-trip"
    );
}

void test_extract_confirmation_link_returns_empty_for_unrelated_body()
{
    const auto link = core::detail::extract_confirmation_link(
        "No MEGA confirmation data appears in this body."
    );

    require(!link.has_value(), "unrelated message bodies should not produce a confirmation link");
}

void test_extract_confirmation_link_rejects_bare_prefix_without_key()
{
    const auto link = core::detail::extract_confirmation_link(
        "Incomplete link: https://mega.nz/#confirm and nothing else."
    );

    require(!link.has_value(), "bare confirmation prefixes should not be treated as valid links");
}

void test_extract_confirmation_link_skips_invalid_prefix_and_finds_later_valid_link()
{
    const auto link = core::detail::extract_confirmation_link(
        "Broken: https://mega.nz/confirm?ignored Later: https://mega.nz/#confirmValid_123"
    );

    require(link.has_value(), "a later valid confirmation link should still be found");
    require(
        *link == "https://mega.nz/#confirmValid_123",
        "the first valid confirmation link should be returned"
    );
}

void test_probable_mega_message_heuristic()
{
    const mail::MessageSummary sender_match{
        .mail_from = "support@mega.nz",
    };
    const mail::MessageSummary subject_match{
        .mail_subject = "Welcome to MEGA",
    };
    const mail::MessageSummary no_match{
        .mail_from = "hello@example.com",
        .mail_subject = "Weekly digest",
    };

    require(
        core::detail::is_probable_mega_message(sender_match),
        "sender containing mega should be treated as a probable MEGA message"
    );
    require(
        core::detail::is_probable_mega_message(subject_match),
        "subject containing mega should be treated as a probable MEGA message"
    );
    require(
        !core::detail::is_probable_mega_message(no_match),
        "unrelated messages should not be treated as probable MEGA messages"
    );
}

void test_probable_mega_message_heuristic_is_case_insensitive()
{
    const mail::MessageSummary mixed_case_sender{
        .mail_from = "Support@Mega.NZ",
    };
    const mail::MessageSummary mixed_case_subject{
        .mail_subject = "Confirm your MeGa account",
    };

    require(
        core::detail::is_probable_mega_message(mixed_case_sender),
        "sender matching should be case-insensitive"
    );
    require(
        core::detail::is_probable_mega_message(mixed_case_subject),
        "subject matching should be case-insensitive"
    );
}

void test_split_display_name()
{
    const auto single_part = core::detail::split_display_name("Automation");
    require(single_part.first_name == "Automation", "single-part names should map to first_name");
    require(single_part.last_name.empty(), "single-part names should leave last_name empty");

    const auto multi_part = core::detail::split_display_name("  Automation Bot Runner  ");
    require(multi_part.first_name == "Automation", "first token should become first_name");
    require(
        multi_part.last_name == "Bot Runner",
        "remaining tokens should become last_name"
    );
}

void test_normalize_optional_string()
{
    const auto missing_value = core::detail::normalize_optional_string(std::nullopt);
    require(!missing_value.has_value(), "missing optional strings should stay disengaged");

    const auto empty_value = core::detail::normalize_optional_string(std::string{});
    require(!empty_value.has_value(), "empty optional strings should normalize to nullopt");

    const auto non_empty_value = core::detail::normalize_optional_string(std::string{"value"});
    require(non_empty_value.has_value(), "non-empty optional strings should be preserved");
    require(
        *non_empty_value == "value",
        "non-empty optional strings should retain their original contents"
    );
}

void test_validate_confirmed_email_value()
{
    core::detail::validate_confirmed_email_value(
        "generated@example.com",
        "generated@example.com"
    );

    require_mega_signup_error(
        []
        {
            core::detail::validate_confirmed_email_value("", "generated@example.com");
        },
        "empty confirmed email should throw MegaSignupError"
    );

    require_mega_signup_error(
        []
        {
            core::detail::validate_confirmed_email_value(
                "different@example.com",
                "generated@example.com"
            );
        },
        "mismatched confirmed email should throw MegaSignupError"
    );
}

void test_generate_random_alias_shape()
{
    const auto alias = core::detail::generate_random_alias(24);
    require(alias.size() == 24, "generated alias should respect the requested length");
    require(
        std::all_of(alias.begin(), alias.end(), [](char value)
        {
            return (value >= 'a' && value <= 'z') || (value >= '0' && value <= '9');
        }),
        "generated alias should contain only lowercase letters and digits"
    );
}

} // namespace

int main()
{
    test_extract_confirmation_link_from_plain_text_body();
    test_extract_confirmation_link_from_html_body();
    test_extract_confirmation_link_returns_empty_for_unrelated_body();
    test_extract_confirmation_link_rejects_bare_prefix_without_key();
    test_extract_confirmation_link_skips_invalid_prefix_and_finds_later_valid_link();
    test_probable_mega_message_heuristic();
    test_probable_mega_message_heuristic_is_case_insensitive();
    test_split_display_name();
    test_normalize_optional_string();
    test_validate_confirmed_email_value();
    test_generate_random_alias_shape();
    return 0;
}
