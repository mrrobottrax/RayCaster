#version 450

// Based on https://www.shadertoy.com/view/4dX3zl

layout(binding = 0) uniform readonly RendererInput {
    mat4 camMat;
    uvec2 screenSize;
    vec3 startPos;
    float aspect;
} uInput;

layout(r8ui, binding = 1) uniform readonly uimage3D uChunk;
layout(binding = 2) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 outColor;

const int chunkSize = 64;
const vec3 sunDir = normalize(vec3(-2, -3, -1));
const vec3 skyColor = vec3(0.7, 0.9, 1);

struct TraceResult
{
    bool hit;
    uint blockId;
    vec3 normal;
    vec3 position;
    vec3 direction;
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
    vec3 inc = abs(1 / rayDir);
    vec3 sideDist = (sign(rayDir) * (vec3(gridPos) - startPos) + (sign(rayDir) * 0.5) + 0.5) * inc; 
    vec3 oldSideDist = vec3(0);

    bool hit = false;
    for (uint i = 0; i < maxSteps; ++i)
    {        
        // Check bounds
        if (gridPos.x < 0 || gridPos.y < 0 || gridPos.z < 0 ||
            gridPos.x >= chunkSize || gridPos.y >= chunkSize || gridPos.z >= chunkSize)
        {
            hit = false;
            break;
        }

        // Check voxel
        blockId = imageLoad(uChunk, gridPos).r;

        if (blockId > 1)
        {
            hit = true;
            break;
        }
        else if (blockId == 1)
        {
            // Glass
            // Set start pos
            mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));

            vec3 maskedDist = vec3(mask) * sideDist;
            float dist = max(maskedDist.x, max(maskedDist.y, maskedDist.z));
            startPos = startPos + rayDir * dist;

            // Refract
            rayDir = refract(rayDir, vec3(mask) * -rayStep, 0.9); // todo: this is broken

            // Restart trace with new direction
            rayStep = ivec3(sign(rayDir));
            inc = abs(1 / rayDir);
            sideDist = (sign(rayDir) * (vec3(gridPos) - startPos) + (sign(rayDir) * 0.5) + 0.5) * inc;
            oldSideDist = vec3(0);
        }

        // Step
        mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));
        oldSideDist = sideDist;
        sideDist += vec3(mask) * inc;
		gridPos += ivec3(mask) * rayStep;
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
    result.direction = rayDir;

    return result;
}

vec3 GetSurfaceColor(TraceResult trace)
{
    if (!trace.hit)
    {
        return skyColor;
    }

    vec3 surfacePos = trace.position + trace.normal * 0.0001;
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
    float brightness = clamp(dot(trace.normal, -sunDir) * 0.5 + 0.5, 0.1, 1);
    surfaceColor *= brightness;

    return surfaceColor;
}

void main() {
    vec2 proportion = (gl_FragCoord.xy / uInput.screenSize - 0.5) * 2;
    proportion.x *= uInput.aspect;

    vec3 viewSpaceRayDir = vec3(proportion.xy, 1);
    viewSpaceRayDir = normalize(viewSpaceRayDir);

    // World space ray dir
    vec3 rayDir = (uInput.camMat * vec4(viewSpaceRayDir, 0)).xyz;
    rayDir.y *= -1;

    TraceResult trace = TraceVoxelRay(uInput.startPos, rayDir, 1024);

    if (!trace.hit)
    {
         discard;
    }

    vec3 surfaceColor = GetSurfaceColor(trace);
    vec3 surfacePos = trace.position + trace.normal * 0.0001;

    // Trace shadow ray
    TraceResult shadowResult = TraceVoxelRay(surfacePos, -sunDir, 64);

    if (shadowResult.hit)
    {
        surfaceColor *= 0.4;
    }

    // Trace reflect ray
    float fresnel = 0.01 + 0 * pow(1.0 + dot(trace.normal, trace.direction), 1);
    if (fresnel > 0.001)
    {
        vec3 reflectDir = reflect(trace.direction, trace.normal);
        TraceResult reflectResult = TraceVoxelRay(surfacePos, reflectDir, 32);

        vec3 reflectColor = GetSurfaceColor(reflectResult);

        surfaceColor = mix(surfaceColor, reflectColor, fresnel);
    }

    // Fog
    float fogAmt = min(trace.dist / 64, 1);
    surfaceColor = mix(surfaceColor, skyColor, fogAmt * fogAmt);

    outColor = vec4(surfaceColor, 1);
}