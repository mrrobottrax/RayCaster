// Constants
static const uint UINT_MAX = 4294967295;

static const int maxDepth = 3;
static const int pi = 3.14159265358979323846f;
static const int sampleCount = 64;

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
float Ranf()
{
	return float(Rand()) / float(UINT_MAX);
}

// Generate a random vector
float3 RandomUnitVector()
{
	float3 vec = float3
	(
		Ranf() * 2 - 1,
		Ranf() * 2 - 1,
		Ranf() * 2 - 1
	);

	return normalize(vec);
}

// Generate a random vector in a hemisphere
float3 RandomHemisphereVector(const float3 normal)
{
	float3 vec = RandomUnitVector();

	if (dot(vec, normal) < 0)
	{
		vec *= -1;
	}

	return vec;
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

// Path tracing
struct PathHit
{
	float3 BRDF;
	float3 emittance;
	float cos_theta;
};

float3 TracePath(const Ray startRay, const int depth)
{
	PathHit hits[maxDepth];

	int hitCount = -1;
	Ray ray = startRay;
	for (int i = 0; i < maxDepth; ++i)
	{
		const RaycastResult result = CastRay(ray);
		if (result.wallType == 0)
		{
			PathHit hit =
			{
				float3(0, 0, 0),
				float3(2, 2, 2),
				0
			};
			hits[i] = hit;
			++hitCount;
			break; // Skybox was hit.
		}

		// Material material = ray.thingHit->material;
		// Color emittance = material.emittance;
		float3 emittance = float3(0, 0, 0);

		// Pick a random direction from here and keep going.
		ray.origin = result.hitPoint + result.normal * 0.0001f;

		// This is NOT a cosine-weighted distribution!
		ray.direction = RandomHemisphereVector(result.normal);

		// Compute the BRDF for this ray (assuming Lambertian reflection)
		const float cos_theta = dot(ray.direction, result.normal);
		// Color BRDF = material.reflectance / PI;
		float3 BRDF;

		BRDF = float3(1, 0, 0) / pi;
		
		if (abs(result.normal.x) == 1)
		{
			BRDF = float3(0.9f, 0, 0) / pi;
		}
		else if (abs(result.normal.y) == 1)
		{
			BRDF = float3(0, 0.9f, 0) / pi;
		}
		else
		{
			BRDF = float3(0.9f, 0.9f, 0.9f) / pi;
		}

		PathHit hit =
		{
			BRDF,
			emittance,
			cos_theta
		};
		hits[i] = hit;
		
		++hitCount;
	}

	// Probability of the newRay
	const float p = 1 / (2.f * pi);

	// Apply rendering equation
	float3 color = float3(0, 0, 0);
	for (int j = hitCount; j >= 0; --j)
	{
		float3 mult = float3(
			color.x * hits[j].BRDF.x,
			color.y * hits[j].BRDF.y,
			color.z * hits[j].BRDF.z
		);

		// Apply the Rendering Equation here.
		color = hits[j].emittance + (mult * hits[j].cos_theta / p);
	}

	return color;
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
	
	// Collect samples
	float3 color = float3(0, 0, 0);
	for (int i = 0; i < sampleCount; ++i)
	{
		color += TracePath(ray, 0) / float(sampleCount);
	}

	color.x = color.x > 1 ? 1 : color.x;
	color.y = color.y > 1 ? 1 : color.y;
	color.z = color.z > 1 ? 1 : color.z;
	
	TexOut[DTid.xy] = float4(color, 1);
}