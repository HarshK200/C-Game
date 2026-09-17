#pragma once

// third party header files (Cross paltform)
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#include "third_party/fast_noise_lite.h"

// std libs
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <cmath>
#include <stdio.h>
#include <iterator>
#include <assert.h>

// windows header files
#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#endif
