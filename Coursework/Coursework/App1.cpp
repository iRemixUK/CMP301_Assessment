// CMP301 Assignment
// Aaron Crawford - 1900400
// Github - iRemixUK
#include "App1.h"

App1::App1()
{	
	// Start up
	heightMap = nullptr;
	water = nullptr;
	model = nullptr;

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initalise scene variables.
	// Load in textures
	textureMgr->loadTexture(L"height", L"res/heightmap.png");
	textureMgr->loadTexture(L"water", L"res/water_tex2.png");
	textureMgr->loadTexture(L"terrain", L"res/texture2.png");
	textureMgr->loadTexture(L"Brick", L"res/brick1.dds");

	// Initialise the meshes and shaders
	initMeshes(screenWidth, screenHeight);
	initShaders(hwnd);

	// Variables for defining shadow map
	// 4k resolution for shadow map
	int shadowmapWidth = 3840;
	int shadowmapHeight = 2160;

	int sceneWidth = 100;
	int sceneHeight = 100;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Initialise the required render textures
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth , screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	thresholdTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Initialise lights
	initLights();
	directionalLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	// Configure variables for water
	speed = 2.2f;
	frequency = 0.4f;
	amplitude = 0.4f;
	time = 0.f;

	// Bloom variables
	toggleBloom = true;
	bloomThreshold = 1.1f;

	// Set normal rendering to false on startup
	toggleWaterNormals = false;
	toggleHeightNormals = false;

	// Sets the camera position
	camera->setPosition(-36, 40, -16);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D objects.
	if (heightMap)
	{
		delete heightMap;
		heightMap = 0;
	}

	if (water)
	{
		delete water;
		water = 0;
	}

	if (model)
	{
		delete model;
		model = 0;
	}
}

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Increment time for water
	time += timer->getTime();

	// Update dynamic light position every frame, ensures that the ImGui sliders work
	PointLight1->setPosition(PointLight1Pos.x, PointLight1Pos.y, PointLight1Pos.z);
	PointLight2->setPosition(PointLight2Pos.x, PointLight2Pos.y, PointLight2Pos.z);

	// Update light colours every frame 
	PointLight1->setDiffuseColour(PointLight1DiffuseColours[0], PointLight1DiffuseColours[1], PointLight1DiffuseColours[2], PointLight1DiffuseColours[3]);
	PointLight2->setDiffuseColour(PointLight2DiffuseColours[0], PointLight2DiffuseColours[1], PointLight2DiffuseColours[2], PointLight2DiffuseColours[3]);
	directionalLight->setDiffuseColour(directionalDiffuseColours[0], directionalDiffuseColours[1], directionalDiffuseColours[2], directionalDiffuseColours[3]);
	directionalLight->setDirection(directionalDirection[0], directionalDirection[1], directionalDirection[2]);

	// Check if normals are rendering, if true turn off bloom 
	if (toggleHeightNormals == true || toggleWaterNormals == true)
	{
		toggleBloom = false;
	}

	result = render();
	if (!result)
	{
		return false;
	}
	return true;
}

