cbuffer scene : register(b0){
    float4x4 View;
    float4x4 Projection;
    float4 ViewPos;
    float4 LightPos;
    float4 LightAmb;
    float LightInt;
};

cbuffer local : register(b1){
    float4x4 World;
    bool UseTexture;
};

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
    float2 UV       : TEXCOORD;
    float4 PositionWorld: POSITION;
};

Texture2D<float4> Tex0 : register(t0);
SamplerState Samp0 : register(s0);

PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;
    
    float4x4 wvp = mul(View, World);
    wvp = mul(Projection, wvp);

    float4 pos4 = float4(input.Position, 1.0);

	output.Position = mul(wvp, pos4);
    output.Color = input.Color;
	output.Normal = mul(World, input.Normal);
    output.UV = input.UV;
    output.PositionWorld = mul(World, pos4);

    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET{

    float3 V = normalize(ViewPos - input.PositionWorld.xyz);
    float3 L = normalize(LightPos - input.PositionWorld.xyz);
    float3 H = normalize(V + L);
    
    float4 ambient = LightAmb;
    float diffuse = clamp(dot(input.Normal, L), 0.0, 1.0);
    float specular = pow(clamp(dot(input.Normal, H), 0.0, 1.0), 50.0);

    float4 surf_color = float4(diffuse, diffuse, diffuse, 1.0) + float4(specular, specular, specular, 1.0);
    float4 tex_color = UseTexture ? Tex0.Sample(Samp0, input.UV) : input.Color;
    surf_color *= LightInt;
    surf_color *= tex_color;
    surf_color += ambient;

    return surf_color;

    //return float4(1.0, 1.0, 1.0, 1.0);
    //return float4(LightPos, 1.0);
    //return float4(ViewPos[0], 0.0, 0.0, 1.0);
    //return float4(input.Normal, 1.0);

    //return float4(input.UV, 0.0, 1.0);
    //return Tex0.Sample(Samp0, input.UV);
}