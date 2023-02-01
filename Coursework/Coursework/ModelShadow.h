#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;
class ModelShadow : public BaseShader
{
private:
	// Buffer to send matrices to the vertex shader
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
	};

public:

	ModelShadow(ID3D11Device* device, HWND hwnd);
	~ModelShadow();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap, Light* light, Light* light2, Light* light3);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

