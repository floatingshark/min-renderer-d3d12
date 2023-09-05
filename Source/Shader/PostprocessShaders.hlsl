Texture2D<float4> RenderedTexture      : register(t0);
SamplerState 	  Sampler0             : register(s0);

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
	float2 window_size = (800, 600);

	// Mosaic Effect
	float block_size = 4;
	float2 uv = input.UV * window_size;
	uv /= block_size;
	uv = floor(uv) * block_size;
	uv /= window_size;

	return RenderedTexture.Sample(Sampler0, uv);
}