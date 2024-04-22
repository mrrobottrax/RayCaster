// Constants
static const uint UINT_MAX = 4294967295;

static const int maxDepth = 3;
static const int pi = 3.14159265358979323846f;
static const int two_pi = pi * 2;
static const int sampleCount = 512;

// Input
struct CameraData
{
	float3 position;
	float angle;
	uint frame;
};

cbuffer Buffer0 : register(b0)
{
	CameraData cameraData;
}
Texture2D<int> LevelInput : register(t0);
RWTexture2D<float4> TexOut : register(u0);

// Structs
struct Ray
{
	float3 origin;
	float3 direction;
};

struct Material
{
	float3 emittance;
	float3 reflectance;
};

struct RaycastResult
{
	float3 hitPoint;
	float3 normal;
	int wallType;
};

// Random number using PCG
static uint seed;
uint Rand()
{
	seed = seed * 747796405u + 2891336453u;
	uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
	return (word >> 22u) ^ word;
}

// Random float from 0 to 1
float Randf()
{
	return float(Rand()) / float(UINT_MAX);
}

// Cosine weighted hemisphere vector in tangent space
float3 RandomHemisphereVectorCosine()
{
	const float u1 = Randf();
	const float u2 = Randf();
	
	const float r = sqrt(u1);
	const float theta = two_pi * u2;
 
	const float x = r * cos(theta);
	const float y = r * sin(theta);

	return float3(x, y, sqrt(1 - u1));
}

float3 TangentToWorld(const float3 vec, const float3 normal)
{
	float3 x = cross(normal, abs(normal.z) == 1 ? float3(1, 0, 0) : float3(0, 0, 1));
	float3 y = cross(normal, x);
	
	return x * vec.x + y * vec.y + normal * vec.z;
}

// Get the contents of a position
int GetGridType(const uint2 wallPosition)
{
	if (wallPosition.x < 0 || wallPosition.y < 0 || // Negative
		wallPosition.x >= 5 || wallPosition.y >= 5) // Out of range
		return 1;
	
	return LevelInput[uint2(wallPosition.x, wallPosition.y)];
}

// Floor float3
uint3 GetGridPos(float3 pos)
{
	return uint3(floor(pos));
}

// Cast a ray against level geomentry
RaycastResult CastRay(const Ray ray)
{
	int3 gridPos = GetGridPos(ray.origin);
	
	int contents = 0;
	
	const int3 dirSign = sign(ray.direction);
	const float3 slope = 1.f / ray.direction;
	
	float3 dist = (gridPos - ray.origin + (0.5 + dirSign * 0.5)) * slope; // dist to line on each axis
	
	bool3 minimum;
	
	while (contents <= 0)
	{
		minimum.x = dist.x < dist.y && dist.x < dist.z;
		minimum.y = dist.y < dist.x && dist.y < dist.z;
		minimum.z = !(minimum.x || minimum.y);
		
		dist += minimum * dirSign * slope;
		gridPos += minimum * dirSign;
		
		contents = GetGridType(gridPos.xy);
		
		if (minimum.z)
		{
			float t = (-ray.origin.z + 0.5 + 0.5 * dirSign.z) * slope.z;
			
			RaycastResult result =
			{
				ray.origin + ray.direction * t,
				float3(0, 0, -dirSign.z),
				dirSign.z == 1 ? contents : 1
			};
			return result;
		}
	}
	
	float3 axis = (gridPos - ray.origin + 0.5 - 0.5 * dirSign) * slope;
	float t = max(axis.x, axis.y);
	
	const float3 normal = minimum.x ? float3(-dirSign.x, 0, 0) : float3(0, -dirSign.y, 0);
	
	RaycastResult result =
	{
		ray.origin + ray.direction * t,
		normal,
		contents
	};
	return result;
}

Material GetMaterial(int wallType, float3 normal)
{
	Material material =
	{
		float3(0, 0, 0),
		float3(0, 0, 0)
	};
	
	// Sky
	if (wallType == 0)
	{
		material.emittance = float3(5, 5, 5);
		return material;
	}
	
	// Wall
	if (abs(normal.x) == 1)
	{
		material.reflectance = float3(0.9, 0, 0);
	}
	else if (abs(normal.y) == 1)
	{
		material.reflectance = float3(0, 0.9, 0);
	}
	else
	{
		material.reflectance = float3(0.9, 0.9, 0.9);
	}
	
	return material;
}

// Path tracing
struct PathHit
{
	Material material;
	float cos_theta;
	float probability;
};

float3 Sample(const RaycastResult startHit, const PathHit startPathHit)
{
	PathHit hits[maxDepth];
	hits[0] = startPathHit;
	
	RaycastResult result = startHit;
	for (int i = 1; i < maxDepth; ++i)
	{
		PathHit hit;
		
		// Pick a random direction from here and keep going.
		Ray ray;
		ray.origin = result.hitPoint + result.normal * 0.01f;
		ray.direction = TangentToWorld(RandomHemisphereVectorCosine(), result.normal);
		
		hit.cos_theta = dot(ray.direction, result.normal);
		// hit.probability = 1.f / (2.f * pi); // Uniform
		hit.probability = hit.cos_theta / pi; // Cosine weighted
		
		result = CastRay(ray);

		hit.material = GetMaterial(result.wallType, result.normal);
		
		hits[i] = hit;
	}

	// Apply rendering equation
	float3 incoming = float3(0, 0, 0);
	for (int j = maxDepth - 1; j >= 0; --j)
	{
		PathHit hit = hits[j];
		
		float3 BRDF = hit.material.reflectance / pi;
		
		float3 BRDFxLi = float3(
			incoming.x * BRDF.x,
			incoming.y * BRDF.y,
			incoming.z * BRDF.z
		);
		
		hit.probability = max(hit.probability, 0.01f);
		incoming = hit.material.emittance + (BRDFxLi * hit.cos_theta / hit.probability);
	}
	
	return incoming;
}

// Get a ray that fires to where the pixel projects to
Ray GetPixelRay(uint x, uint y, float3 forwards, float3 right, float3 up)
{
	uint width, height;
	TexOut.GetDimensions(width, height);
	
	float rightScale = x / float(width) * 2.f - 1;
	float upScale = -(y / float(height) * 2.f) + 1;

	float3 dir = forwards + right * rightScale + up * upScale;
	dir = normalize(dir);
	
	Ray ray =
	{
		cameraData.position,
		dir
	};
	return ray;
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	seed = DTid.x;
	seed = DTid.y ^ Rand();
	seed = cameraData.frame ^ Rand();
	
	// Get direction vectors
	const float yaw = cameraData.angle;
	
	const float3 forwards = float3(cos(yaw), sin(yaw), 0);
	const float3 right = float3(sin(yaw), -cos(yaw), 0);
	const float3 up = float3(0, 0, 1);
	
	// Get ray
	Ray ray = GetPixelRay(DTid.x, DTid.y, forwards, right, up);
	RaycastResult hit = CastRay(ray);
	
	PathHit pathHit;
	pathHit.cos_theta = 1;
	pathHit.probability = 1;
	pathHit.material = GetMaterial(hit.wallType, hit.normal);
	
	// Collect samples
	float3 color = float3(0, 0, 0);
	for (int i = 0; i < sampleCount; ++i)
	{
		color += Sample(hit, pathHit) / float(sampleCount);
	}

	color.x = color.x > 1 ? 1 : color.x;
	color.y = color.y > 1 ? 1 : color.y;
	color.z = color.z > 1 ? 1 : color.z;
	
	TexOut[DTid.xy] = float4(color, 1);
}