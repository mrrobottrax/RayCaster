#version 450

// Based on https://www.shadertoy.com/view/4dX3zl

layout(binding = 0) uniform readonly RendererInput {
    mat4 invView;
    uvec2 screenSize;
    vec3 startPos;
    float aspect;
} uInput;

layout(r8ui, binding = 1) uniform readonly uimage3D uChunk;
layout(binding = 2) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 outColor;

const int chunkSize = 64;
const vec3 sunDir = normalize(vec3(-2, -3, -1));
const vec3 skyColor = vec3(0.53, 0.81, 0.92);

struct TraceResult
{
    bool hit;
    uint blockId;
    vec3 normal;
    vec3 position;
    ivec3 blockCoord;
    bvec3 mask;
    float dist;
};

TraceResult TraceVoxelRay(vec3 startPos, vec3 rayDir, uint maxSteps)
{
    rayDir = normalize(rayDir);

    // Start tracing
    uint blockId;
    bvec3 mask = bvec3(false);
    ivec3 gridPos = ivec3(startPos);
    ivec3 rayStep = ivec3(sign(rayDir));
    vec3 slope = abs(1 / rayDir);
    vec3 sideDist = (sign(rayDir) * (vec3(gridPos) - startPos) + (sign(rayDir) * 0.5) + 0.5) * slope; 
    vec3 oldSideDist = vec3(0);

    bool hit = false;

    for (uint i = 0; i < maxSteps; ++i)
    {          
        // Check voxel
        blockId = imageLoad(uChunk, gridPos).r;

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
    result.position = startPos + rayDir * dist;
    result.mask = mask;
    result.dist = dist;
    result.blockCoord = gridPos;

    return result;
}

vec3 GetSurfaceColor(TraceResult trace)
{
    if (!trace.hit)
    {
        return skyColor;
    }

    vec3 surfacePos = trace.position + trace.normal * 0.00001;
    vec3 uv0 = vec3(not(trace.mask)) * mod(surfacePos, 1);

    vec2 uvX = vec2(uv0.z, uv0.y);
    vec2 uvY = vec2(uv0.x, uv0.z);
    vec2 uvZ = vec2(uv0.x, uv0.y);

    float x = float(!trace.mask.y) * float(!trace.mask.z);
    float y = float(!trace.mask.x) * float(!trace.mask.z);
    float z = float(!trace.mask.x) * float(!trace.mask.y);

    vec2 uv = uvX * x + uvY * y + uvZ * z;
    vec3 surfaceColor = texture(uTextureSampler, uv).rgb;

    // Checkerboard
//    bvec3 evenVec = greaterThanEqual(mod(surfacePos * 8, 2), vec3(1));
//    bool checker = evenVec.y && evenVec.x == evenVec.z || !evenVec.y && evenVec.x != evenVec.z;
//
//    if (checker)
//    {
//        surfaceColor *= 0.5;
//    }

    // Brightness
    float brightness = clamp(dot(trace.normal, -sunDir) * 0.5 + 0.5, 0.05, 1);

    surfaceColor *= brightness;

    return surfaceColor;
}

void main() {
    vec2 proportion = (gl_FragCoord.xy / uInput.screenSize - 0.5) * 2;
    proportion.x *= uInput.aspect;

    vec3 viewSpaceRayDir = vec3(proportion.xy, 1);
    viewSpaceRayDir = normalize(viewSpaceRayDir);

    // World space ray dir
    vec3 rayDir = (uInput.invView * vec4(viewSpaceRayDir, 0)).xyz;
    rayDir.y *= -1;

    TraceResult result = TraceVoxelRay(uInput.startPos, rayDir, 1024);

    if (!result.hit)
    {
         discard;
    }

    vec3 surfacePos = result.position + result.normal * 0.00001;
    vec3 surfaceColor = GetSurfaceColor(result);

    // Trace reflect ray
    float fresnel = 0.0001 + 0.8 * pow(1.0 + dot(result.normal, rayDir), 6);
    if (fresnel > 0.001)
    {
        vec3 reflectDir = reflect(rayDir, result.normal);
        TraceResult reflectResult = TraceVoxelRay(surfacePos, reflectDir, 32);

        vec3 reflectColor = GetSurfaceColor(reflectResult);

        surfaceColor = mix(surfaceColor, reflectColor, fresnel);
    }

    // Trace shadow ray
    TraceResult shadowResult = TraceVoxelRay(surfacePos, -sunDir, 64);

    if (shadowResult.hit)
    {
        surfaceColor *= 0.4;
    }

    // Fog
    float fogAmt = min(result.dist / 64, 1);
    surfaceColor = mix(surfaceColor, skyColor, fogAmt * fogAmt);

    outColor = vec4(surfaceColor, 1);
}