bool App1::render()
{
	// Does a depth pass for the shadows 
	depthPass();
	
	// First pass to render scene
	firstPass();

	// Threshold pass for bloom
	thresholdPass();

	// Apply horizontal blur stage
	horizontalBlur();

	// Apply vertical blur to the horizontal blur stage
	verticalBlur();

	// Render final pass to frame buffer
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	directionalLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = directionalLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = directionalLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	// Renders height map
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	heightMap->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height"), height);
	depthShader->render(renderer->getDeviceContext(), heightMap->getIndexCount());

	// Move the model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotLocation[0], teapotLocation[1], teapotLocation[2]);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Render model
	model->sendData(renderer->getDeviceContext());
	modelDepth->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	modelDepth->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::firstPass()
{
	// Set the render target to be the render to texture.
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), backgroundColour[0], backgroundColour[1], backgroundColour[2], backgroundColour[3]);

	camera->update();
	// Get matrices needed
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX projMatrix = renderer->getProjectionMatrix(); 
	XMMATRIX viewMatrix = camera->getViewMatrix();	

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Render the height map which has textures and lighting applied. Shadows are calculated here
	heightMap->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projMatrix,
		textureMgr->getTexture(L"height"), textureMgr->getTexture(L"terrain"), shadowMap->getDepthMapSRV(),
		PointLight1, PointLight2, directionalLight, height, toggleHeightNormals);
	shadowShader->render(renderer->getDeviceContext(), heightMap->getIndexCount());

	// Move teapot model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotLocation[0], teapotLocation[1], teapotLocation[2]);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Render teapot model with shadows
	model->sendData(renderer->getDeviceContext());
	modelShadow->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projMatrix, textureMgr->getTexture(L"Brick"),
		shadowMap->getDepthMapSRV(), PointLight1, PointLight2, directionalLight);
	modelShadow->render(renderer->getDeviceContext(), model->getIndexCount());

	// Move water up and forward
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix *= XMMatrixTranslation(-50.0f, 1.3f, -10.0f);

	// Renders water with shadows
	renderer->setAlphaBlending(true); // Make water slightly transparent
	water->sendData(renderer->getDeviceContext());
	waterShadow->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projMatrix,
		textureMgr->getTexture(L"water"), shadowMap->getDepthMapSRV(),
		PointLight1, PointLight2, directionalLight, time, amplitude, frequency, speed, toggleWaterNormals);
	waterShadow->render(renderer->getDeviceContext(), water->getIndexCount());
	renderer->setAlphaBlending(false);

	renderer->setBackBufferRenderTarget();
}

