#!/usr/bin/env bash

printf 'Starting Build...\n'
args="$1"

# ================== Colors ==================
PASTEL_RED=$'\e[38;2;220;120;120m'
PASTEL_GREEN=$'\e[38;2;120;200;140m'
RESET=$'\e[0m'

# ========== OUTPUT DIR & EXE SETUP ==========
out_dir_debug_relative="build/debug"
out_dir_release_relative="build/release"

if [[ "$args" == "debug" ]]; then
    out_dir_relative="$out_dir_debug_relative"
elif [[ "$args" == "release" ]]; then
    out_dir_relative="$out_dir_release_relative"
else
    printf 'No build specification argument pass. Try build_win32.sh debug\n'
    exit 1
fi
pch_path="$out_dir_relative/pch.pch"
out_exe_path="$out_dir_relative/win32_d3d11.exe"
[[ -d "$out_dir_relative" ]] || mkdir -p "$out_dir_relative"


debug_build()
{
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
    flags=(
        -g
        # -ftime-trace
    )

    # =================== PCH ====================
    # -nt checks if pch.h is newer than $pch_path if so then only rebuild pch
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
        "${defines[@]}"                                         \
        "${libs[@]}"                                            \
        "${warnings[@]}"                                        \
        "${flags[@]}"                                           \
        -include-pch "$pch_path"
}

release_build()
{
    # =========== BUILD CONFIG ============
    defines=(
        -D_CRT_SECURE_NO_WARNINGS
        -DNDEBUG
    )
    libs=(
        -luser32
        -ld3d11
        -ld3dcompiler
    )
    warnings=(
        -Wno-format-security
    )
    flags=(
        -O3
        -flto
        -fuse-ld=lld
    )

    # ============== MAIN COMPILATION ============
    clang++                                                     \
        -std=c++20 -I. src/main.cpp -o "$out_exe_path"          \
        "${defines[@]}"                                         \
        "${libs[@]}"                                            \
        "${warnings[@]}"                                        \
        "${flags[@]}"
}



# Start Time
start_us=${EPOCHREALTIME/./}

if [[ "$args" == "debug" ]]; then
    debug_build
elif [[ "$args" == "release" ]]; then
    release_build
fi

# Build Command Status
build_status=$?

# End Time
end_us=${EPOCHREALTIME/./}

# ============== PRINT RESULTS ==============
if ((build_status != 0)); then
    printf 'Compilation %sfailed%s\n' "$PASTEL_RED" "$RESET"
    exit "$build_status"
fi

printf 'Compilation %ssuccessful%s\n' "$PASTEL_GREEN" "$RESET"

build_time_us=$((end_us - start_us))
build_time_ms=$((build_time_us / 1000))
printf 'Build time: %d ms\n' "$build_time_ms"
