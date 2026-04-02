#ifndef MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MAIL_GUERRILLAMAIL_CLIENT_DETAIL_HPP
#define MEGANZ_ACCOUNT_GENERATOR_CPP_SRC_MAIL_GUERRILLAMAIL_CLIENT_DETAIL_HPP

#include "mail/guerrillamail_client.hpp"

#include <guerrillamail_client.h>

namespace mail::detail
{

[[nodiscard]] GuerrillaMailStatus translate_status(gm_status_t status) noexcept;
[[nodiscard]] MessageSummary to_message_summary(const gm_message_t& message);
[[nodiscard]] EmailDetails to_email_details(const gm_email_details_t& details);

} // namespace mail::detail

#endif
