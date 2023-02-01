// Horizontal blur shader handler
// Loads horizontal blur shaders (vs and ps)
// Passes screen width to shaders, for sample coordinate calculation
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class Threshold : public BaseShader
{
private:
	// Buffer to send the threshold
	struct ThresholdBufferType
	{
		float threshold;
		XMFLOAT3 padding;
	};

public:

	Threshold(ID3D11Device* device, HWND hwnd);
	~Threshold();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float threshold);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* screenSizeBuffer;
};