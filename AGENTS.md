# AGENTS.md

## Project Intent

This repository is a C++ account generator for MEGA.

The implementation should be idiomatic modern C++, not a transliteration of the Rust project. The Rust repository is useful for understanding the product behavior, but it is not the API, design, or style target for this codebase.

Project-wide architecture rules and coding rules live in this file. Issue descriptions may reference this file instead of duplicating those rules.

Primary dependencies:

- MEGA SDK: `https://github.com/meganz/sdk`
- GuerrillaMail C client: `https://github.com/11philip22/guerrillamail-client-c`

Companion references:

- Rust behavior reference: `https://github.com/11philip22/meganz-account-generator`
- SDK usage example: `https://github.com/11philip22/mega-sdk-sequence`

## Dependency Policy

Do not assume dependency locations, install methods, or preconfigured environments.

That means:

- do not assume `meganz/sdk` lives in a fixed sibling directory
- do not assume `guerrillamail-client-c` is installed system-wide
- do not assume Homebrew, vcpkg, apt, or any other package manager is present unless the user explicitly says so
- do not assume optional SDK transitive dependencies are already resolved
- do not assume a prebuilt library exists when a source build may be required

Allowed dependency handling:

- require explicit dependency paths through CMake cache variables
- validate those paths during configure time
- fail fast with concrete diagnostics when required inputs are missing
- document only the workflows that are actually supported by the repository

Not allowed:

- hidden path guesses
- undocumented fallback magic
- writing docs that describe hypothetical setups that were never encoded in the build

If the project later adds bootstrap scripts, those scripts become the supported setup. Until then, explicit configuration is preferred over guessing.

## What To Preserve

Preserve the end-to-end behavior, not the Rust API shape.

The core flow is:

1. Create a temporary GuerrillaMail inbox.
2. Start a MEGA signup through the official SDK.
3. Poll the inbox for the MEGA confirmation email.
4. Extract the confirmation link or confirmation key.
5. Confirm the signup through the SDK.
6. Return confirmed credentials.
7. Attempt inbox cleanup as a best-effort final step.

## What Not To Preserve

Do not preserve any of the following just because the Rust version does it:

- crate-style builder APIs
- Rust naming or module structure
- Rust error taxonomy
- async-first design
- CLI flag layout
- Rust-specific abstractions or control flow

If a cleaner C++ design differs from the Rust project, prefer the cleaner C++ design.

## C++ Design Principles

This project should look like a well-structured C++ library with a small CLI on top.

Prefer:

- RAII for all owned resources
- value types for returned data
- small focused classes
- clear ownership boundaries
- exceptions or a single consistent result type, but not a mixed ad hoc error style
- `std::string`, `std::string_view`, `std::optional`, `std::chrono`, and standard library facilities over custom utility code
- strong encapsulation around C and callback-heavy APIs
- deterministic cleanup
- minimal shared mutable state
- explicit timeouts
- testable units with narrow responsibilities

Avoid:

- porting Rust abstractions directly
- exposing C API details in public interfaces
- leaking raw SDK listener machinery into high-level business logic
- overengineering generic frameworks before the basic flow works
- singleton-heavy design
- manual memory management in application logic

## Style Direction

Write modern, readable C++.

Guidelines:

- Favor composition over inheritance unless SDK listeners require inheritance.
- Use `std::unique_ptr` where ownership is singular.
- Use raw pointers only for non-owning views or where the SDK requires them.
- Keep interfaces const-correct.
- Prefer free functions for pure parsing/helpers and classes for stateful orchestration.
- Keep headers small and stable.
- Minimize macro usage.
- Prefer narrow public APIs and richer internal helpers.
- Keep error messages actionable.
- Use descriptive names instead of clever ones.

Target standard:

- Prefer modern C++20 style when it improves clarity.
- Do not fight the SDK or toolchain for the sake of newer language features.
- If SDK integration is cleaner with C++17-compatible code, prefer compatibility over novelty.

## C++ House Style

Use one consistent style across the repository.

### Naming

- namespaces: lowercase
- types and enums: `PascalCase`
- functions, methods, variables, and files: `snake_case`
- private data members: trailing underscore, for example `client_`
- constants: `kPascalCase`
- enum values: `PascalCase`

Examples:

- `namespace mega_client`
- `class AccountGenerator`
- `struct GeneratedAccount`
- `std::string extract_confirmation_key(...)`
- `MailClient client_`
- `constexpr auto kDefaultTimeout = std::chrono::minutes{5};`

