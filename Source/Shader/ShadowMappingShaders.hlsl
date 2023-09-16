cbuffer scene : register(b0){
    float4x4    ViewMatrix;
    float4x4    ProjectionMatrix;
    float4      ViewPosition;
    float4      LightPosition;
    float4      LightAmbient;
    float4x4    LightViewMatrix;
    float4x4    LightProjectionMatrix;
    float       LightIntensity;
    bool        IsEnabledShadowMapping;
    float       ShadowMappingBias;
};

cbuffer local : register(b1){
    float4x4    WorldMatrix;
    float       Specular;
};

struct VS_INPUT{
    float3 Position : POSITION0;
    float4 Color    : COLOR0;
	float3 Normal   : NORMAL0;
    float2 UV       : TEXCOORD0;
};

// For Shadow Mapping
float4 VSShadowMap(VS_INPUT input) : SV_POSITION{

	float4 pos = float4(input.Position, 1.0f);
	pos = mul(WorldMatrix, pos);
	pos = mul(LightViewMatrix, pos);
    pos = mul(LightProjectionMatrix, pos);

	return pos;
}