#pragma once

#include "../../../../common/mathf/mathf.h"

typedef struct CameraUniform {
    Matrix4x4 viewProjcetionMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
} CameraUniform;