# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project intends to follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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

### Changed

- Documented the supported local build inputs and verified build commands in the README.
- Clarified that `cargo` is required and must be available on `PATH` when using `GUERRILLAMAIL_CLIENT_C_ROOT`.
- Updated the smoke target to validate the internal wrapper layer instead of touching raw dependency APIs directly.
- Removed the deprecated password-based account confirmation overload from the MEGA wrapper.
- Removed GuerrillaMail C ABI leakage from the main C++ wrapper header.

### Fixed

- Fixed the MEGA request waiter timeout path so the listener stays alive until the SDK delivers `onRequestFinish`.
- Fixed exception-safety holes when adopting raw `gm_client_t*` handles into the C++ wrapper.
- Fixed undefined-behavior risk in the mail wrapper detail test by removing `const_cast` on string literals.