void App1::thresholdPass()
{
	// Set the render target to be the render to texture.
	thresholdTexture->setRenderTarget(renderer->getDeviceContext());
	thresholdTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	camera->update();
	// Get matrices needed
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = thresholdTexture->getOrthoMatrix();

	renderer->setZBuffer(false);

	// Extracts bright areas of the scene used for bloom
	orthoMesh->sendData(renderer->getDeviceContext());
	threshold->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), bloomThreshold);
	threshold->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::horizontalBlur()
{
	// Get the size of the screen in the X direction
	float screenSizeX = (float)horizontalBlurTexture->getTextureWidth();

	// Set the render target to be the render to texture.
	horizontalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	horizontalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 0.0f, 0.0f);

	camera->update();
	// Get matrices needed
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = horizontalBlurTexture->getOrthoMatrix();

	renderer->setZBuffer(false);

	// Render Horizontal Blur
	orthoMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, thresholdTexture->getShaderResourceView(), screenSizeX/4);
	horizontalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::verticalBlur()
{
	// Get the size of the screen in the Y direction
	float screenSizeY = (float)verticalBlurTexture->getTextureHeight();
	
	// Set the render target to be the render to texture.
	verticalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	verticalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 0.0f);

	camera->update();
	// Get matrices needed
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = verticalBlurTexture->getOrthoMatrix();

	renderer->setZBuffer(false);

	// Render Vertical Blur
	orthoMesh->sendData(renderer->getDeviceContext());
	verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, horizontalBlurTexture->getShaderResourceView(), screenSizeY/4);
	verticalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	renderer->setZBuffer(false);
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	// Render the final scene 
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, verticalBlurTexture->getShaderResourceView(), renderTexture->getShaderResourceView(), toggleBloom);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	
	renderer->setZBuffer(true);
	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Camera Position: %.2f, %.2f, %.2f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	// All Vertex manipulation variables are controlled here
	if (ImGui::TreeNode("Vertex Manipulation Variables"))
	{
		// Water variables
		if (ImGui::TreeNode("Water Variables"))
		{
			ImGui::SliderFloat("Frequency", &frequency, 0.0f, 2.0f);
			ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 2.0f);
			ImGui::SliderFloat("Speed", &speed, 0.0f, 5.0f);
			ImGui::TreePop();
		}
		ImGui::SliderFloat("Height map height", &height, 0.000f, 20.000f);
		ImGui::TreePop();
	}

	// All light variables are controlled here
	if (ImGui::TreeNode("Lights"))
	{
		// Dynamic point light variables
		if (ImGui::TreeNode("Point Light 1"))
		{
			ImGui::SliderFloat("Light X", &PointLight1Pos.x, -100.0f, 100.f);
			ImGui::SliderFloat("Light Y", &PointLight1Pos.y, -100.0f, 100.f);
			ImGui::SliderFloat("Light Z", &PointLight1Pos.z, -100.0f, 100.f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Point Light 2"))
		{
			ImGui::SliderFloat("Light X", &PointLight2Pos.x, -100.0f, 100.f);
			ImGui::SliderFloat("Light Y", &PointLight2Pos.y, -100.0f, 100.f);
			ImGui::SliderFloat("Light Z", &PointLight2Pos.z, -100.0f, 100.f);
			ImGui::TreePop();
		}
		// Dynamic point light variables
		if (ImGui::TreeNode("Directional light"))
		{
			ImGui::SliderFloat("Light X", &directionalDirection[0], -1.0f, 1.f);
			ImGui::SliderFloat("Light Y", &directionalDirection[1], -1.0f, 1.f);
			ImGui::SliderFloat("Light Z", &directionalDirection[2], -1.0f, 1.f);
			ImGui::TreePop();
		}

		// Change colour of lights
		if (ImGui::TreeNode("Light Colours"))
		{
			ImGui::ColorEdit4("Point Light 1 Colour", PointLight1DiffuseColours);
			ImGui::ColorEdit4("Point Light 2 Colour", PointLight2DiffuseColours);
			ImGui::ColorEdit4("Directional Light Colour", directionalDiffuseColours);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	// All post processing variables are controlled here
	if (ImGui::TreeNode("Post processing"))
	{
		ImGui::Checkbox("Bloom", &toggleBloom);
		ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 2.0f);
		ImGui::TreePop();
	}

	// Extra details are controlled here
	if (ImGui::TreeNode("Extra details"))
	{
		ImGui::Checkbox("Toggle normals for the height map", &toggleHeightNormals);
		ImGui::Checkbox("Toggle normals for the wave", &toggleWaterNormals);
		ImGui::ColorEdit4("Background colour", backgroundColour);
		ImGui::TreePop();
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::initLights()
{
	// Configure dynamic point light
	PointLight1Pos = XMFLOAT3(0.f, 5.f, 50.0f);
	PointLight1 = new Light();
	PointLight1->setDiffuseColour(PointLight1DiffuseColours[0], PointLight1DiffuseColours[1], PointLight1DiffuseColours[2], PointLight1DiffuseColours[3]);
	PointLight1->setAmbientColour(0.1, 0.1, 0.1, 1.0f);
	PointLight1->setPosition(PointLight1Pos.x, PointLight1Pos.y, PointLight1Pos.z);

	// Configure fixed point light
	PointLight2Pos = XMFLOAT3(36.f, 8.f, 16.0f);
	PointLight2 = new Light();
	PointLight2->setDiffuseColour(PointLight2DiffuseColours[0], PointLight2DiffuseColours[1], PointLight2DiffuseColours[2], PointLight2DiffuseColours[3]);
	PointLight2->setAmbientColour(0.0, 0.0, 0.0, 1.0f);
	PointLight2->setPosition(PointLight2Pos.x, PointLight2Pos.y, PointLight2Pos.z);

	// Configure directional light
	directionalLight = new Light();
	directionalLight->setDiffuseColour(directionalDiffuseColours[0], directionalDiffuseColours[1], directionalDiffuseColours[2], directionalDiffuseColours[3]);
	directionalLight->setPosition(0.f, 30.f, 10.f);
	directionalLight->setDirection(directionalDirection[0], directionalDirection[1], directionalDirection[2]);
	directionalLight->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
}

void App1::initShaders(HWND hwnd)
{
	// Initialise shaders
	horizontalBlurShader = new HorizontalBlurShader(renderer->getDevice(), hwnd);
	verticalBlurShader = new VerticalBlurShader(renderer->getDevice(), hwnd);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	modelDepth = new ModelDepth(renderer->getDevice(), hwnd);
	modelShadow = new ModelShadow(renderer->getDevice(), hwnd);
	waterShadow = new WaterShadow(renderer->getDevice(), hwnd);
	threshold = new Threshold(renderer->getDevice(), hwnd);
}

void App1::initMeshes(int screenWidth, int screenHeight)
{
	// Initialise meshes
	heightMap = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 100);
	water = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
}

