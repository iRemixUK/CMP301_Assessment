// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 directionalAmbient;
    float4 directionalDiffuse;
    float3 direction;
    float padding;
    
    float4 dynamicAmbient;
    float4 dynamicDiffuse;
    float3 dynamicPosition;
    float padding2;
    
    float4 fixedAmbient;
    float4 fixedDiffuse;
    float3 fixedPosition;
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

float4 finalLighting(InputType input)
{
    float constantFactor = 0.5;
    float linearFactor = 0.125;
    float quadraticFactor = 0.0;
    float dist = 2;

	// Calculate attenuation
    float attenuation = 1 / (constantFactor + (linearFactor * dist) + (quadraticFactor * pow(dist, 2)));
    
    float4 directionalLight = calculateLighting(direction, input.normal, directionalDiffuse * attenuation);
    
    // Setup light colour for dynamic point light
    float3 dynamicLightVector = normalize(dynamicPosition - input.worldPosition);
    float4 dynamicLight = calculateLighting(-dynamicLightVector, input.normal, dynamicDiffuse * attenuation);
    
    // Setup light colour for dynamic point light
    float3 fixedLightVector = normalize(fixedPosition - input.worldPosition);
    float4 fixedLight = calculateLighting(-fixedLightVector, input.normal, fixedDiffuse * attenuation);
    
    return directionalLight + dynamicLight + fixedLight;
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
   
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    textureColour = texture0.Sample(sampler0, input.tex);
   
    
    lightColour = finalLighting(input);
  //  lightColour.a = 0.5;
    return (lightColour * textureColour) * float4(0.5,0.5, 0.5, 0.5);
}