### Formatting

- use 4 spaces for indentation
- keep lines reasonably bounded; optimize for readability over density
- use braces consistently; do not rely on single-line implicit blocks
- prefer one declaration per line
- prefer early returns over deep nesting
- keep functions short when practical, but do not split logic into meaningless fragments

If a formatter is added later, follow the formatter. Until then, preserve the existing style in touched files and move new files toward this style.

### Includes

- every header must be self-sufficient
- include the matching project header first in each `.cpp` file
- then standard library headers
- then third-party headers
- then project headers, if any remain
- prefer forward declarations in headers when they reduce coupling cleanly
- do not forward declare types you immediately need by value

### Interfaces

- prefer return values over out-parameters
- use `std::string_view` for non-owning string inputs when lifetime is clear
- use `std::span` for non-owning ranges when available and helpful
- use `std::optional` for optional values
- use `enum class` instead of unscoped enums
- prefer small explicit configuration structs over long parameter lists
- mark functions `const` when they do not mutate observable state
- mark move-only types explicitly if copying is not valid

### Ownership And Lifetime

- use RAII everywhere
- avoid naked `new` and `delete` outside dependency interop boundaries
- wrap raw C and SDK resources immediately after acquisition
- use `std::unique_ptr` for unique ownership
- use `std::shared_ptr` only when shared ownership is actually required
- raw pointers are non-owning by default
- references are preferred when null is not a valid state

### Error Handling

- use one project-wide error strategy consistently
- prefer exceptions over ad hoc mixtures of status codes, null returns, and out-parameters unless the project explicitly standardizes on a `Result` type
- preserve upstream error context when translating dependency failures
- make timeout, parse, mail, and MEGA failures distinguishable
- do not swallow failures silently unless the operation is explicitly best-effort cleanup

### Standard Library Preference

Prefer standard library facilities before custom helpers.

Examples:

- `std::chrono` instead of raw integer timeouts
- `std::filesystem` instead of manual path string handling
- `std::array` or `std::vector` instead of manual buffers where appropriate
- `<algorithm>` and ranges-based code where they improve clarity
- `std::regex` only when the parsing problem is genuinely regex-shaped

### Comments

- comment the why, not the obvious what
- keep comments short and technical
- do not narrate trivial code
- add comments at dependency boundaries where behavior is non-obvious
- remove stale comments when code changes

### Testing Style

- test behavior at clear seams
- prefer deterministic unit tests over fragile end-to-end automation
- keep external-service integration behind boundaries that can be faked
- test parsing, timeouts, and wrapper conversions directly
- avoid tests that duplicate implementation structure too closely

## Architecture Direction

Preferred layering:

1. Thin RAII wrapper around `guerrillamail-client-c`
2. Thin synchronous facade over MEGA SDK request/listener mechanics
3. High-level signup orchestration layer
4. Small CLI frontend

Possible module breakdown:

- `mail/`
  - GuerrillaMail client wrapper
  - inbox/message value types
- `mega/`
  - request waiter
  - proxy/session helpers
  - signup operations
- `core/`
  - account generator
  - confirmation-link extraction
  - timeouts/retry policy
- `cli/`
  - argument parsing
  - output formatting

Exact names are flexible. Separation of responsibilities is more important than matching this layout literally.

## Repository Architecture Rules

Keep the repository split by responsibility.

Recommended structure:

- `include/`
  - public headers only
- `src/`
  - implementation files only
- `src/mail/`
  - GuerrillaMail wrapper layer
- `src/mega/`
  - MEGA SDK integration layer
- `src/core/`
  - high-level signup orchestration
- `src/cli/`
  - command-line frontend
- `tests/`
  - focused unit and seam-level tests

Rules:

- public headers must not expose `gm_*` C structs
- public headers must not expose `MegaRequestListener` plumbing
- C ABI details belong in the mail integration layer only
- MEGA SDK callback/listener details belong in the MEGA integration layer only
- orchestration code may depend on mail and MEGA wrappers, but wrappers must not depend on orchestration code
- CLI code may depend on the public library API, but library code must not depend on the CLI

Keep dependencies pointing inward. High-level code should not reach around lower-level interfaces to call raw dependency APIs directly.

## GuerrillaMail Guidance

Reference repo inspected:

- `https://github.com/11philip22/guerrillamail-client-c`

