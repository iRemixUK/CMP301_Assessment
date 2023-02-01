// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
	float3 viewVector : TEXCOORD2;
};

float GetHeight(float2 uv)
{
	float textureColour;
	textureColour = texture0.SampleLevel(sampler0, uv, 0);
	return textureColour * 8;
}

float3 calcNormal(float3 inputNormal, float2 uv)
{
	float3 normal = float3(0, 0, 0);
	//distance to go along the uv coords 
	float distance = 0.005f;
	// get the left, right, up,down and middle coords 
	
	float2 LRUDMcoords[5] =
	{
		uv + float2(-distance, 0),
		uv + float2(distance, 0),
		uv + float2(0, distance),
		uv + float2(0, -distance),
		uv
	};
	float3 LRUDMpositions[5] =
	{
		float3(-distance, 0, 0),
		float3(distance, 0, 0),
		float3(0, 0, -distance),
		float3(0, 0, distance),
		float3(0, 0, 0),
	};

	for (int i = 0; i < 5; i++)
	{
		LRUDMpositions[i] = float3(LRUDMpositions[i].x, GetHeight(uv), LRUDMpositions[i].z);
	}

	for (int j = 0; j < 5; j++)
	{
		LRUDMpositions[j] = normalize(LRUDMpositions[j] - LRUDMpositions[4]);
	}
	
	float3 normA = cross(LRUDMpositions[0], LRUDMpositions[3]);
	float3 normb = cross(LRUDMpositions[1], LRUDMpositions[2]);

	normal = normA;
	
	return normalize(normal);
}
OutputType main(InputType input)
{
	OutputType output;
	input.position.y += GetHeight(input.tex);
	input.normal = calcNormal(input.normal, input.tex);
   
	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);
    
	output.worldPosition = mul(input.position, worldMatrix).xyz;
	//output.viewVector = cameraPosition.xyz - output.worldPosition.xyz;
	//output.viewVector = normalize(output.viewVector);

	return output;
}