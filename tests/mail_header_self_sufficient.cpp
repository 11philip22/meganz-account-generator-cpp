#include "mail/guerrillamail_client.hpp"

#include <type_traits>
#include <utility>

static_assert(
    std::is_same_v<
        decltype(std::declval<const mail::GuerrillaMailError&>().status()),
        mail::GuerrillaMailStatus
    >,
    "GuerrillaMailError::status() should expose the C++ status enum"
);

int main()
{
    mail::ClientOptions options{};
    mail::MessageSummary summary{};
    mail::EmailDetails details{};

    return static_cast<int>(options.danger_accept_invalid_certs)
        + static_cast<int>(summary.mail_id.size())
        + static_cast<int>(details.mail_subject.size());
}
