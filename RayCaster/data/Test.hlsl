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

// Get the contents of a position
int GetGridType(const uint3 wallPosition)
{
	if (wallPosition.x < 0 || wallPosition.y < 0 || wallPosition.z < 0 || // Negative
		wallPosition.x >= 256 || wallPosition.y >= 256 || wallPosition.z >= 256) // Out of range
		return 255;
	
	return LevelInput[uint3(wallPosition.x, wallPosition.y, wallPosition.z)] == 1 ? 1 : 0;
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
	float3 start = ray.origin;
	uint3 gridPos = GetGridPos(pos);
	int contents = 0;

	int lastType = 0;
	float totalDist = 0;
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
		
		if (lastType == 3)
		{
			distZ = 1 / abs(ray.direction.z);
		}
		else
		{
			distZ = ray.direction.z > 0 ? 1 - fmod(pos.z, 1) : fmod(pos.z, 1);
			distZ /= abs(ray.direction.z);
		}

		// move to the closest line
		if (distX < distY && distX < distZ)
		{
			// move x
			gridPos.x += ray.direction.x > 0 ? 1 : -1;

			totalDist += distX;
			
			lastType = 1;
		}
		else if (distY < distX && distY < distZ)
		{
			// move y
			gridPos.y += ray.direction.y > 0 ? 1 : -1;

			totalDist += distY;

			lastType = 2;
		}
		else
		{
			// move z
			gridPos.z += ray.direction.z > 0 ? 1 : -1;
			
			totalDist += distZ;

			lastType = 3;
		}
		
		pos = start + totalDist * ray.direction;

		// check if wall is solid
		contents = GetGridType(gridPos);
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
	else
	{
		normal = float3(0, 0, ray.direction.z > 0 ? -1.f : 1.f);
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
	float3 color;
};

Material GetMaterial(int wallType, float3 normal)
{
	Material material;
	
	// Sky
	if (wallType == 255)
	{
		material.color = float3(1, 1, 1);
	}
	
	// Wall
	else if (abs(normal.x) == 1)
	{
		material.color = float3(0.9f, 0, 0);
	}
	else if (abs(normal.y) == 1)
	{
		material.color = float3(0, 0.9f, 0);
	}
	else
	{
		material.color = float3(0.9f, 0.9f, 0.9f);
	}
	
	return material;
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
	// Get direction vectors
	const float yaw = cameraData.angle;
	
	const float3 forwards = float3(cos(yaw), sin(yaw), 0);
	const float3 right = float3(sin(yaw), -cos(yaw), 0);
	const float3 up = float3(0, 0, 1);
	
	// Get ray
	Ray ray = GetPixelRay(DTid.x, DTid.y, forwards, right, up);
	RaycastResult hit = CastRay(ray);
	
	// Collect samples
	float3 color = GetMaterial(hit.wallType, hit.normal).color;

	color.x = color.x > 1 ? 1 : color.x;
	color.y = color.y > 1 ? 1 : color.y;
	color.z = color.z > 1 ? 1 : color.z;
	
	TexOut[DTid.xy] = float4(color, 1);
}