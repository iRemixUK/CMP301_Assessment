#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;
class WaterShadow : public BaseShader
{
	// Buffer to send matrices to vertex buffer
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	// Buffer to send the light variables
	struct LightBufferType
	{
		XMFLOAT4 directionalAmbient;
		XMFLOAT4 directionalDiffuse;
		XMFLOAT3 direction;
		float padding;

		XMFLOAT4 pointLight1Ambient;
		XMFLOAT4 pointLight1Diffuse;
		XMFLOAT3 pointLight1Position;
		float padding2;

		XMFLOAT4 pointLight2Ambient;
		XMFLOAT4 pointLight2Diffuse;
		XMFLOAT3 pointLight2Position;
		float padding3;

		float toggleNormals;
		XMFLOAT3 padding4;
	};

	// Buffer to send the water variables to the vertex shader for manipulation
	struct WaterBuffer
	{
		float time;
		float amplitude;
		float frequency;
		float speed;
	};

public:
	WaterShadow(ID3D11Device* device, HWND hwnd);
	~WaterShadow();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap, Light* dynamicPointLight, Light* fixedPointLight, Light* directionalLight, float time, float amplitude, float frequency, float speed, bool toggleNormals);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* waterBuffer;

};

