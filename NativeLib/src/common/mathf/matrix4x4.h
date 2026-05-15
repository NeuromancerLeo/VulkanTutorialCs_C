#pragma once

/// @brief 代表 4 x 4 矩阵的数据结构，大小为 64 字节.
typedef struct Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} Matrix4x4;