#pragma once

#include <math.h>
#include <string.h>

struct Vector4 {
    float x, y, z, w;
    Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
};


struct Matrix4x4 {
    float m[16];
    Matrix4x4() {
        m[0] = 1; m[4] = 0; m[8] = 0; m[12] = 0;
        m[1] = 0; m[5] = 1; m[9] = 0; m[13] = 0;
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
    }
    
    Matrix4x4(const float* arr) {
        for (int i = 0; i < 16; ++i) m[i] = arr[i];
    }
    
    Vector4 operator*(const Vector4& v) const {
        Vector4 res;
        res.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
        res.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
        res.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
        res.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
        return res;
    }
    
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[j * 4 + i] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[j * 4 + i] += m[k * 4 + i] * other.m[j * 4 + k];
                }
            }
        }
        return result;
    }
};