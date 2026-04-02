#include "mega/mega_api_client.hpp"

#include <string_view>

template <typename T>
concept HasDeprecatedConfirmAccountOverload = requires(
    T& client,
    std::string_view link,
    std::string_view password
)
{
    client.confirm_account(link, password);
};

static_assert(
    !HasDeprecatedConfirmAccountOverload<mega_integration::MegaApiClient>,
    "MegaApiClient should not expose the deprecated confirm_account(link, password) overload"
);

int main()
{
    return 0;
}
