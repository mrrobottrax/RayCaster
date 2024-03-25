// Constants
static const uint UINT_MAX = 4294967295;

static const int maxDepth = 3;
static const int pi = 3.14159265358979323846f;
static const int two_pi = pi * 2;
static const int sampleCount = 16;

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
Texture3D<int> LevelInput : register(t0);
RWTexture2D<float4> TexOut : register(u0);

// Structs
struct Ray
{
	float3 origin;
	float3 direction;
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
	float u1 = Randf();
	float u2 = Randf();
	
	const float r = sqrt(u1);
	const float theta = two_pi * u2;
 
	const float x = r * cos(theta);
	const float y = r * sin(theta);
	
	return float3(x, y, sqrt(max(0.0f, 1 - u1)));
}

float3 TangentToWorld(const float3 vec, const float3 normal)
{
	if (normal.z == 1)
	{
		return float3(vec.x, vec.y, vec.z);
	}
	else if (normal.z == -1)
	{
		return float3(0, 0, -1);
	}
	else if (normal.x == 1)
	{
		return float3(vec.z, vec.y, -vec.x);
	}
	else if (normal.x == -1)
	{
		return float3(-vec.z, vec.y, vec.x);
	}
	else if (normal.y == 1)
	{
		return float3(vec.x, vec.z, -vec.y);
	}
	else if (normal.y == -1)
	{
		return float3(vec.x, -vec.z, vec.y);
	}
	
	return vec;
}

// Get the contents of a position
int GetGridType(const uint2 wallPosition)
{
	if (wallPosition.x < 0 || wallPosition.y < 0 || // Negative
		wallPosition.x >= 5 || wallPosition.y >= 5) // Out of range
		return 1;
	
	return LevelInput[uint3(wallPosition.x, wallPosition.y, 0)];
}

// Floor float3
uint3 GetGridPos(float3 pos)
{
	return uint3(floor(pos));
}

// Cast a ray against level geomentry
RaycastResult CastRay(const Ray ray)
{
	float3 pos = ray.origin;
	uint3 gridPos = GetGridPos(pos);
	int contents = 0;

	int lastType = 0;
	while (contents <= 0)
	{
		float distX, distY, distZ;

		// find distances to next line
		if (lastType == 1)
		{
			distX = 1 / abs(ray.direction.x);
		}
		else
		{
			distX = ray.direction.x > 0 ? 1 - fmod(pos.x, 1) : fmod(pos.x, 1);
			distX /= abs(ray.direction.x);
		}

		if (lastType == 2)
		{
			distY = 1 / abs(ray.direction.y);
		}
		else
		{
			distY = ray.direction.y > 0 ? 1 - fmod(pos.y, 1) : fmod(pos.y, 1);
			distY /= abs(ray.direction.y);
		}

		distZ = ray.direction.z > 0 ? 1 - fmod(pos.z, 1) : fmod(pos.z, 1);
		distZ /= abs(ray.direction.z);

		// move to the closest line
		if (distX < distY && distX < distZ)
		{
			// move x
			gridPos.x += ray.direction.x > 0 ? 1 : -1;

			pos.x += distX * ray.direction.x;
			pos.y += distX * ray.direction.y;
			pos.z += distX * ray.direction.z;

			lastType = 1;
		}
		else if (distY < distX && distY < distZ)
		{
			// move y
			gridPos.y += ray.direction.y > 0 ? 1 : -1;

			pos.x += distY * ray.direction.x;
			pos.y += distY * ray.direction.y;
			pos.z += distY * ray.direction.z;

			lastType = 2;
		}
		else
		{
			// move z
			pos.x += distZ * ray.direction.x;
			pos.y += distZ * ray.direction.y;
			pos.z += distZ * ray.direction.z;

			const int wall = GetGridType(gridPos.xy);

			if (wall < 0)
			{
				contents = wall;
			}
			else if (ray.direction.z < 0)
			{
				contents = -1;
			}
			else
			{
				contents = 0;
			}

			// return a floor/ceiling
			RaycastResult result =
			{
				pos,
				float3(0, 0, ray.direction.z > 0 ? -1.f : 1.f),
				contents
			};
			return result;
		}

		// check if wall is solid
		contents = GetGridType(gridPos.xy);
	}

	float3 normal;
	if (lastType == 1)
	{
		normal = float3(ray.direction.x > 0 ? -1.f : 1.f, 0, 0);
	}
	else if (lastType == 2)
	{
		normal = float3(0, ray.direction.y > 0 ? -1.f : 1.f, 0);
	}

	RaycastResult result =
	{
		pos,
		normal,
		contents
	};
	return result;
}

struct Material
{
	float3 emittance;
	float3 reflectance;
};

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
		material.reflectance = float3(0.9f, 0, 0);
	}
	else if (abs(normal.y) == 1)
	{
		material.reflectance = float3(0, 0.9f, 0);
	}
	else
	{
		material.reflectance = float3(0.9f, 0.9f, 0.9f);
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

float3 Sample(const RaycastResult startHit)
{
	PathHit hits[maxDepth];
	
	hits[0].cos_theta = 1;
	hits[0].probability = 1;
	hits[0].material = GetMaterial(startHit.wallType, startHit.normal);
	
	RaycastResult result = startHit;
	for (int i = 1; i < maxDepth; ++i)
	{
		PathHit hit;
		
		// Pick a random direction from here and keep going.
		Ray ray;
		ray.origin = result.hitPoint + result.normal * 0.0001f;
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
	
	// Collect samples
	float3 color = float3(0, 0, 0);
	for (int i = 0; i < sampleCount; ++i)
	{
		color += Sample(hit) / float(sampleCount);
	}

	color.x = color.x > 1 ? 1 : color.x;
	color.y = color.y > 1 ? 1 : color.y;
	color.z = color.z > 1 ? 1 : color.z;
	
	TexOut[DTid.xy] = float4(color, 1);
}