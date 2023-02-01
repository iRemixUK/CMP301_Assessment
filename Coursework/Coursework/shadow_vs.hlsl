Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
    float height; 
    float3 padding;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float4 lightViewPos : TEXCOORD2;
};

float GetHeight(float2 uv)
{
    // Sample displacement map
    float textureColour;
    textureColour = texture0.SampleLevel(sampler0, uv, 0).r;
    
    // Multiply displacement map by height
    return textureColour * height;
}

float3 calcNormal(float2 uv)
{
    // Sidestep needed to get vertices in other directions
    float sidestep = 0.01;
    
    // Get left, right, up and down vertices
    float2 leftTex = uv + float2(-sidestep, 0);
    float2 rightTex = uv + float2(sidestep, 0);
    float2 downTex = uv + float2(0, sidestep);
    float2 upTex = uv + float2(0, -sidestep);
	
    // Get the vertices height
    float leftY = GetHeight(leftTex);
    float rightY = GetHeight(rightTex);
    float downY = GetHeight(downTex);
    float upY = GetHeight(upTex);
    
    // Calculate tangent and bitangent
    float3 tangent = normalize(float3( 2.f * (sidestep), (rightY - leftY), 0.0f));
    float3 bitan = normalize(float3(0.0f, (upY - downY), 2.f * -sidestep));
    
    // Cross product to find normal
    float3 normal = cross(tangent, bitan);
    return normalize(normal);
}


OutputType main(InputType input)
{
    OutputType output;
    
    // Set y position to height generated by displacement map
    input.position.y += GetHeight(input.tex);
    
    // Calculate the normals
    input.normal = calcNormal(input.tex);
    
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
    output.lightViewPos = mul(input.position, worldMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);

    output.tex = input.tex;
    output.worldPosition = mul(input.position, worldMatrix).xyz;
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}