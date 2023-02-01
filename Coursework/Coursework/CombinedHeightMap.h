#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;
class CombinedHeightMap : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambient[2];
		XMFLOAT4 diffuse[3];
		XMFLOAT3 position1;
		float padding;
		XMFLOAT3 position2;
		float padding2;
		XMFLOAT3 direction;
		float padding3;
	};
	
	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:
	CombinedHeightMap(ID3D11Device* device, HWND hwnd);
	~CombinedHeightMap();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, Light* dynamicPointLight, Light* fixedPointLight, Light* directionalLight);
private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
};

