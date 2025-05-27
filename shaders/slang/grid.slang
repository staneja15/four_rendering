struct VSOutput {
    float4 position : SV_Position;
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
VSOutput vertex_main(float2 vertex_position) {
    VSOutput output = {};

    float4 world_position = mul(model.model, float4(vertex_position.x, 1.0, vertex_position.y, 1.0));  // vertex_position.y is used on the z axis as we have a 2D array of x and z positions as input.
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    output.position = clip_space;
    
    return output;
}

[shader("fragment")]
float3 fragment_main() {
    return {0.0, 0.5, 0.1};
}
