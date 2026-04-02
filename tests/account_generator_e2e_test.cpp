#include "core/account_generator.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <system_error>

namespace
{

constexpr int kSkipExitCode = 77;
constexpr std::string_view kDefaultDisplayName = "Automation Bot";
constexpr std::string_view kDefaultUserAgent = "meganz-account-generator-cpp-e2e";
constexpr auto kDefaultTimeout = std::chrono::minutes{5};
constexpr auto kDefaultPollInterval = std::chrono::seconds{5};
constexpr int kMaxCreateAttempts = 16;

[[nodiscard]] const char* get_env_raw(const char* name)
{
    return std::getenv(name);
}

[[nodiscard]] std::optional<std::string> get_env_string(const char* name)
{
    const char* value = get_env_raw(name);
    if(value == nullptr || value[0] == '\0')
    {
        return std::nullopt;
    }

    return std::string(value);
}

[[nodiscard]] std::optional<std::chrono::milliseconds> get_env_milliseconds(const char* name)
{
    const auto value = get_env_string(name);
    if(!value)
    {
        return std::nullopt;
    }

    try
    {
        return std::chrono::milliseconds(std::stoll(*value));
    }
    catch(const std::exception&)
    {
        throw std::runtime_error(
            std::string("Environment variable ") + name + " must be a valid integer"
        );
    }
}

class TemporaryBasePath
{
public:
    TemporaryBasePath()
    {
        path_ = create_unique_path();
    }

    ~TemporaryBasePath()
    {
        std::error_code error_code;
        std::filesystem::remove_all(path_, error_code);
    }

    [[nodiscard]] std::string string() const
    {
        return path_.string();
    }

private:
    [[nodiscard]] static std::string random_suffix(std::size_t length = 16)
    {
        static constexpr std::string_view kAlphabet = "abcdefghijklmnopqrstuvwxyz0123456789";

        std::random_device random_device;
        std::mt19937_64 engine(random_device());
        std::uniform_int_distribution<std::size_t> distribution(0, kAlphabet.size() - 1);

        std::string suffix;
        suffix.reserve(length);

        for(std::size_t index = 0; index < length; ++index)
        {
            suffix.push_back(kAlphabet[distribution(engine)]);
        }

        return suffix;
    }

    [[nodiscard]] static std::filesystem::path create_unique_path()
    {
        const auto temp_root = std::filesystem::temp_directory_path();

        for(int attempt = 0; attempt < kMaxCreateAttempts; ++attempt)
        {
            const auto candidate =
                temp_root / ("meganz-account-generator-cpp-e2e-" + random_suffix());

            std::error_code error_code;
            if(std::filesystem::create_directory(candidate, error_code))
            {
                return candidate;
            }

            if(error_code)
            {
                throw std::runtime_error(
                    "Failed to create temporary E2E base path: " + error_code.message()
                );
            }
        }

        throw std::runtime_error(
            "Failed to create a unique temporary E2E base path after " +
            std::to_string(kMaxCreateAttempts) +
            " attempts"
        );
    }

    std::filesystem::path path_;
};

void require(bool condition, std::string_view message)
{
    if(condition)
    {
        return;
    }

    throw std::runtime_error(std::string(message));
}

} // namespace

int main()
{
    try
    {
        const auto app_key = get_env_string("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_APP_KEY");
        const auto password = get_env_string("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PASSWORD");

        if(!app_key || !password)
        {
            std::cerr
                << "Skipping account_generator_e2e_test because "
                << "MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_APP_KEY and "
                << "MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PASSWORD are required.\n";
            return kSkipExitCode;
        }

        const auto display_name = get_env_string("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_DISPLAY_NAME")
            .value_or(std::string(kDefaultDisplayName));
        const auto proxy = get_env_string("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PROXY");
        const auto timeout = get_env_milliseconds("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_TIMEOUT_MS")
            .value_or(kDefaultTimeout);
        const auto poll_interval =
            get_env_milliseconds("MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_POLL_INTERVAL_MS")
                .value_or(kDefaultPollInterval);

        TemporaryBasePath temporary_base_path;
        core::AccountGeneratorConfig config{
            .app_key = *app_key,
            .password = *password,
            .display_name = display_name,
            .proxy = proxy,
            .base_path = temporary_base_path.string(),
            .user_agent = std::string(kDefaultUserAgent),
            .timeout = timeout,
            .poll_interval = poll_interval,
        };

        core::AccountGenerator generator(std::move(config));
        const auto account = generator.generate();

        require(!account.email.empty(), "generated account email should not be empty");
        require(account.password == *password, "generated account password should round-trip");
        require(
            account.display_name == display_name,
            "generated account display name should round-trip"
        );

        std::cout << "Created account: " << account.email << '\n';
        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << "account_generator_e2e_test failed: " << error.what() << '\n';
        return 1;
    }
}
