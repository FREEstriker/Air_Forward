#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#define START_SET 7

layout(set = 0, binding = 0) uniform MatrixData{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 itModel;
} matrixData;

vec4 ObjectToProjection(vec4 position)
{
    return matrixData.projection * matrixData.view * matrixData.model * position;
}

vec4 ObjectToWorld(vec4 position)
{
    return matrixData.model * position;
}

vec3 DirectionObjectToWorld(vec3 direction)
{
    return mat3(matrixData.itModel) * direction;
}
#endif