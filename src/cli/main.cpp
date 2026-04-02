#include "meganz_account_generator/account_generator.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{

constexpr std::string_view kDefaultDisplayName = "Automation Bot";

struct CliOptions
{
    std::optional<std::string> app_key;
    std::optional<std::string> password;
    std::string display_name{std::string(kDefaultDisplayName)};
    std::optional<std::string> proxy;
    std::chrono::milliseconds timeout{std::chrono::minutes{5}};
    std::chrono::milliseconds poll_interval{std::chrono::seconds{5}};
    bool show_help{false};
};

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

[[nodiscard]] std::chrono::milliseconds parse_milliseconds(
    std::string_view option_name,
    std::string_view value
)
{
    try
    {
        const auto parsed = std::stoll(std::string(value));
        if(parsed <= 0)
        {
            throw std::invalid_argument("non-positive");
        }

        return std::chrono::milliseconds(parsed);
    }
    catch(const std::exception&)
    {
        throw std::invalid_argument(
            std::string(option_name) + " must be a positive integer number of milliseconds"
        );
    }
}

[[nodiscard]] std::string require_value(
    std::string_view option_name,
    int argc,
    char* argv[],
    int& index
)
{
    if(index + 1 >= argc)
    {
        throw std::invalid_argument(std::string(option_name) + " requires a value");
    }

    ++index;
    return argv[index];
}

void print_help(std::ostream& stream)
{
    stream
        << "Usage:\n"
        << "  meganz_account_generator_cpp_cli --password <password> [options]\n\n"
        << "Required:\n"
        << "  --password <password>          Password for the created MEGA account\n\n"
        << "Runtime prerequisite:\n"
        << "  Provide a MEGA app key with --app-key <key> or the\n"
        << "  MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY environment variable.\n\n"
        << "Options:\n"
        << "  --app-key <key>               MEGA app key; overrides the environment variable\n"
        << "  --display-name <name>         Account display name (default: Automation Bot)\n"
        << "  --proxy <url>                 Proxy URL for MEGA and GuerrillaMail requests\n"
        << "  --timeout-ms <milliseconds>   Total time to wait for the confirmation email\n"
        << "  --poll-interval-ms <milliseconds>\n"
        << "                                Inbox polling interval while waiting for mail\n"
        << "  --help                        Show this help text\n\n"
        << "Notes:\n"
        << "  A real run requires network access to MEGA and GuerrillaMail.\n";
}

[[nodiscard]] CliOptions parse_arguments(int argc, char* argv[])
{
    CliOptions options;
    options.app_key = get_env_string("MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY");

    for(int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];

        if(argument == "--help" || argument == "-h")
        {
            options.show_help = true;
            return options;
        }

        if(argument == "--app-key")
        {
            options.app_key = require_value(argument, argc, argv, index);
            continue;
        }

        if(argument == "--password")
        {
            options.password = require_value(argument, argc, argv, index);
            continue;
        }

        if(argument == "--display-name")
        {
            options.display_name = require_value(argument, argc, argv, index);
            continue;
        }

        if(argument == "--proxy")
        {
            options.proxy = require_value(argument, argc, argv, index);
            continue;
        }

        if(argument == "--timeout-ms")
        {
            options.timeout = parse_milliseconds(
                argument,
                require_value(argument, argc, argv, index)
            );
            continue;
        }

        if(argument == "--poll-interval-ms")
        {
            options.poll_interval = parse_milliseconds(
                argument,
                require_value(argument, argc, argv, index)
            );
            continue;
        }

        throw std::invalid_argument(
            "Unknown argument: " + std::string(argument) + ". Use --help for usage."
        );
    }

    if(!options.app_key)
    {
        throw std::invalid_argument(
            "A MEGA app key is required. Provide --app-key or "
            "MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY."
        );
    }

    if(!options.password)
    {
        throw std::invalid_argument("--password is required");
    }

    return options;
}

[[nodiscard]] meganz_account_generator::AccountGeneratorConfig make_config(
    CliOptions options
)
{
    return meganz_account_generator::AccountGeneratorConfig{
        .app_key = std::move(*options.app_key),
        .password = std::move(*options.password),
        .display_name = std::move(options.display_name),
        .proxy = std::move(options.proxy),
        .timeout = options.timeout,
        .poll_interval = options.poll_interval,
    };
}

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const auto options = parse_arguments(argc, argv);
        if(options.show_help)
        {
            print_help(std::cout);
            return 0;
        }

        meganz_account_generator::AccountGenerator generator(make_config(options));
        const auto account = generator.generate();

        std::cout
            << "Created MEGA account\n"
            << "Email: " << account.email << '\n'
            << "Display name: " << account.display_name << '\n';
        return 0;
    }
    catch(const std::invalid_argument& error)
    {
        std::cerr << "Argument error: " << error.what() << '\n';
        std::cerr << "Use --help for usage.\n";
        return 2;
    }
    catch(const std::exception& error)
    {
        std::cerr << "Account generation failed: " << error.what() << '\n';
        return 1;
    }
}
