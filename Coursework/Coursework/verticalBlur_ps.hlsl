Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenHeight;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float weight0, weight1, weight2, weight3, weight4, weight5;
    float4 colour;

	// Create the weights that each neighbor pixel will contribute to the blur.
    weight0 = 0.198596f;
    weight1 = 0.175713f;
    weight2 = 0.121703f;
    weight3 = 0.065984f;
    weight4 = 0.028002f;
    weight5 = 0.0093f;

    // Initialize the colour to black.
    colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float texelSize = 1.0f / screenHeight;
   
    // Add the vertical pixels to the colour by the specific weight of each. 
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, -5.0f)) * weight5;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, -4.0f)) * weight4;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, -3.0f)) * weight3;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, -2.0f)) * weight2;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, -1.0f)) * weight1;
    colour += shaderTexture.Sample(SampleType, input.tex) * weight0;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, 1.0f)) * weight1;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, 2.0f)) * weight2;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, 3.0f)) * weight3;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, 4.0f)) * weight4;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 0.0f, 5.0f)) * weight5;
    
    // Set the alpha channel to one.
    colour.a = 1.0f;

    return colour;
}

