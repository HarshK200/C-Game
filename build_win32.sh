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
pch_path="$out_dir_relative/pch.pch"
[[ -d "$out_dir_relative" ]] || mkdir -p "$out_dir_relative"


# =========== BUILD CONFIG ============
defines=(
    -DISEKAIED_DEBUG
    -D_CRT_SECURE_NO_WARNINGS
)

libs=(
    -luser32
    -ld3d11
    -ld3dcompiler
)

warnings=(
    -Wno-format-security
)

extra_flags=(
    # -ftime-trace
)

# Start Time
start_us=${EPOCHREALTIME/./}


# =================== PCH ====================
# -nt checks if pch.h last modified != $pch_path last modified then only build
if [[ ! -f "$pch_path" || "src/pch.h" -nt "$pch_path" ]]; then
    printf 'Building PCH...\n'
clang++ \
    -std=c++20 \
    -I. \
    "${defines[@]}" \
    "${warnings[@]}" \
    -x c++-header \
    src/pch.h \
    -o "$pch_path"
fi


# ============== MAIN COMPILATION ============
clang++                                                     \
    -std=c++20 -I. src/main.cpp -o "$out_exe_path"          \
    -g                                                      \
    "${defines[@]}"                                         \
    "${libs[@]}"                                            \
    "${warnings[@]}"                                        \
    "${extra_flags[@]}"                                     \
    -include-pch "$pch_path"

# End Time
end_us=${EPOCHREALTIME/./}

# Build Command Status
build_status=$?


# ============== PRINT RESULTS ==============
if ((build_status != 0)) then
    printf 'Compilation %sfailed%s\n' "$PASTEL_RED" "$RESET"
    exit "$build_status"
fi

printf 'Compilation %ssuccessful%s\n' "$PASTEL_GREEN" "$RESET"

build_time_us=$((end_us - start_us))
build_time_ms=$((build_time_us / 1000))
printf 'Build time: %d ms\n' "$build_time_ms"
