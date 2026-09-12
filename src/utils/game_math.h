#pragma once

#include <cmath>

#include "src/utils/log.h"
#include "src/utils/arena_allocator.h"


typedef union Vec2
{
    struct
    {
        float x, y;
    };
    float elements[2];
} Vec2;

typedef union Vec3
{
    struct
    {
        float x, y, z;
    };
    float elements[3];
} Vec3;

typedef union Vec4
{
    struct
    {
        float x, y, z, w;
    };
    float elements[4];

    inline float& operator[](int Index) { return elements[Index]; }
    inline const float& operator[](int Index) const { return elements[Index]; }
} Vec4;

typedef union Vec2i
{
    struct
    {
        int x, y;
    };

    int elements[2];
} Vec2i;


// Column Major 4x4 Matrix
typedef union Mat4
{
    Vec4 columns[4];
    float elements[4][4];

    inline Vec4& operator[](int index) { return columns[index]; }
    inline const Vec4& operator[](int index) const { return columns[index]; }
} Mat4;


/*
    Mat4 x Vec4 operator overload
*/
inline Vec4 operator*(Mat4 m, Vec4 v)
{
    Vec4 result = {};

    // Vec3[row]
    for (int row = 0; row < 4; row++)
    {
        // multiplication
        for (int x = 0; x < 4; x++)
        {
            result[row] += v[x] * m[x][row];
        }
    }

    return result;
}


/*
    TODO(harsh): write the Normalize function and overload it with Vec2, Vec3 and Vec4
    not to be used directly, call Noramlize() instead
*/
inline Vec2 normalize_vec2(Vec2 vec)
{
    // pythagoras thorem
    float length = sqrt(pow(vec.x, 2) + pow(vec.y, 2));
    Vec2 normalized = {};
    normalized.x = vec.x / length;
    normalized.y = vec.y / length;

    return normalized;
}

/*
Returns a 4x4 identity matrix

visualized:
[ 1  0  0  0 ]
[ 0  1  0  0 ]
[ 0  0  1  0 ]
[ 0  0  0  1 ]
*/
inline Mat4 Identity_Mat4()
{
    Mat4 m = {};
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;

    return m;
}

/*
    Multiplies two Column Major Mat4, Read right -> Left in order *NON-COMMUTATIVE*
*/
inline Mat4 Mat4xMat4(Mat4 left, Mat4 right)
{
    Mat4 result = {};

    // result[column]
    for (int col = 0; col < 4; col++)
    {
        // result[column][row]
        for (int row = 0; row < 4; row++)
        {
            // multiplication
            for (int x = 0; x < 4; x++)
            {
                result[col][row] += right[col][x] * left[x][row];
            }
        }
    }


    return result;
}

/*
    Returns a 4x4 scale matrix that scales a Vec3 by provided vec3

    visualized:
    [ S1  0   0   0 ]
    [ 0   S2  0   0 ]
    [ 0   0   S3  0 ]
    [ 0   0   0   1 ]
*/
inline Mat4 Scale_Mat4(Vec3 scale)
{
    Mat4 m = Identity_Mat4();
    m[0].x = scale.x;
    m[1].y = scale.y;
    m[2].z = scale.z;

    return m;
}

/*
    Returns a 4x4 matrix that translates a Vec3 by provided Vec3

    visualized:
    [ 1  0  0  Tx ],
    [ 0  1  0  Ty ],
    [ 0  0  1  Tz ],
    [ 0  0  0   1 ],
*/
inline Mat4 Translate_Mat4(Vec3 translation)
{
    Mat4 m = Identity_Mat4();
    m[3].x += translation.x;
    m[3].y += translation.y;
    m[3].z += translation.z;

    return m;
}

/*
    NOTE(harsh): THIS FOLLOWS Y+ "Down" CONVENTION

    Retuns a 4x4 right handed orthographic projection matrix with Z ranging from 0 to 1 (DirectX/vulkan convention)
    Left, Right, Bottom, Top specify the coordinate of there respective clipping space

    z* = (near) / (near - far)
    visualized:
    [ 2.0/width     0           0               -1 ],
    [   0       2.0/height      0               -1 ],
    [   0           0       1.0/(near - far)    z* ],
    [   0           0           0                1 ],
*/
inline Mat4 Orthograhpic_RH_ZO_Mat4(
    float left,
    float right,
    float bottom,
    float top,
    float near_plane,
    float far_plane)
{
    Mat4 m = Identity_Mat4();

    m[0][0] = 2.0f / (right - left);
    m[1][1] = 2.0f / (top - bottom);
    m[2][2] = 2.0f / (near_plane - far_plane);

    m[3][0] = (left + right) / (left - right);
    m[3][1] = (bottom + top) / (bottom - top);
    m[3][2] = (near_plane) / (near_plane - far_plane);

    return m;
}

/*
    Returns a View Matrix
*/
inline Mat4 ViewMat4(Vec2 position, Vec2 offset, float zoom)
{
    Mat4 view_matrix = Translate_Mat4({offset.x, offset.y, 0.0f});

    // TODO(harsh): figure out the correct order for scaling/zooming
    // view_matrix = Mat4xMat4(view_matrix, Scale_Mat4({zoom, zoom, 1.0f}));

    view_matrix = Mat4xMat4(view_matrix, Translate_Mat4({-position.x, -position.y, 0.0f}));

    return view_matrix;
}
