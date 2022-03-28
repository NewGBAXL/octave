#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	GeometryUniforms geometry;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec2 outTexcoord;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = geometry.mLightWVP * vec4(inPosition, 1.0);
    outTexcoord = inTexcoord;
}