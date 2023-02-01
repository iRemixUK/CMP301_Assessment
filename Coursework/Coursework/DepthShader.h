// Colour shader.h
// Simple shader example.
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{

public:

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float height);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	// Buffer to send the height of the displacement map
	struct heightBufferType
	{
		float height;
		XMFLOAT3 padding;
	};

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* heightBuffer;
	ID3D11SamplerState* sampleState;
};
