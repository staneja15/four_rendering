struct VSOutput {
    float4 position : SV_Position;
    float3 color;
};

struct VSInput {
    // Vertex data
    [[vk::location(0)]] float2 vertex_position;
    
    // Instance data
    [[vk::location(1)]] float4 transform0;
    [[vk::location(2)]] float4 transform1;  
    [[vk::location(3)]] float4 transform2;
    [[vk::location(4)]] float4 transform3;
    [[vk::location(5)]] float3 instance_color;

    // Shader draw data
    uint instance_id : SV_InstanceID;
    uint index       : SV_VertexID;
};

struct ViewProj {
    float4x4 view;
    float4x4 proj;
};

struct StorageBufferInfo {
    uint32_t MAX_ELEVATION_IDX;
    uint32_t INSTANCE_BUFFER_SIZE;
};

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
StructuredBuffer<float> height_data;

[[vk::binding(2, 0)]]
ConstantBuffer<StorageBufferInfo> height_info;

[shader("vertex")]
VSOutput vertex_main(VSInput input) {
    // Get the index position for the height array
    uint instance_offset = height_info.INSTANCE_BUFFER_SIZE * input.instance_id;
    uint idx = min(input.index + instance_offset, height_info.MAX_ELEVATION_IDX);

    // Calculate the MVP
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


    // Calculate Normal:
    // U = h(x + d, y) - h(x, y)
    // V = h(x, y + d) - h(x, y)
    // Sources:
    //  https://www.reddit.com/r/opengl/comments/k1gvgt/how_to_calculate_pervertex_normals_when_all_i/
    //  https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    uint d = 1;
    float3 U = float3(input.vertex_position.x, height_data[idx+d] - height_data[idx], input.vertex_position.y);
    float3 V = float3(input.vertex_position.x, height_data[idx+uint(sqrt(height_info.INSTANCE_BUFFER_SIZE)*d)] - height_data[idx], input.vertex_position.y);
    float3 normal = normalize(U * V);

    // Set the light direction and colour
    float3 light_dir = normalize(float3(-0.7f, -5.0f, -0.8f));
    float3 light_color = float3(0.89, 0.85, 0.53);
    float diff = max(0.0, dot(normal, -light_dir));

    // Bind the output data
    VSOutput output = {};
    output.position = clip_space;
    output.color = (diff * light_color) * 0.5 + input.instance_color * 0.5;
    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    return input.color;  // + float3(0.01, 0.37, 0.19) * 0.7;
}
