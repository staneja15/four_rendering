struct VSOutput {
    float4 position : SV_Position;
    float3 color;
};

struct Model {
    float4x4 model;
};

struct ViewProj {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
ConstantBuffer<Model> model;

[shader("vertex")]
VSOutput vertex_main(float2 vertex_position, float3 color) {
    VSOutput output = {};

    float4 world_position = mul(model.model, float4(vertex_position, 1.0, 1.0));
    // float4 world_position = float4(vertex_position, 1.0, 1.0);
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    output.position = clip_space;
    output.color = color;
    
    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    return input.color;
}