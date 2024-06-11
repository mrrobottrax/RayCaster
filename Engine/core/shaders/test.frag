#version 450

// Based on https://www.shadertoy.com/view/4dX3zl

layout(binding = 0) uniform readonly UniformInput {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 invModel;
    mat4 invView;
    uvec2 screenSize;
} uniformInput;

layout(r8ui, binding = 1) uniform readonly uimage3D pixels;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 startPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 proportion = (gl_FragCoord.xy / uniformInput.screenSize - 0.5) * 2;

    vec3 viewSpaceRayDir = vec3(proportion.xy, 1);
    normalize(viewSpaceRayDir);

    // Model space ray slope
    vec3 rayDir = (uniformInput.invView * uniformInput.invModel * vec4(viewSpaceRayDir, 0)).xyz;
    rayDir.y *= -1;

    // Start tracing
    uint blockId;
    bvec3 mask;
    ivec3 gridPos = ivec3(startPos);
    ivec3 rayStep = ivec3(sign(rayDir));
    vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);
    vec3 sideDist = (sign(rayDir) * (vec3(gridPos) - startPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist; 

    for (uint i = 0; i < 64; ++i)
    {          
        // Check voxel
        blockId = imageLoad(pixels, gridPos).r;

        if (blockId > 0)
        {
            break;
        }

        // Step
        mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));
        sideDist += vec3(mask) * deltaDist;
		gridPos += ivec3(vec3(mask)) * rayStep;

        // Check bounds
        if (gridPos.x < 0 || gridPos.y < 0 || gridPos.z < 0 ||
            gridPos.x > 3 || gridPos.y > 3 || gridPos.z > 3)
        {
            outColor = vec4(0, 0, 0, 1);
            return;
        }
    }

    vec3 color = vec3(1, 0, 1);

    if (mask.x)
    {
        color = vec3(0.3);
    }

    if (mask.y)
    {
        color = vec3(0.75);
    }

    if (mask.z)
    {
        color = vec3(0.5);
    }

    outColor = vec4(color, 1);
}