struct VSOutput {
    float4 position : SV_Position;
};

struct ViewProj {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
StructuredBuffer<float> height_data;

[shader("vertex")]
VSOutput vertex_main(float2 vertex_position, uint32_t index) {
    VSOutput output = {};

    float4 world_position = float4(vertex_position.x, height_data[index] / 100, vertex_position.y, 1.0);  // vertex_position.y is used on the z axis as we have a 2D array of x and z positions as input.
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    output.position = clip_space;
    
    return output;
}

[shader("fragment")]
float3 fragment_main() {
    return {0.0, 0.5, 0.1};
}
