#version 450

layout(location = 0) in vec3 in_vertPosition;
layout(location = 1) in vec3 in_vertColor;

layout(set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 viewProjectionMatrix;  /* 视图-投影矩阵  */
    mat4 viewMatrix;            /* 视图矩阵，备用 */
    mat4 projectionMatrix;      /* 投影矩阵，备用 */
} uCamera;

layout(set = 1, binding = 0) uniform DrawItemUniformBuffer {
    mat4 modelMatrix;           /* 模型矩阵 */
} uDrawItem;

layout(location = 0) out vec3 out_fragColor;


void main()
{
    gl_Position =
        vec4(in_vertPosition, 1.0)
        * uDrawItem.modelMatrix
        * uCamera.viewProjectionMatrix;

    out_fragColor = in_vertColor;
}