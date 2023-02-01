#pragma once

#include "BaseShader.h"
#include "DXF.h"
using namespace std;
using namespace DirectX;

class TextureShader : public BaseShader
{
public:
	TextureShader(ID3D11Device* device, HWND hwnd);
	~TextureShader();


	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2, bool toggleBloom);

private:
	// Buffer to send the toggle bloom variable
	struct LightBufferType
	{
		float toggleBloom;
		XMFLOAT3 padding;
	};

	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
};

