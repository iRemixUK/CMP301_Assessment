Texture2D texture0 : register(t0);
Texture2D normalTexture : register(t1);
SamplerState Sampler0 : register(s0);

cbuffer toggleBuffer : register(b0)
{
    bool toggleBloom;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
	// Sample both textures
    float4 bloomColor = texture0.Sample(Sampler0, input.tex);
    float4 sceneTexture = normalTexture.Sample(Sampler0,input.tex);

   // Determines whether to add bloom to final texture or not
   if (toggleBloom == 1)
    {
        float4 finalColour = bloomColor + sceneTexture;
        return finalColour;
    }
    else
    {
        return sceneTexture;
    }
}