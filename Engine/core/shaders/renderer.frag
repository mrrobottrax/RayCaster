#version 450

// Based on https://www.shadertoy.com/view/4dX3zl

layout(binding = 0) uniform readonly RendererInput {
    mat4 camMat;
    uvec2 screenSize;
    vec3 startPos;
    float aspect;
} uInput;

struct MaterialData
{
    float reflectivity;
    bool shouldRefract;
    float ior;
};

const MaterialData materials[6] = MaterialData[6](
        MaterialData(0, false, 1), // Grass
        MaterialData(0.4, false, 1), // Iron
        MaterialData(0.1, false, 1), // Wood
        MaterialData(0.1, false, 1), // Cbbl
        MaterialData(1, true, 1.52), // Glass
        MaterialData(0.9, false, 1) // Mirror
    );

layout(r8ui, binding = 1) uniform readonly uimage3D uChunk;
layout(binding = 2) uniform sampler2DArray uTextureSampler;
layout(binding = 3) uniform sampler2DArray uNormalSampler;

layout(location = 0) out vec4 outColor;

const int chunkSize = 64;
const vec3 sunDir = normalize(vec3(-2, -3, -1));
const vec3 skyColor = vec3(0.7, 0.9, 1);
const float shadowMultiply = 0.4;

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

TraceResult TraceVoxelRay(vec3 startPos, vec3 rayDir, uint maxSteps, bool includeRefracted)
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
	uint i;
    for (i = 0; i < maxSteps; ++i)
    {
        // Step
        mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));
        oldSideDist = sideDist;
        sideDist += vec3(mask) * inc;
        gridPos += ivec3(mask) * rayStep;

        // Check bounds
        if (gridPos.x < 0 || gridPos.y < 0 || gridPos.z < 0 ||
                gridPos.x >= chunkSize || gridPos.y >= chunkSize || gridPos.z >= chunkSize)
        {
            hit = false;
            break;
        }

        // Check voxel
        blockId = imageLoad(uChunk, gridPos).r;

        if (blockId > 0)
        {
			if (includeRefracted)
			{
				hit = true;
				break;
			}

			if (!materials[blockId - 1].shouldRefract)
			{
				hit = true;
				break;
			}
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
    result.direction = rayDir;

    return result;
}

vec3 DarkenWithLight(vec3 color, vec3 normal)
{
	float brightness = clamp(dot(normal, -sunDir) * 0.5 + 0.5, 0.1, 1);
	color *= brightness;
	return color;
}

vec2 GetUV(TraceResult trace)
{
    vec3 surfacePos = trace.position + trace.normal * 0.0001;
    vec3 uv0 = vec3(not(trace.mask)) * mod(surfacePos, 1);

    vec2 uvX = vec2(uv0.z, uv0.y);
    vec2 uvY = vec2(uv0.x, uv0.z);
    vec2 uvZ = vec2(uv0.x, uv0.y);

    float x = float(!trace.mask.y) * float(!trace.mask.z);
    float y = float(!trace.mask.x) * float(!trace.mask.z);
    float z = float(!trace.mask.x) * float(!trace.mask.y);

    vec2 uv = uvX * x + uvY * y + uvZ * z;

	return uv;
}

vec3 GetSurfaceColor(TraceResult trace)
{
    if (!trace.hit)
    {
        return skyColor;
    }

	vec2 uv = GetUV(trace);
    vec3 surfaceColor = texture(uTextureSampler, vec3(uv, trace.blockId - 1)).rgb;

    // Checkerboard
    //    bvec3 evenVec = greaterThanEqual(mod(surfacePos * 8, 2), vec3(1));
    //    bool checker = evenVec.y && evenVec.x == evenVec.z || !evenVec.y && evenVec.x != evenVec.z;
    //
    //    if (checker)
    //    {
    //        surfaceColor *= 0.5;
    //    }

    return surfaceColor;
}

vec3 GetSurfaceNormal(TraceResult trace)
{
    if (!trace.hit)
    {
        return skyColor;
    }

	vec2 uv = GetUV(trace);
    vec3 surfaceNormal = texture(uNormalSampler, vec3(uv, trace.blockId - 1)).rgb;

    return surfaceNormal;
}

