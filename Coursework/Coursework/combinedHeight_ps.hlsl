// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D heightMap : register(t0);
Texture2D heightTexture : register(t1);
Texture2D blur : register(t2);
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
    float width;
};


struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculateSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
    float3 halfway = normalize(lightDirection);
    //+viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
    return saturate(specularColour * specularIntensity);
}

float4 main(InputType input, InputType input2, InputType input3) : SV_TARGET
{
	float4 textureColour = heightTexture.Sample(sampler0, input2.tex);
	float4 blurColour = blur.Sample(sampler0, input3.tex);
	
	
    float4 textureColour2;
    float4 finalTex;
    float4 lightColour;
    
    float constantFactor = 0.5;
    float linearFactor = 0.125;
    float quadraticFactor = 0.0;
    float dist = 2;

	// Calculate attenuation
    float attenuation = 1 / (constantFactor + (linearFactor * dist) + (quadraticFactor * pow(dist, 2)));
    
    
    // Setup light colour for first point light
    float3 dynamicLightVector = normalize(position1 - input2.worldPosition);
    float4 dynamicPointLight = ambient[0] + calculateLighting(dynamicLightVector, input.normal, diffuse[0] * attenuation);
	
	// Setup light colour for second point light
    float3 fixedLightVector = normalize(position2 - input2.worldPosition);
    float4 fixedPointLight = ambient[1] + calculateLighting(fixedLightVector, input.normal, diffuse[1] * attenuation);
    
     // Setup light colour for directional light
    float4 directionalLight = calculateLighting(-direction, input.normal, diffuse[2] * attenuation);
	
    // Blends textures together to allow grass to change colour gradually and the snow to be bright
    textureColour = textureColour * (0.5f, 0.5f, 0.5f, 0.5f);
    textureColour2 = textureColour2 * (0.5f, 0.5f, 0.5f, 0.5f);
    finalTex = (textureColour + textureColour2);
   
    //lightColour = (dynamicPointLight + fixedPointLight);
    lightColour = directionalLight;
    
    float weight0, weight1, weight2;
    float4 colour;

	// Create the weights that each neighbor pixel will contribute to the blur.
    weight0 = 0.4062f;
    weight1 = 0.2442f;
    weight2 = 0.0545f;

	// Initialize the colour to black.
    colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float brightness = dot(lightColour.rgb, float3(0.2126, 0.7152, 0.0722));
  //  float brightness = (lightColour.r + lightColour.g + lightColour.b) / 3.0f;
    
    if (lightColour.r > 0.9 || lightColour.g > 0.9 || lightColour.b > 0.9)
    {
        //return textureColour * blurColour * brightness * lightColour;
        
       // return blurColour * lightColour * brightness;
        return blurColour * lightColour;
    }
    else
    {
        return textureColour * lightColour;
    }
    

}




