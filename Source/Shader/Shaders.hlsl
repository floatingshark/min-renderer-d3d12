cbuffer cbBuffer : register(b0){
    //float4x4 WVP;
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

struct VS_INPUT{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

// (VS_OUTPUT)
struct PS_INPUT{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};


PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;

    float4 pos4 = float4(input.Position, 1.0);
    pos4 = mul(World, pos4);
    pos4 = mul(View, pos4);
    pos4 = mul(Projection, pos4);

	output.Position = pos4;
    output.Color = input.Color;

    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET{
    return input.Color;
}