vec3 ApplyNormalMap(vec3 worldNormal, vec3 tangentNormal)
{
    tangentNormal = tangentNormal * 2.0 - 1.0;
	//tangentNormal = normalize(tangentNormal);

    vec3 tangent = abs(worldNormal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 bitangent = normalize(-cross(tangent, worldNormal));
    
    mat3 TBN = mat3(bitangent, tangent, worldNormal);
    
   return normalize(TBN * tangentNormal);
}

struct ReflectResult
{
	bool didReflect;
	vec3 reflectColor;
	float fresnel;
};
ReflectResult TraceReflectRay(MaterialData material, TraceResult trace, vec3 surfacePos)
{
		float fresnel = material.reflectivity + (1 - material.reflectivity) *  pow(1 + dot(trace.normal, trace.direction), 5);
		fresnel *= material.reflectivity;
		fresnel = clamp(fresnel, 0, 1);
		if (fresnel <= 0.001)
		{
			return ReflectResult(false, vec3(1), 0);
		}

		vec3 reflectDir = reflect(trace.direction, trace.normal);
		trace = TraceVoxelRay(surfacePos, reflectDir, 64, false);

		vec3 reflectColor = GetSurfaceColor(trace);
		if (trace.hit)
		{
			reflectColor = DarkenWithLight(reflectColor, trace.normal);

			// Trace shadow ray
			vec3 surfacePosRefract = trace.position + trace.normal * 0.0001;
			TraceResult shadowResult = TraceVoxelRay(surfacePosRefract, -sunDir, 64, false);

			if (shadowResult.hit)
			{
				reflectColor *= shadowMultiply;
			}
		}

		// Fog
		float fogAmt = min(trace.dist / 64, 1);
		reflectColor = mix(reflectColor, skyColor, fogAmt * fogAmt);

		return ReflectResult(true, reflectColor, fresnel);
}

void main() {
    vec2 proportion = (gl_FragCoord.xy / uInput.screenSize - 0.5) * 2;
    proportion.x *= uInput.aspect;

    vec3 viewSpaceRayDir = vec3(proportion.xy, 1);
    viewSpaceRayDir = normalize(viewSpaceRayDir);

    // World space ray dir
    vec3 rayDir = (uInput.camMat * vec4(viewSpaceRayDir, 0)).xyz;
    rayDir.y *= -1;

    TraceResult trace = TraceVoxelRay(uInput.startPos, rayDir, 1024, true);

    if (!trace.hit)
    {
        discard;
    }

    vec3 surfaceColor = GetSurfaceColor(trace);
	vec3 surfaceNormal = GetSurfaceNormal(trace);
    vec3 surfacePos = trace.position + trace.normal * 0.0001;
	trace.normal = ApplyNormalMap(trace.normal, surfaceNormal);

    // Trace shadow ray
    TraceResult shadowResult = TraceVoxelRay(surfacePos, -sunDir, 64, false);

    if (shadowResult.hit)
    {
        surfaceColor *= shadowMultiply;
    }

    MaterialData material = materials[trace.blockId - 1];

	float fogDist = trace.dist;

    if (!material.shouldRefract)
    {
		surfaceColor = DarkenWithLight(surfaceColor, trace.normal);

        // Trace reflect ray
        ReflectResult reflectResult = TraceReflectRay(material, trace, surfacePos);
		if (reflectResult.didReflect)
		{
			surfaceColor = mix(surfaceColor, reflectResult.reflectColor, reflectResult.fresnel);
		}
    }
    else
    {
        // Trace refract ray
        vec3 refractDir = refract(trace.direction, trace.normal, 1.0 / material.ior);
        trace = TraceVoxelRay(surfacePos, refractDir, 64, false);

        vec3 refractColor = GetSurfaceColor(trace);
		if (trace.hit)
		{
       	 	vec3 refractNormal = GetSurfaceNormal(trace);
			refractColor = DarkenWithLight(refractColor, trace.normal);

			// Trace shadow ray
			surfacePos = trace.position + trace.normal * 0.0001;
			trace.normal = ApplyNormalMap(trace.normal, refractNormal);
			material = materials[trace.blockId - 1];
			TraceResult shadowResult = TraceVoxelRay(surfacePos, -sunDir, 64, false);

			if (shadowResult.hit)
			{
				refractColor *= shadowMultiply;
			}

			// Trace reflect ray
			ReflectResult reflectResult = TraceReflectRay(material, trace, surfacePos);
			if (reflectResult.didReflect)
			{
				refractColor = mix(refractColor, reflectResult.reflectColor, reflectResult.fresnel);
			}
		}

		// Fog
		float fogAmt = min(trace.dist / 64, 1);
		refractColor = mix(refractColor, skyColor, fogAmt * fogAmt);

        surfaceColor = surfaceColor * refractColor;
    }

    // Fog
    float fogAmt = min(fogDist / 64, 1);
    surfaceColor = mix(surfaceColor, skyColor, fogAmt * fogAmt);

    outColor = vec4(surfaceColor, 1);
}
