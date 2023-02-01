// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"

#include "VerticalBlurShader.h"
#include "HorizontalBlurShader.h"
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "ModelDepth.h"
#include "ModelShadow.h"
#include "Threshold.h"
#include "WaterShadow.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();

	void firstPass();

	void depthPass();

	void thresholdPass();
	void verticalBlur();
	void horizontalBlur();

	void finalPass();
	void gui();

	void initLights();
	void initShaders(HWND hwnd);
	void initMeshes(int screenWidth, int screenHeight);

private:
	// Shaders
	ShadowShader* shadowShader;
	DepthShader* depthShader;
	ModelDepth* modelDepth;
	ModelShadow* modelShadow;
	WaterShadow* waterShadow;
	Threshold* threshold;
	TextureShader* textureShader;
	VerticalBlurShader* verticalBlurShader;
	HorizontalBlurShader* horizontalBlurShader;

	AModel* model;

	ShadowMap* shadowMap;

	// Meshes
	PlaneMesh* heightMap;
	PlaneMesh* water;

	OrthoMesh* orthoMesh;
	
	// Lights
	Light* PointLight1;
	Light* PointLight2;
	Light* directionalLight;

	// Water variables
	float amplitude;
	float frequency;
	float speed;
	float time;

	// Render to texture
	RenderTexture* renderTexture;
	RenderTexture* thresholdTexture;
	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;

	// Position of lights
	XMFLOAT3 PointLight1Pos;
	XMFLOAT3 PointLight2Pos;

	// Colour of lights
	float PointLight1DiffuseColours[4] = { 1.f, 1.f, 1.f, 1.f };
	float PointLight2DiffuseColours[4] = { 1.f, 1.f, 1.f, 1.f };
	float directionalDiffuseColours[4] = { 1.f, 1.f, 1.f, 1.f };

	// Directional light's direction
	float directionalDirection[3] = { 0.0f, -0.477f, 1.f };

	// Location of teapot
	float teapotLocation[3] = { 0.0f, 37.f, 52.f };

	// Bloom variables
	bool toggleBloom;
	float bloomThreshold;

	// Toggle normals for height and water
	bool toggleHeightNormals;
	bool toggleWaterNormals;

	// Height map height
	float height = 8.f;

	// Colour of background
	float backgroundColour[4] = { 0.f, 0.f, 1.f, 1.f };

	Model* modelMgr;
};

#endif