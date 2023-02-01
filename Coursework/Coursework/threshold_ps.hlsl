Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float threshold;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 colour;
    float4 textureColour;

	// Initialize the colour to black.
    colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    textureColour = shaderTexture.Sample(SampleType, input.tex);
    
    // Luma conversion
    float brightness = (textureColour.r * 0.2126) + (textureColour.g * 0.7152) + (textureColour.b * 0.722);
    
    // Extract brightness of the pixel
    if (brightness > threshold)
    {
        colour += shaderTexture.Sample(SampleType, input.tex);
    }
    else
    {
        colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
  
	// Set the alpha channel to one.
    colour.a = 1.0f;

    return colour;
}
