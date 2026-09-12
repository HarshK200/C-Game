cbuffer FrameUniforms : register(b0)
{
    column_major float4x4 VIEW;
    column_major float4x4 PROJECTION;
};
cbuffer EntityUniforms : register(b1)
{
    column_major float4x4 MODEL;
    float4 UV_MIN_MAX;
};

// input struct
struct vs_in {
    float3 pos : POS0;
    float2 uv  : TEXCOORD0;
};
// output struct
struct vs_out {
    float4 pos : SV_POSITION; // MUST BE EXPORTED
    float2 uv  : TEXCOORD0;
};


vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory

    float4 position = float4(input.pos, 1.0f);

    position = mul(MODEL, position);
    position = mul(VIEW, position);
    position = mul(PROJECTION, position);

    output.pos = position;
    output.uv = lerp(UV_MIN_MAX.xy, UV_MIN_MAX.zw, input.uv);

    return output;
}



Texture2D       SpriteTex    : register(t0);
SamplerState    PointSampler : register(s0);

float4 ps_main(vs_out input) : SV_TARGET {
    float4 albedo = SpriteTex.Sample(PointSampler, input.uv);
    if(albedo.w < 0.01) {
        discard;
    }

    return albedo; // MUST be a RGBA value in range 0..1
}
