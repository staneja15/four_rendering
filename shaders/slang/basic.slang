struct VSOutput {
    float4 position : SV_Position;
    float3 color;
    float2 UV : TEXCOORD0;
};

struct ViewProj {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
Sampler2D texture_color : register(t1);

[shader("vertex")]
VSOutput vertex_main(float2 vertex_position, float3 color, float2 UV) {
    VSOutput output = {};

    float4 world_position = float4(vertex_position, -5.0, 1.0);  // Doesn't require a model mat here as it would just be a glm::mat4(1.0f) anyway.
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    output.position = clip_space;
    output.color = color;
    output.UV = UV;
    
    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    float3 tex = texture_color.Sample(input.UV).xyz;
    float3 final_color = lerp(tex, input.color, 0.3);
    return final_color;
}