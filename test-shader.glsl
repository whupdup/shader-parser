
#include "test-header.glh"

#if defined(VS_BUILD)

layout (local_size_x = 1, local_size_y = 16) in;

layout (location = 0) in vec3 position;
layout (binding = 0, rgba32f) readonly writeonly uniform image2D myImage;

layout (std140, binding = 0) uniform TestUBO {
    vec3 testV3;
    mat4 matrices[32];
    float myFloat;
    ivec3 something;
};

layout (std430) buffer TestSSBO {
    vec4 testV4;
    vec2 testV2Ary[];
} bufferVar;

void main() {

}

#elif defined(FS_BUILD)

layout (location = 0) out vec4 outColor;

void main() {

}

#endif