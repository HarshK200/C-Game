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
    output.pos = float4(input.pos, 1.0f);
    output.uv = input.uv;
    return output;
}



Texture2D       SpriteTex    : register(t0);
SamplerState    PointSampler : register(s0);

float4 ps_main(vs_out input) : SV_TARGET {
    float4 albedo = SpriteTex.Sample(PointSampler, input.uv);
    if(albedo.w < 0.01) {
        return float4(0.5f, 0.0f, 0.0f, 1.0f);
    }

    return albedo; // MUST be a RGBA value in range 0..1
}
