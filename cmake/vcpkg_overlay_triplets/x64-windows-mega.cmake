set(VCPKG_TARGET_ARCHITECTURE x64)

# Use the default vcpkg Visual Studio and toolset discovery on Windows.
# This triplet only preserves the MEGA SDK linkage choices.
set(VCPKG_CRT_LINKAGE dynamic)

if(PORT MATCHES "ffmpeg")
    # Build ffmpeg as DLL due to its LGPL packaging requirements.
    set(VCPKG_LIBRARY_LINKAGE dynamic)
else()
    # Prefer static libraries for simpler SDK consumption.
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
