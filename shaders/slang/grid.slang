struct VSOutput {
    float4 position : SV_Position;
    float3 color;
    float3 normal;
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
    float d;  // Distance between each height index (x, z)
};

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProj> vp;

[[vk::binding(1, 0)]]
StructuredBuffer<float> height_data;

[[vk::binding(2, 0)]]
ConstantBuffer<StorageBufferInfo> height_info;

float3 calculate_normal(float3 p1, float3 p2, float3 p3) {
    // Calculate Normal Positions
    // Sources:
    //  https://www.reddit.com/r/opengl/comments/k1gvgt/how_to_calculate_pervertex_normals_when_all_i/
    //  https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    //  https://math.stackexchange.com/questions/2461485/find-a-normal-vector-using-cross-product

    float3 U = p2 - p1;
    float3 V = p3 - p1;
    return normalize(cross(U, V));
}

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

    float4 world_position = mul(model, float4(input.vertex_position.x, height_data[idx], input.vertex_position.y, 1.0));  // vertex_position.y is used on the z axis as we have a 2D array of x and z positions as input.
    float4 view_position = mul(vp.view, world_position);
    float4 clip_space = mul(vp.proj, view_position);

    // Calculate normal
    float d = height_info.d;
    uint width = uint(sqrt(height_info.INSTANCE_BUFFER_SIZE));
    float3 p1 = float3(input.vertex_position.x,     height_data[idx]      , input.vertex_position.y    );
    float3 p2 = float3(input.vertex_position.x + d, height_data[idx+1]    , input.vertex_position.y    );
    float3 p3 = float3(input.vertex_position.x    , height_data[idx+width], input.vertex_position.y + d);
    float3 normal = calculate_normal(p1, p2, p3);

    // Bind the output data
    VSOutput output = {};
    output.position = clip_space;
    output.color = input.instance_color;
    output.normal = normal;

    return output;
}

[shader("fragment")]
float3 fragment_main(VSOutput input) {
    float3 light_dir = normalize(-float3(20.0f, 50, 20.0f));  // todo: This should be in a uniform buffer

    // Ambient
    float3 ambient = 0.05f * input.color;

    // Diffuse
    float3 light_diffuse = float3(0.8f);
    float diff = max(dot(input.normal, light_dir), 0.0);
    float3 diffuse = light_diffuse * diff * input.color;
    
    float3 frag_color = ambient + diffuse;
    return frag_color;
}
