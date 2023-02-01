Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture : register(t1);

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 directionalAmbient;
    float4 directionalDiffuse;
    float3 direction;
    float padding;
    
    float4 pointLight1Ambient;
    float4 pointLight1Diffuse;
    float3 pointLight1Position;
    float padding2;
    
    float4 pointLight2Ambient;
    float4 pointLight2Diffuse;
    float3 pointLight2Position;
    float padding3;
    
    float toggleNormals;
    float3 padding4;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float4 lightViewPos : TEXCOORD2;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return true;
    }
    return false;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 finalLighting(InputType input)
{
    // Variables needed for attenuation
    float constantFactor = 0.5;
    float linearFactor = 0.125;
    float quadraticFactor = 0.0;
    float dist = 2;

	// Calculate attenuation
    float attenuation = 1 / (constantFactor + (linearFactor * dist) + (quadraticFactor * pow(dist, 2)));
    
    // Setup the light colour for the directional light 
    float4 directionalLight = calculateLighting(-direction, input.normal, directionalDiffuse * attenuation);
    
    // Setup light colour for point light 1 
    float3 pointLight1Vector = normalize(pointLight1Position - input.worldPosition);
    float4 pointLight1 = calculateLighting(pointLight1Vector, input.normal, pointLight1Diffuse * attenuation);
    
    // Setup light colour for point light 2
    float3 pointLight2Vector = normalize(pointLight2Position - input.worldPosition);
    float4 pointLight2 = calculateLighting(pointLight2Vector, input.normal, pointLight2Diffuse * attenuation);
    
    // Add all light colours together and return 
    return directionalLight + pointLight1 + pointLight2;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
   
	// Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.lightViewPos);
	
    // Shadow test. Is or isn't in shadow
    if (hasDepthData(pTexCoord))
    {
        // Has depth map data
        if (isInShadow(depthMapTexture, pTexCoord, input.lightViewPos, shadowMapBias))
        {
            // is NOT in shadow, therefore light
            colour = finalLighting(input);
        }
    }
   
    // Add ambient to the colour
    colour = saturate(colour + directionalAmbient + pointLight1Ambient + pointLight2Ambient);
    
    // Determine whether to render normals or not
    if (toggleNormals == 1)
    {
         return float4(input.normal, 1);
    }
    else
    {
        return saturate(colour * textureColour);
    }
}