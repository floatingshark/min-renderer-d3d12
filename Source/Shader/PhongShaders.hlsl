cbuffer wvp : register(b0){
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

cbuffer light : register(b1){
    float4 LightPos;
    float4 ViewPos;
};

Texture2D<float4> Tex : register(t0);
SamplerState Samp : register(s0);

struct VS_INPUT{
    float3 Position : POSITION;
    float4 Color    : COLOR;
	float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

struct PS_INPUT{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
	float3 Normal	: NORMAL;
    float4 PositionW: POSITION;
};

PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;
    
    float4x4 wvp = mul(View, World);
    wvp = mul(Projection, wvp);

    float4 pos4 = float4(input.Position, 1.0);

	output.Position = mul(wvp, pos4);
    output.Color = input.Color;
	output.Normal = mul(World, input.Normal);
    output.PositionW = mul(World, pos4);

    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET{

    float3 V = normalize(ViewPos - input.PositionW.xyz);
    float3 L = normalize(LightPos - input.PositionW.xyz);
    float3 H = normalize(V + L);
    
    float4 ambient = float4(0.5, 0.5, 0.5, 1.0);
    float diffuse = clamp(dot(input.Normal, L), 0.0, 1.0) * 0.5;
    float specular = pow(clamp(dot(input.Normal, H), 0.0, 1.0), 50.0);

    float4 surf_color = float4(diffuse, diffuse, diffuse, 1.0) + float4(specular, specular, specular, 1.0) + ambient;
    surf_color *= input.Color;

    return surf_color;

    //return float4(1.0, 1.0, 1.0, 1.0);
    //return float4(LightPos, 1.0);
    //return float4(ViewPos[0], 0.0, 0.0, 1.0);
    //return float4(input.Normal, 1.0);
}