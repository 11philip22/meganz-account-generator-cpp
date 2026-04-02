# meganz-account-generator-cpp

C++ implementation of a MEGA account generator built around the official MEGA SDK and the `guerrillamail-client-c` C ABI.

This repository currently contains:

- explicit MEGA SDK and GuerrillaMail dependency wiring
- internal wrapper layers for GuerrillaMail and MEGA SDK requests
- a public C++ library API
- a thin CLI frontend over that library
- a Pass 3 core account-generation workflow behind the public boundary
- deterministic local tests and an opt-in end-to-end verification harness

## Dependency Inputs

This repository does not guess dependency locations.

The supported inputs are:

- `SDK_ROOT`
  - Required
  - Path to a local `meganz/sdk` checkout
- `GUERRILLAMAIL_CLIENT_C_ROOT`
  - Optional
  - Path to a local `guerrillamail-client-c` checkout
  - When set, CMake will invoke `cargo build` for that repository
  - Requires `cargo` to be installed and available on `PATH`
- `GUERRILLAMAIL_CLIENT_C_PROFILE`
  - Optional
  - `Debug` or `Release`
  - Used only with `GUERRILLAMAIL_CLIENT_C_ROOT`
- `GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR`
  - Optional
  - Include directory containing `guerrillamail_client.h`
- `GUERRILLAMAIL_CLIENT_C_LIBRARY`
  - Optional
  - Path to a prebuilt `guerrillamail-client-c` library file
- `GUERRILLAMAIL_CLIENT_C_IMPLIB`
  - Optional
  - Windows-only import library for DLL-based linking when needed

Choose exactly one GuerrillaMail setup mode:

1. `GUERRILLAMAIL_CLIENT_C_ROOT`
2. `GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR` and `GUERRILLAMAIL_CLIENT_C_LIBRARY`

If neither mode is provided, configure fails with a concrete error.

Additional prerequisite for mode 1:

- `cargo` must be installed and available on `PATH`

## Build Prerequisites

You need:

- CMake 3.20 or newer
- a C compiler with C11 support
- a C++ compiler with C++20 support
- a local `meganz/sdk` checkout passed through `SDK_ROOT`
- one supported GuerrillaMail dependency mode

Depending on how your local `meganz/sdk` checkout is configured, you may also need to provide
explicit discovery hints for upstream dependencies such as ICU and Crypto++.

## Supported Build Shape

The project integrates:

- `meganz/sdk` as a CMake subdirectory via `SDK_ROOT`
- `guerrillamail-client-c` either from source via `cargo build` or from explicit include/library paths

The project disables unused MEGA SDK extras by default:

- `ENABLE_SDKLIB_EXAMPLES=OFF`
- `ENABLE_SDKLIB_TESTS=OFF`
- `ENABLE_SDKLIB_WERROR=OFF`
- `USE_FREEIMAGE=OFF`
- `USE_FFMPEG=OFF`
- `USE_PDFIUM=OFF`
- `USE_READLINE=OFF`

Those defaults can be overridden through normal CMake cache editing if needed later.

## Configure And Build

Example using a local `guerrillamail-client-c` checkout:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSDK_ROOT=/absolute/path/to/sdk \
  -DGUERRILLAMAIL_CLIENT_C_ROOT=/absolute/path/to/guerrillamail-client-c \
  -DGUERRILLAMAIL_CLIENT_C_PROFILE=Debug

cmake --build build -j
```

If your local `meganz/sdk` checkout relies on system package discovery for Crypto++ and ICU, pass those hints explicitly instead of assuming CMake will find them on its own.

Verified on this machine with adjacent SDK checkouts:

```bash
PKG_CONFIG_PATH=/opt/homebrew/opt/cryptopp/lib/pkgconfig \
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSDK_ROOT=../sdk \
  -DGUERRILLAMAIL_CLIENT_C_INCLUDE_DIR=../../guerrillamail-client-c/include \
  -DGUERRILLAMAIL_CLIENT_C_LIBRARY=../../guerrillamail-client-c/target/debug/libguerrillamail_client_c.dylib \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/icu4c@78 \
  -DICU_ROOT=/opt/homebrew/opt/icu4c@78

cmake --build build --target pass1_smoke -j4
```

Example using an already built GuerrillaMail library:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSDK_ROOT=/absolute/path/to/sdk \
  -DGUERRILLAMAIL_CLIENT_C_INCLUDE_DIR=/absolute/path/to/guerrillamail-client-c/include \
  -DGUERRILLAMAIL_CLIENT_C_LIBRARY=/absolute/path/to/libguerrillamail_client_c.dylib

cmake --build build -j
```

## Smoke Target

The current smoke target is:

- `pass1_smoke`

It links both upstream dependencies and exits successfully without performing any network requests.

Run it with:

```bash
./build/src/smoke/pass1_smoke
```

The exact path can differ depending on the generator.

## Architecture

The current implementation is split into these layers:

- `include/meganz_account_generator/`
  - stable public headers for ordinary C++ callers
- `src/public/`
  - translation layer between the public API and internal orchestration
