struct VSOutput {
    float4 position : SV_Position;
    float3 color;
};

struct Camera {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0)]]
ConstantBuffer<Camera> camera;

[shader("vertex")]
VSOutput vertex_main(float2 vertex_position, float3 color) {
    VSOutput output = {};

    float4 world_position = mul(camera.model, float4(vertex_position, 1.0, 1.0));
    float4 view_position = mul(camera.view, world_position);
    float4 clip_space = mul(camera.proj, view_position);

    output.position = clip_space;
    output.color = color;
    
    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    return input.color;
}