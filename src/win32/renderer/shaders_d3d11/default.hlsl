// input struct
struct vs_in {
    float3 pos : POS0;
};
// output struct
struct vs_out {
    float4 pos : SV_POSITION; // required output of VS
};


vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory
    output.pos = float4(input.pos, 1.0);
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET {
    return float4(0.6824f, 0.8392f, 0.9451f, 1.0f); // MUST be a RGBA value
}
