# meganz-account-generator-cpp

C++ implementation of a MEGA account generator built around the official MEGA SDK and the `guerrillamail-client-c` C ABI.

This repository currently contains the Pass 1 build skeleton:

- root CMake project
- explicit dependency wiring
- smoke target proving the link step works

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

## Source Layout

The initial layout is intentionally simple:

- `cmake/`
  - local CMake helpers
- `include/`
  - future public headers
- `src/`
  - project sources
- `src/smoke/`
  - Pass 1 smoke target
- `tests/`
  - future tests

Future passes will fill out the mail, mega, core, and cli layers described in `AGENTS.md`.
