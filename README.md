<h1 align="center">MEGANZ Account Generator C++</h1>

<p align="center">
  <strong>Modern C++20 library and CLI for creating MEGA accounts with the official MEGA SDK and a temporary GuerrillaMail inbox.</strong>
</p>

<p align="center">
  <a href="https://en.cppreference.com/w/cpp/20"><img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++20"></a>
  <a href="https://cmake.org/"><img src="https://img.shields.io/badge/CMake-3.22%2B-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake 3.22+"></a>
  <a href="https://github.com/meganz/sdk"><img src="https://img.shields.io/badge/MEGA-SDK-D9272E?style=for-the-badge" alt="MEGA SDK"></a>
  <a href="https://github.com/11philip22/guerrillamail-client-cpp"><img src="https://img.shields.io/badge/GuerrillaMail-C%2B%2B%20Client-2F855A?style=for-the-badge" alt="GuerrillaMail C++ client"></a>
</p>

<p align="center">
  <a href="#features">Features</a> &middot;
  <a href="#requirements">Requirements</a> &middot;
  <a href="#quick-start">Quick Start</a> &middot;
  <a href="#library-api">Library API</a> &middot;
  <a href="#cli">CLI</a> &middot;
  <a href="#layout">Layout</a>
</p>

---

`meganz-account-generator-cpp` is a small C++ library with a thin command-line frontend. It uses the native GuerrillaMail C++ client and hides the callback-heavy MEGA SDK behind narrow C++ interfaces, then orchestrates the account signup flow synchronously.

The core flow is:

1. Create a temporary GuerrillaMail inbox.
2. Start MEGA signup through the official SDK.
3. Poll the inbox for the confirmation message.
4. Extract the confirmation link or key.
5. Confirm the account through the SDK.
6. Return the generated account details.
7. Attempt inbox cleanup as a best-effort final step.

> [!IMPORTANT]
> This tool creates real external-service accounts. Use it only for permitted testing or automation, and follow the terms and limits of the services involved.

## Features

- Public C++20 API with value-style configuration and results.
- Direct use of `guerrillamail-cpp`.
- Synchronous facade over MEGA SDK request/listener mechanics.
- Explicit timeout, polling, proxy, base path, and user-agent configuration.
- Thin CLI that delegates to the library API.

## Requirements

- CMake 3.22 or newer.
- C++20 compiler for the project library and CLI.
- Initialized submodules for `third_party/meganz-sdk` and `third_party/guerrillamail-cpp`.
- CMake-visible SDK/client dependencies, including ICU, curl, and nlohmann-json.
- Network access to MEGA and GuerrillaMail for real account-generation runs.

The build uses bundled submodules. It does not guess dependency locations outside the repository. If CMake cannot find SDK dependencies such as ICU or Crypto++, pass explicit CMake discovery hints for your local machine, for example through `CMAKE_PREFIX_PATH`, `ICU_ROOT`, or dependency-specific cache variables.

## Quick Start

Initialize submodules:

```bash
git submodule update --init --recursive
```

Configure and build:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build --parallel
```

With multi-config generators such as Visual Studio, pass the configuration at build time:

```bash
cmake --build build --config Debug
```

Useful CMake inputs:

| Name | Default | Purpose |
| --- | --- | --- |
| `MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS` | `OFF` | Treat this project's warnings as errors. |

## Library API

Public headers live in `include/meganz_account_generator/`. Link callers against:

```cmake
target_link_libraries(my_app PRIVATE meganz_account_generator_cpp::library)
```

Minimal usage:

```cpp
#include "meganz_account_generator/account_generator.hpp"

int main()
{
    meganz_account_generator::AccountGeneratorConfig config{
        .app_key = "your-mega-app-key",
        .password = "your-test-password",
        .display_name = "Automation Bot",
    };

    meganz_account_generator::AccountGenerator generator(config);
    const auto account = generator.generate();

    return account.email.empty() ? 1 : 0;
}
```

The public API is synchronous. Failures are reported with `AccountGenerationError` subclasses so callers can distinguish mail failures, MEGA signup failures, confirmation timeouts, and confirmation-link parse failures.

## CLI

The CLI target is `meganz_account_generator_cpp_cli`.

Show help:

```bash
./build/src/meganz_account_generator_cpp_cli --help
```

On Visual Studio style builds, the debug executable is usually:

```powershell
.\build\src\Debug\meganz_account_generator_cpp_cli.exe --help
```

Create an account:

```bash
./build/src/meganz_account_generator_cpp_cli \
  --password "your-test-password" \
  --display-name "Automation Bot"
```

Supported options:

| Option | Required | Description |
| --- | --- | --- |
| `--password <password>` | Yes | Password for the created MEGA account. |
| `--display-name <name>` | No | Account display name. Defaults to `Automation Bot`. |
| `--proxy <url>` | No | Proxy URL used for MEGA and GuerrillaMail requests. |
| `--timeout-ms <milliseconds>` | No | Total time to wait for the confirmation email. |
| `--poll-interval-ms <milliseconds>` | No | Inbox polling interval while waiting for mail. |
| `--help` | No | Print usage text. |

The CLI generates a fresh random MEGA app key for each process. It prints the created email and display name on success, but not the supplied password.

## Layout

```text
cmake/      Local CMake helpers
include/    Public C++ headers
src/public/ Public API translation layer
src/core/   Account-generation orchestration
src/mega/   MEGA SDK facade and request waiter
src/cli/    Command-line frontend
```

## Troubleshooting

- Missing submodule during configure: run `git submodule update --init --recursive`.
- ICU or SDK dependency discovery fails: pass explicit CMake discovery hints for your installed dependencies.
- Live run times out: verify network access, proxy settings, MEGA app key validity, and GuerrillaMail delivery.
