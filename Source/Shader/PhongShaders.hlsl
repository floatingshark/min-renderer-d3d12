cbuffer scene : register(b0){
    float4x4    ViewMatrix;
    float4x4    ProjectionMatrix;
    float4      ViewPosition;
    float4      LightPosition;
    float4      LightAmbient;
    float4x4    LightViewMatrix;
    float       LightIntensity;
};

cbuffer local : register(b1){
    float4x4    WorldMatrix;
    bool        UseTexture;
    float       Specular;
};

struct VS_INPUT{
    float3 Position : POSITION;
    float4 Color    : COLOR;
	float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

struct PS_INPUT{
    float4 Position         : SV_POSITION;
    float4 Color            : COLOR;
	float3 Normal	        : NORMAL;
    float2 UV               : TEXCOORD;
    float4 PositionWorld    : POSITION;
    float4 PositionShadow	: POSITION_SM;
};

Texture2D<float4> Tex0      : register(t0);
Texture2D<float4> ShadowMap : register(t1);
SamplerState Samp0          : register(s0);
SamplerState Samp1          : register(s1);

PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;
    
    float4x4 wvp = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));
    float4 pos4 = float4(input.Position, 1.0);

	output.Position = mul(wvp, pos4);
    output.Color = input.Color;
	output.Normal = mul(WorldMatrix, input.Normal);
    output.UV = input.UV;
    output.PositionWorld = mul(WorldMatrix, pos4);

    float4 pos_shadow = mul(WorldMatrix, input.Position);
    pos_shadow = mul(LightViewMatrix, pos_shadow);
    pos_shadow = mul(ProjectionMatrix, pos_shadow);
    pos_shadow.xyz = pos_shadow.xyz / pos_shadow.w;

    output.PositionShadow.x = (1.0 + pos_shadow.x) / 2.0f;
	output.PositionShadow.y = (1.0 - pos_shadow.y) / 2.0f;
	output.PositionShadow.z = pos_shadow.z;

    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET{

    float3 V = normalize(ViewPosition - input.PositionWorld.xyz);
    float3 L = normalize(LightPosition - input.PositionWorld.xyz);
    float3 H = normalize(V + L);
    
    float4 ambient = LightAmbient;
    float diffuse = clamp(dot(input.Normal, L), 0.0, 1.0);
    float specular = pow(clamp(dot(input.Normal, H), 0.0, 1.0), Specular);

    float4 surf_color = float4(diffuse, diffuse, diffuse, 1.0) + float4(specular, specular, specular, 1.0);
    float4 tex_color = UseTexture ? Tex0.Sample(Samp0, input.UV) : input.Color;
    surf_color *= LightIntensity;
    surf_color *= tex_color;
    surf_color += ambient;
    
    float shadow_map = ShadowMap.Sample(Samp1, input.PositionShadow.xy);
    float shadow_alpha = (input.PositionShadow.z - 0.005f < shadow_map) ? 1.0f : 0.0f;
    surf_color *= shadow_alpha;

    return surf_color;

    //return float4(1.0, 1.0, 1.0, 1.0);
    //return float4(LightPosition, 1.0);
    //return float4(ViewPosition[0], 0.0, 0.0, 1.0);
    //return float4(input.Normal, 1.0);

    //return float4(input.UV, 0.0, 1.0);
    //return Tex0.Sample(Samp0, input.UV);
    //return ShadowMap.Sample(Samp0, input.UV);
}

// For Shadow Mapping
float4 VSShadowMap(VS_INPUT input) : SV_POSITION{

	float4 pos = float4(input.Position, 1.0f);
	pos = mul(WorldMatrix, pos);
	pos = mul(LightViewMatrix, pos);
    pos = mul(ProjectionMatrix, pos);

	return pos;
}