cbuffer scene : register(b0){
    float4x4    ViewMatrix;
    float4x4    ProjectionMatrix;
    float4      ViewPosition;
};

cbuffer local : register(b1){
    float4x4    WorldMatrix;
};

Texture2D<float4> Texture0      : register(t0);
Texture2D<float4> ShadowMap     : register(t1);
TextureCube       CubeMap       : register(t2);
SamplerState      Sampler0      : register(s0);
SamplerState      Sampler1      : register(s1);

struct VS_INPUT{
    float3 Position : POSITION0;
	float3 Normal   : NORMAL0;
};

struct PS_INPUT{
    float4 Position         : SV_POSITION;
	float3 Normal	        : NORMAL0;
    float3 Reflect          : COLOR1;
};

PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;
    
    float4x4 wvp = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));
    float4 pos4 = float4(input.Position, 1.0);
	output.Position = mul(wvp, pos4);
	output.Normal = mul(WorldMatrix, input.Normal);

    //float3 eye_dir = normalize(input.Position.xyz - ViewPosition.xyz);
    //output.Reflect = reflect(eye_dir, -input.Normal.xyz);
    output.Reflect = input.Position;

    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET{
    float4 cube_map_color = CubeMap.Sample(Sampler0, input.Reflect);
    return cube_map_color;
}