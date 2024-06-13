#version 450

// Based on https://www.shadertoy.com/view/4dX3zl

layout(binding = 0) uniform readonly RendererInput {
    mat4 invView;
    uvec2 screenSize;
    vec3 startPos;
} uInput;

layout(r8ui, binding = 1) uniform readonly uimage3D uBlocks;

layout(location = 0) out vec4 outColor;

const int chunkSize = 16;

struct TraceResult
{
    bool hit;
    uint blockId;
    vec3 normal;
    vec3 position;
    bvec3 mask;
    float dist;
};

TraceResult TraceVoxelRay(vec3 startPos, vec3 rayDir)
{
    vec3 normalDir = normalize(rayDir);

    // Start tracing
    uint blockId;
    bvec3 mask = bvec3(false);
    ivec3 gridPos = ivec3(startPos);
    ivec3 rayStep = ivec3(sign(rayDir));
    vec3 slope = abs(1 / normalDir);
    vec3 sideDist = (sign(rayDir) * (vec3(gridPos) - startPos) + (sign(rayDir) * 0.5) + 0.5) * slope; 
    vec3 oldSideDist = vec3(0);

    bool hit = false;

    for (uint i = 0; i < 2048; ++i)
    {          
        // Check voxel
        blockId = imageLoad(uBlocks, gridPos).r;

        if (blockId > 0)
        {
            hit = true;
            break;
        }

        // Step
        mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));
        oldSideDist = sideDist;
        sideDist += vec3(mask) * slope;
		gridPos += ivec3(mask) * rayStep;

        // Check bounds
        if (gridPos.x < 0 || gridPos.y < 0 || gridPos.z < 0 ||
            gridPos.x >= chunkSize || gridPos.y >= chunkSize || gridPos.z >= chunkSize)
        {
            hit = false;
            break;
        }
    }

    vec3 maskedDist = vec3(mask) * oldSideDist;
    float dist = max(maskedDist.x, max(maskedDist.y, maskedDist.z));

    TraceResult result;
    result.hit = hit;
    result.blockId = blockId;
    result.normal = vec3(mask) * -rayStep;
    result.position = startPos + normalDir * dist;
    result.mask = mask;
    result.dist = dist;

    return result;
}

vec3 GetSurfaceColor(TraceResult trace, vec3 surfacePos)
{
    if (!trace.hit)
    {
        return vec3(1);
    }

    vec3 surfaceColor = trace.normal * 0.5 + 0.5;

    // Checkerboard
    bvec3 evenVec = greaterThanEqual(mod(surfacePos * 8, 2), vec3(1));
    bool checker = evenVec.y && evenVec.x == evenVec.z || !evenVec.y && evenVec.x != evenVec.z;

    if (checker)
    {
        surfaceColor *= 0.5;
    }

    // Trace shadow ray
    vec3 shadowDir = normalize(vec3(2, 3, 1));
    TraceResult shadowResult = TraceVoxelRay(surfacePos, shadowDir);

    if (shadowResult.hit)
    {
        surfaceColor *= 0.4;
    }

    return surfaceColor;
}

void main() {
    vec2 proportion = (gl_FragCoord.xy / uInput.screenSize - 0.5) * 2;

    vec3 viewSpaceRayDir = vec3(proportion.xy, 1);
    normalize(viewSpaceRayDir);

    // World space ray dir
    vec3 rayDir = (uInput.invView * vec4(viewSpaceRayDir, 0)).xyz;
    rayDir.y *= -1;

    TraceResult result = TraceVoxelRay(uInput.startPos, rayDir);

    if (!result.hit)
    {
         discard;
    }

    vec3 surfacePos = result.position + result.normal * 0.00001;
    vec3 surfaceColor = GetSurfaceColor(result, surfacePos);

    // Trace reflect ray
    vec3 reflectDir = reflect(rayDir, result.normal);
    TraceResult reflectResult = TraceVoxelRay(surfacePos, reflectDir);

    vec3 reflectColor = GetSurfaceColor(reflectResult, reflectResult.position + reflectResult.normal * 0.00001);
    float fresnel = 0.01 + 0.3 * pow(1 + dot(result.normal, rayDir), 2);

    surfaceColor = mix(surfaceColor, reflectColor, fresnel);

    // Fog
    float fogAmt = result.dist / 32;
    surfaceColor = mix(surfaceColor, vec3(1), fogAmt * fogAmt);

    outColor = vec4(surfaceColor, 1);
}