Relevant header:

- `include/guerrillamail_client.h`

Important facts:

- The API is blocking.
- It exposes explicit allocation/free functions.
- It provides a last-error string accessor.

Preferred handling:

- Hide the C ABI behind a C++ wrapper immediately.
- Convert the C structs into normal C++ value types.
- Centralize error translation in one place.
- Ensure all allocated C resources are released via RAII wrappers.

Public code outside the wrapper should not manipulate `gm_*` structs directly.

## MEGA SDK Guidance

Reference repo inspected:

- `https://github.com/meganz/sdk`

Relevant files inspected:

- `README.md`
- `examples/simple_client/README.md`
- `examples/simple_client/simple_client.cpp`
- `tests/integration/SdkTestCreateAccount_test.cpp`

Important facts:

- The public API entry point is `include/megaapi.h`.
- The SDK is callback/listener based.
- Account creation in the SDK involves explicit signup operations and confirmation handling.

Preferred handling:

- Build a small synchronous request bridge on top of the SDK.
- Isolate `MegaRequestListener` usage in a dedicated helper.
- Expose straightforward C++ methods for the operations the app actually needs.
- Keep high-level signup logic free from callback bookkeeping.

The SDK integration layer should be the only place where listener-waiting mechanics are visible.

## mega-sdk-sequence Guidance

Reference repo inspected:

- `https://github.com/11philip22/mega-sdk-sequence`

Relevant files inspected:

- `readme.md`
- `CMakeLists.txt`
- `src/login_only.cpp`

This repo is useful because it shows a practical way to consume the MEGA SDK from a standalone C++ project.

Patterns worth reusing:

- add the SDK as a CMake subdirectory
- link against `MEGA::SDKlib`
- bridge async SDK requests to synchronous code with a small waiter
- configure SDK proxy settings explicitly

These are infrastructure references, not style constraints.

## Behavior Reference From Rust

Reference repo inspected:

- `https://github.com/11philip22/meganz-account-generator`

Relevant files inspected:

- `README.md`
- `Cargo.toml`
- `src/lib.rs`
- `src/generator.rs`

Use this repo only to understand intended behavior:

- temporary GuerrillaMail address creation
- polling for confirmation mail
- confirmation-link extraction
- timeout behavior
- best-effort inbox deletion

Do not copy its public API or architecture.

## Error Handling

Pick one coherent strategy and use it consistently.

Preferred order:

1. Exceptions internally and at the library boundary if the project stays small and synchronous.
2. A focused `Result<T, Error>`-style abstraction if there is a strong reason to avoid exceptions.

Do not mix exceptions, status codes, nullable returns, and out-parameters arbitrarily.

Whatever strategy is chosen:

- preserve original error context where possible
- include upstream SDK or GuerrillaMail error text
- distinguish timeout, parse failure, mail failure, and MEGA failure in a meaningful way

## Public API Expectations

The public C++ API should feel natural for C++ users.

Reasonable direction:

- one main generator class or service object
- a small configuration type
- a returned value type for created account details
- simple synchronous entry points

The exact API should be chosen for clarity, not parity with another language.

## CLI Expectations

The CLI is secondary to the library design.

Priorities:

1. library flow is correct
2. CLI is thin and unsurprising

The CLI should not dictate awkward library design decisions.

## Build Guidance

Use CMake.

Likely direction based on inspected references:

- local project CMake at the root
- MEGA SDK added via `-DSDK_ROOT=...`
- SDK pulled in with `add_subdirectory`
- target linked to `MEGA::SDKlib`

Keep build files straightforward. Favor a build that is easy to understand and easy to reproduce locally over a highly abstract CMake setup.

## Testing Guidance

Test the pieces that are under local control.

High-value tests:

- confirmation-link extraction
- timeout behavior around polling logic
- translation of GuerrillaMail responses into C++ value types
- request-waiting helpers where practical

Where external services make testing awkward, isolate the integration boundary so behavior can still be tested with fakes or small adapters.

## Practical Instructions For Future Agents

- Use the MEGA SDK instead of reverse-engineering MEGA web requests.
- Use `guerrillamail-client-c` instead of writing a new GuerrillaMail client.
- Treat the Rust project as a behavior reference only.
- Prefer modern C++ design over cross-language parity.
- Keep C APIs and SDK callback machinery behind narrow wrappers.
- Optimize for clarity, ownership safety, and maintainability.