- `src/core/`
  - synchronous account-generation flow, confirmation-link parsing, and retry policy
- `src/mega/`
  - synchronous request bridge over the callback-based MEGA SDK
- `src/mail/`
  - RAII wrapper over the blocking `guerrillamail-client-c` API
- `src/cli/`
  - thin command-line frontend over the public library API
- `tests/`
  - deterministic seam-level tests plus an opt-in live end-to-end harness

## Public C++ API

Public headers live under:

- `include/meganz_account_generator/`

The current public entry point is:

- `meganz_account_generator::AccountGenerator`

It uses:

- `meganz_account_generator::AccountGeneratorConfig`
- `meganz_account_generator::GeneratedAccount`

Link ordinary C++ code against:

- `meganz_account_generator_cpp::library`

Minimal example:

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
}
```

The public API is synchronous. Public headers do not expose the GuerrillaMail C ABI or MEGA
listener plumbing.

## CLI Usage

The CLI target is:

- `meganz_account_generator_cpp_cli`

Show help with:

```bash
./build/src/meganz_account_generator_cpp_cli --help
```

The CLI requires:

- `--password <value>`
- a MEGA app key via `--app-key <value>` or `MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY`

Optional first-run flags:

- `--display-name <value>`
- `--proxy <url>`
- `--timeout-ms <milliseconds>`
- `--poll-interval-ms <milliseconds>`

Example:

```bash
MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY=your-app-key \
./build/src/meganz_account_generator_cpp_cli \
  --password 'your-test-password' \
  --display-name 'Automation Bot'
```

The CLI is a thin frontend over the public library API. It does not expose internal wrapper types
or direct SDK calls. On success it prints the created email and display name, but not the
user-supplied password.

## Runtime Expectations

A real account-generation run requires:

- network access to both MEGA and GuerrillaMail
- a valid MEGA app key
- a password supplied by the caller or CLI user
- enough time for GuerrillaMail delivery and MEGA confirmation processing

If you provide a proxy, the library uses it for both the MEGA SDK and GuerrillaMail wrapper.

## End-to-End Verification

The repository includes an opt-in CTest target, `account_generator_e2e_test`, that exercises
`meganz_account_generator::AccountGenerator` from library code and attempts one real MEGA signup
using a temporary GuerrillaMail inbox.

Required environment variables:

- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_APP_KEY`
- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PASSWORD`

Optional environment variables:

- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_DISPLAY_NAME`
  - default: `Automation Bot`
- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PROXY`
- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_TIMEOUT_MS`
  - default: `300000`
- `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_POLL_INTERVAL_MS`
  - default: `5000`

Build and run it with:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSDK_ROOT=/absolute/path/to/sdk \
  -DGUERRILLAMAIL_CLIENT_C_ROOT=/absolute/path/to/guerrillamail-client-c

cmake --build build -j4

MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_APP_KEY=your-app-key \
MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PASSWORD='your-test-password' \
ctest --test-dir build -R account_generator_e2e_test --output-on-failure
```

If the required environment variables are not set, `account_generator_e2e_test` exits with
code `77` and CTest reports it as skipped.

This harness depends on external MEGA and GuerrillaMail availability. Network issues, service
outages, email delivery delays, and proxy misconfiguration can all cause the live verification
to fail even when the deterministic local suite is green.

## Verified Locally

Verified in this repository:

- `cmake --build build -j4`
- `ctest --test-dir build --output-on-failure`
- `./build/src/smoke/pass1_smoke`
- `./build/src/meganz_account_generator_cpp_cli --help`

Not verified in this pass:

- a live MEGA plus GuerrillaMail end-to-end signup run
- any scenario that requires real network access or externally supplied E2E credentials

## Troubleshooting Local Runs

Common local issues:

- configure fails before generation
  - confirm `SDK_ROOT` points at a real `meganz/sdk` checkout
  - confirm exactly one GuerrillaMail setup mode is configured
- `GUERRILLAMAIL_CLIENT_C_ROOT` mode fails during configure or build
  - make sure `cargo` is installed and available on `PATH`
- MEGA SDK dependency discovery fails
  - pass explicit ICU and Crypto++ discovery hints for your local machine instead of relying on
    implicit system lookup
- `account_generator_e2e_test` is skipped
  - set `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_APP_KEY` and
    `MEGANZ_ACCOUNT_GENERATOR_CPP_E2E_PASSWORD`
- live runs time out while waiting for confirmation mail
  - verify outbound network access
  - verify the MEGA app key is valid
  - verify any configured proxy is reachable and correct
  - expect occasional delivery delays from external services

## Source Layout

The repository is organized by layer:

- `cmake/`
  - local CMake helpers
- `include/`
  - public headers
- `src/`
  - project sources
- `src/core/`
  - high-level signup orchestration
- `src/mail/`
  - GuerrillaMail wrapper layer
- `src/mega/`
  - MEGA SDK integration layer
- `src/cli/`
  - thin command-line frontend
- `src/smoke/`
  - Pass 1 smoke target
- `tests/`
  - unit tests and optional live verification harness

The current implementation follows the mail, mega, core, and cli layering described in
`AGENTS.md`.
