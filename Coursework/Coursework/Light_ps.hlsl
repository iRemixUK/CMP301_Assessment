// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 ambient[2];
    float4 diffuse[3];
    float3 position1;
    float padding;
    float3 position2;
    float padding2;
    float3 direction;
    float padding3;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, -lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
    
    float constantFactor = 0.5;
    float linearFactor = 0.125;
    float quadraticFactor = 0.0;
    float dist = 2;

	// Calculate attenuation
    float attenuation = 1 / (constantFactor + (linearFactor * dist) + (quadraticFactor * pow(dist, 2)));

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    textureColour = texture0.Sample(sampler0, input.tex);
    
    // Setup light colour for first point light
    float3 dynamicLightVector = normalize(position1 - input.worldPosition);
    float4 dynamicPointLight = calculateLighting(-dynamicLightVector, input.normal, diffuse[0] * attenuation);
	
	// Setup light colour for second point light
    float3 fixedLightVector = normalize(position2 - input.worldPosition);
    float4 fixedPointLight = calculateLighting(-fixedLightVector, input.normal, diffuse[1] * attenuation);
    
    // Setup light colour for directional light
    float4 directionalLight = calculateLighting(direction, input.normal, diffuse[2] * attenuation);
    
    lightColour = dynamicPointLight + fixedPointLight + directionalLight;
    float4 normalColour = float4(input.normal.x, input.normal.y, input.normal.z, 1.f);
    
    return ( lightColour );

}