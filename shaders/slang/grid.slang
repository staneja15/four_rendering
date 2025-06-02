struct VSOutput {
    float4 position : SV_Position;
    float3 color;
};

struct VSInput {
    // Vertex data
    [[vk::location(0)]] float2 vertex_position;
    [[vk::location(1)]] uint32_t index;
    
    // Instance data
    [[vk::location(2)]] float4 transform0;
    [[vk::location(3)]] float4 transform1;  
    [[vk::location(4)]] float4 transform2;
    [[vk::location(5)]] float4 transform3;
    [[vk::location(6)]] float3 instance_color;

    // Shader draw data
    uint instance_id : SV_InstanceID;
}

struct ViewProj {
    float4x4 view;
    float4x4 proj;
};

struct StorageBufferInfo {
    uint32_t MAX_ELEVATION_IDX;
}

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
StructuredBuffer<float> height_data;

[[vk::binding(2, 0)]]
ConstantBuffer<StorageBufferInfo> height_info;

[shader("vertex")]
VSOutput vertex_main(VSInput input) {
    uint instance_offset = 10000 * input.instance_id;
    uint idx = min(input.index + instance_offset, height_info.MAX_ELEVATION_IDX);
    VSOutput output = {};

    float4x4 model = {
        input.transform0,
        input.transform1,
        input.transform2,
        input.transform3
    };

    model = transpose(model);

    float4 world_position = mul(model, float4(input.vertex_position.x, height_data[idx] / 100, input.vertex_position.y, 1.0));  // vertex_position.y is used on the z axis as we have a 2D array of x and z positions as input.
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    output.position = clip_space;
    output.color = input.instance_color;
    
    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    return input.color * 0.3 + float3(0.01, 0.37, 0.19) * 0.7;
}
