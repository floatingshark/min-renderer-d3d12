Texture2D<float4> RenderedTexture      : register(t0);
SamplerState Sampler0           : register(s0);

struct VS_INPUT{
};

struct PS_INPUT{
    float4 Position         : SV_POSITION;
    float2 UV               : TEXCOORD;
};

PS_INPUT VSMain(uint id : SV_VERTEXID){
    PS_INPUT output;

	float x = (float)(id % 2) * 2 - 1.0;
	float y = (float)(id / 2) *-2 + 1.0;
	output.Position = float4(x, y, 0, 1);

	float tx = (id % 2);
	float ty = (id / 2);
	output.UV = float2(tx, ty);

    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET{
	return RenderedTexture.Sample(Sampler0, input.UV);
    //return float4(1.0, 0.0, 0.0, 1.0);
}