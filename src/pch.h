#pragma once

#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winuser.h>
#include <winnt.h>
#include <d3d11.h>
#include <dxgiformat.h>
#include <d3dcompiler.h>
#endif

#include <cstdint>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iterator>
#include <assert.h>
#include <sys/stat.h>


// thirdparty
#define FNL_IMPL
#include "third_party/fast_noise_lite.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
