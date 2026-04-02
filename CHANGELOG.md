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

### Changed

- Documented the supported local build inputs and verified build commands in the README.
- Clarified that `cargo` is required and must be available on `PATH` when using `GUERRILLAMAIL_CLIENT_C_ROOT`.
