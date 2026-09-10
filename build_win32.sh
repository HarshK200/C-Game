#!/usr/bin/env bash


# TODO(harsh): pass a -debug / -release flag to this build script,
# and enable debugs flags and etc... only on debug build
# Enable optimizations, no asserts, no bound checks, etc... on release build


printf 'Starting Build...\n'

# ================== Colors ==================
PASTEL_RED=$'\e[38;2;220;120;120m'
PASTEL_GREEN=$'\e[38;2;120;200;140m'
RESET=$'\e[0m'

# ========== OUTPUT DIR & EXE SETUP ==========
out_dir_relative="build/debug"
out_exe_path="$out_dir_relative/win32_d3d11.exe"
[[ -d "$out_dir_relative" ]] || mkdir -p "$out_dir_relative"


# Start Time
start_us=${EPOCHREALTIME/./}


# =========== NINJA BUILD COMMAND ============
ninja


# =========== MANUAL BUILD COMMAND ============

# # BUILD CONSTANTS
# defines=(
#     -DISEKAIED_DEBUG
#     -D_CRT_SECURE_NO_WARNINGS
# )
#
# libs=(
#     -luser32
#     -ld3d11
#     -ld3dcompiler
# )
#
# warnings=(
#     -Wno-format-security
# )
#
#
# # RUN BUILD COMMAND
# clang++                                                     \
#     -std=c++20 -I. src/main.cpp -o "$out_exe_path"          \
#     -g                                                      \
#     "${defines[@]}"                                         \
#     "${libs[@]}"                                            \
#     "${warnings[@]}"    # -ftime-trace


# =============================================


# Build Command Status
build_status=$?

# End Time
end_us=${EPOCHREALTIME/./}


# ============== PRINT RESULTS ==============
build_time_us=$((end_us - start_us))
build_time_ms=$((build_time_us / 1000))


if ((build_status != 0)) then
    printf 'Compilation %sfailed%s\n' "$PASTEL_RED" "$RESET"
    printf 'Build time: %d ms\n' "$build_time_ms"
    exit "$build_status"
fi

printf 'Compilation %ssuccessful%s\n' "$PASTEL_GREEN" "$RESET"
printf 'Build time: %d ms\n' "$build_time_ms"
