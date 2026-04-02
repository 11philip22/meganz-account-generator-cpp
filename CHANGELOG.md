# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project intends to follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added public C++ headers under `include/meganz_account_generator/` with a stable synchronous
  `AccountGenerator` API.
- Added a `meganz_account_generator_cpp` library target and `meganz_account_generator_cpp_cli`
  executable.
- Added CLI help and public API usage validation alongside the existing deterministic test suite.
- Added the initial CMake-based project skeleton for the C++ implementation.
- Added explicit dependency wiring for `meganz/sdk` via `SDK_ROOT`.
- Added explicit dependency wiring for `guerrillamail-client-c` via either:
  - `GUERRILLAMAIL_CLIENT_C_ROOT`, or
  - `GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR` plus `GUERRILLAMAIL_CLIENT_C_LIBRARY`.
- Added a local CMake helper for validating and configuring GuerrillaMail dependency inputs.
- Added a smoke target, `pass1_smoke`, that links the MEGA SDK and GuerrillaMail client without performing network activity.
- Added the initial repository layout under `include/`, `src/`, `tests/`, and `cmake/`.
- Added the internal wrapper library target used by the smoke target and tests.
- Added an idiomatic C++ GuerrillaMail wrapper with:
  - RAII client ownership
  - C-to-C++ message/detail value conversion
  - centralized GuerrillaMail status and error translation
- Added a synchronous MEGA request bridge built on `MegaRequestListener`.
- Added an internal MEGA API facade for proxy configuration, login, account creation, signup-link queries, and account confirmation.
- Added deterministic local tests for:
  - request waiter timeout and completion behavior
  - GuerrillaMail wrapper conversion and status mapping
  - header self-sufficiency
  - deprecated confirmation overload removal
- Added a high-level `core::AccountGenerator` workflow for Pass 3 with:
  - random GuerrillaMail alias generation
  - MEGA signup creation and resume handling
  - inbox polling with confirmation-link extraction
  - MEGA account confirmation
  - best-effort temporary inbox cleanup
- Added deterministic helper tests for:
  - confirmation-link extraction
  - likely-MEGA mail heuristics
  - display-name splitting
  - alias shape validation
- Added an opt-in `account_generator_e2e_test` CTest target that exercises the library flow
  end-to-end when E2E environment variables are configured.

### Changed

- Documented the supported local build inputs and verified build commands in the README.
- Clarified that `cargo` is required and must be available on `PATH` when using `GUERRILLAMAIL_CLIENT_C_ROOT`.
- Documented the public library usage path and thin CLI usage path in the README.
- Narrowed the public account-generator config so it no longer exposes the internal MEGA SDK
  client-type knob.
- Stopped the CLI from echoing the user-supplied password on success.
- Updated the smoke target to validate the internal wrapper layer instead of touching raw dependency APIs directly.
- Updated the opt-in end-to-end harness to exercise the public library API instead of the
  internal `core::AccountGenerator`.
- Removed the deprecated password-based account confirmation overload from the MEGA wrapper.
- Removed GuerrillaMail C ABI leakage from the main C++ wrapper header.
- Updated the README with exact Pass 3 end-to-end verification instructions and a clear note about
  what was and was not verified locally.

### Fixed

- Fixed the MEGA request waiter timeout path so the listener stays alive until the SDK delivers `onRequestFinish`.
- Fixed exception-safety holes when adopting raw `gm_client_t*` handles into the C++ wrapper.
- Fixed undefined-behavior risk in the mail wrapper detail test by removing `const_cast` on string literals.
- Fixed Pass 3 confirmation validation so account generation rejects missing or mismatched
  confirmed-email results from the MEGA SDK.
- Fixed Pass 3 config normalization so empty optional proxy, base-path, and user-agent strings
  are treated consistently as unset values before wrapper construction.
- Fixed the opt-in end-to-end test harness to create an exclusive temporary SDK base path instead
  of potentially reusing an existing directory.
- Fixed the public API usage test so it now links a real out-of-line public-library symbol instead
  of only exercising header-only declarations.
- Fixed the README public API example so it compiles as written without requiring an undeclared
  `std::move`.
