#include "meganz_account_generator/account_generator.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "cli/random_app_key.hpp"

namespace
{

constexpr std::string_view kDefaultDisplayName = "Automation Bot";

struct CliOptions
{
    std::optional<std::string> password;
    std::string display_name{std::string(kDefaultDisplayName)};
    std::optional<std::string> proxy;
    std::chrono::milliseconds timeout{std::chrono::minutes{5}};
    std::chrono::milliseconds poll_interval{std::chrono::seconds{5}};
    bool show_help{false};
};

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
        << "Runtime behavior:\n"
        << "  The CLI generates a fresh random MEGA app key each time it starts.\n\n"
        << "Options:\n"
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

    for(int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];

        if(argument == "--help" || argument == "-h")
        {
            options.show_help = true;
            return options;
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
        .app_key = cli::generate_random_app_key(),
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
