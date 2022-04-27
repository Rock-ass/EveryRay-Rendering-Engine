#define NUM_OF_SHADOW_CASCADES 3

#define VOLUMETRIC_FOG_VOXEL_SIZE_X 160
#define VOLUMETRIC_FOG_VOXEL_SIZE_Y 90
#define VOLUMETRIC_FOG_VOXEL_SIZE_Z 128

static const float PI = 3.141592654f;

struct QUAD_VS_IN
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct QUAD_VS_OUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float LinearizeDepth(float depth, float NearPlaneZ, float FarPlaneZ)
{
    float ProjectionA = FarPlaneZ / (FarPlaneZ - NearPlaneZ);
    float ProjectionB = (-FarPlaneZ * NearPlaneZ) / (FarPlaneZ - NearPlaneZ);

	// Sample the depth and convert to linear view space Z (assume it gets sampled as
	// a floating point value of the range [0,1])
    float linearDepth = ProjectionB / (depth - ProjectionA);

    return linearDepth;
}

float LinearToExponentialDepth(float z, float NearPlaneZ, float FarPlaneZ)
{
    float z_buffer_params_y = FarPlaneZ / NearPlaneZ;
    float z_buffer_params_x = 1.0f - z_buffer_params_y;

    return (1.0f / z - z_buffer_params_y) / z_buffer_params_x;
}
float ExponentialToLinearDepth(float z, float n, float f)
{
    float z_buffer_params_y = f / n;
    float z_buffer_params_x = 1.0f - z_buffer_params_y;

    return 1.0f / (z_buffer_params_x * z + z_buffer_params_y);
}

float3 GetUVFromVolumetricFogVoxelWorldPos(float3 worldPos, float n, float f, float4x4 viewProj)
{
    float4 ndc = mul(float4(worldPos, 1.0f), viewProj);
    ndc = ndc / ndc.w;
    
    float3 uv;
    uv.x = ndc.x * 0.5f + 0.5f;
    uv.y = ndc.y * 0.5f + 0.5f;
    uv.z = ExponentialToLinearDepth(ndc.z * 0.5f + 0.5f, n, f);
    
    float2 params = float2(float(VOLUMETRIC_FOG_VOXEL_SIZE_Z) / log2(f / n), -(float(VOLUMETRIC_FOG_VOXEL_SIZE_Z) * log2(n) / log2(f / n)));
    float view_z = uv.z * f;
    uv.z = (max(log2(view_z) * params.x + params.y, 0.0f)) / VOLUMETRIC_FOG_VOXEL_SIZE_Z;
    return uv;
}
float3 AddVolumetricFog(float3 inputColor, float3 worldPos, float nearPlane, float farPlane, float4x4 viewProj, SamplerState ss, Texture3D<float4> inVoxelTexture)
{
    float3 uv = GetUVFromVolumetricFogVoxelWorldPos(worldPos, nearPlane, farPlane, viewProj);
    float4 scatteredLight = inVoxelTexture.SampleLevel(ss, uv, 0.0f);
    return inputColor * scatteredLight.a + scatteredLight.rgb;
}