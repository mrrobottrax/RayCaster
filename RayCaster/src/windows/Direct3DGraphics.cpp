#include "pch.h"
#include "Direct3DGraphics.h"
#include <rendering/ShaderTypes.h>
#include <map/Map.h>
#include <rendering/Camera.h>
#include <game/game.h>

// Globals
// For initialization and utility
ID3D11Device* g_Device;
IDXGISwapChain* g_Swapchain;
ID3D11DeviceContext* g_DeviceContext;
float g_aspectRatio;

// For drawing  -> New stuff right here
ID3D11RenderTargetView* g_RenderTargetView;
D3D11_VIEWPORT g_viewport;

// Compute shader
ID3D11ComputeShader* g_ComputeShader;
ID3D11Buffer* g_CameraDataBuffer;
ID3D11Texture2D* g_RenderTexture;
ID3D11UnorderedAccessView* g_RenderTextureUAV;
ID3D11Texture2D* g_LevelTexture;
ID3D11ShaderResourceView* g_LevelTextureSRV;

// Vertex shader
ID3D11InputLayout* g_VertexInputLayout;
ID3D11VertexShader* g_VertexShader;
ID3D11Buffer* g_VertexBuffer;

// Pixel shader
ID3D11SamplerState* g_Sampler;
ID3D11PixelShader* g_PixelShader;
ID3D11ShaderResourceView* g_RenderTextureSRV;

void InitD3D11(HWND hWnd)
{
	// D3d11 code here
	RECT rect;
	GetClientRect(hWnd, &rect);

	// Attach d3d to the window
	D3D_FEATURE_LEVEL DX11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swap;
	ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap.BufferCount = 2;
	swap.OutputWindow = hWnd;
	swap.Windowed = true;
	swap.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap.BufferDesc.Width = rect.right - rect.left;
	swap.BufferDesc.Height = rect.bottom - rect.top;
	swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap.SampleDesc.Count = 1;

	g_aspectRatio = swap.BufferDesc.Width / (float)swap.BufferDesc.Height;

	HRESULT result;

	IDXGIFactory6* pFactory;
	result = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
	assert(!FAILED(result));

	IDXGIAdapter* pAdapter;
	result = pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter));
	assert(!FAILED(result));

	result = D3D11CreateDeviceAndSwapChain(pAdapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&DX11,
		1,
		D3D11_SDK_VERSION,
		&swap,
		&g_Swapchain,
		&g_Device,
		0,
		&g_DeviceContext
	);
	assert(!FAILED(result));

	pFactory->Release();
	pAdapter->Release();

	ID3D11Texture2D* backbuffer = nullptr;
	result = g_Swapchain->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	assert(backbuffer);
	assert(!FAILED(result));
	result = g_Device->CreateRenderTargetView(backbuffer, NULL, &g_RenderTargetView);
	assert(!FAILED(result));

	// Release the resource to decrement the counter by one
	// This is necessary to keep the buffer from leaking memory
	backbuffer->Release();

	// Setup viewport
	g_viewport.Width = static_cast<FLOAT>(swap.BufferDesc.Width);
	g_viewport.Height = static_cast<FLOAT>(swap.BufferDesc.Height);
	g_viewport.TopLeftY = g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0;
	g_viewport.MaxDepth = 1;

	// Shaders
	CompileShaders();

	// --------- COMPUTE SHADER ---------

	// Camera Data
	CameraData cameraData{};
	cameraData.angle = 0;
	cameraData.position[0] = 0.5f;
	cameraData.position[1] = 0.5f;
	cameraData.position[2] = 0.5f;
	D3D11_SUBRESOURCE_DATA initCameraData{};
	initCameraData.pSysMem = &cameraData;

	D3D11_BUFFER_DESC cBufferDesc{};
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.ByteWidth = ((sizeof(CameraData) / 16) + 1) * 16;
	result = g_Device->CreateBuffer(&cBufferDesc, &initCameraData, &g_CameraDataBuffer);
	assert(!FAILED(result));

	// Render Texture
	D3D11_TEXTURE2D_DESC renderTextureDesc;
	ZeroMemory(&renderTextureDesc, sizeof(renderTextureDesc));
	renderTextureDesc.Width = 512;
	renderTextureDesc.Height = 512;
	renderTextureDesc.MipLevels = 1;
	renderTextureDesc.ArraySize = 1;
	renderTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTextureDesc.SampleDesc.Count = 1;
	renderTextureDesc.SampleDesc.Quality = 0;
	renderTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	renderTextureDesc.CPUAccessFlags = 0;
	renderTextureDesc.MiscFlags = 0;
	result = g_Device->CreateTexture2D(&renderTextureDesc, 0, &g_RenderTexture);
	assert(!FAILED(result));

	// Render Texture UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC renderTextureUavDesc{};
	renderTextureUavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTextureUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	renderTextureUavDesc.Texture2D.MipSlice = 0;
	result = g_Device->CreateUnorderedAccessView(g_RenderTexture, &renderTextureUavDesc, &g_RenderTextureUAV);
	assert(!FAILED(result));

	// Level texture
	D3D11_TEXTURE2D_DESC levelTextureDesc;
	ZeroMemory(&levelTextureDesc, sizeof(levelTextureDesc));
	levelTextureDesc.Width = mapWidth;
	levelTextureDesc.Height = mapHeight;
	levelTextureDesc.MipLevels = 1;
	levelTextureDesc.ArraySize = 1;
	levelTextureDesc.Format = DXGI_FORMAT_R8_SINT;
	levelTextureDesc.SampleDesc.Count = 1;
	levelTextureDesc.SampleDesc.Quality = 0;
	levelTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	levelTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	levelTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	levelTextureDesc.MiscFlags = 0;
	result = g_Device->CreateTexture2D(&levelTextureDesc, 0, &g_LevelTexture);
	assert(!FAILED(result));

	// Level texture SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC levelTextureSrvDesc{};
	levelTextureSrvDesc.Format = DXGI_FORMAT_R8_SINT;
	levelTextureSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	levelTextureSrvDesc.Texture2D.MipLevels = 1;
	levelTextureSrvDesc.Texture2D.MostDetailedMip = 0;
	result = g_Device->CreateShaderResourceView(g_LevelTexture, &levelTextureSrvDesc, &g_LevelTextureSRV);
	assert(!FAILED(result));

	// --------- VERTEX SHADER ---------

	// Vertex buffer
	D3D11_SUBRESOURCE_DATA initVertexData{};
	float verts[] = {
		-1, -1, 0, 0, 1,
		-1, 1, 0, 0, 0,
		1, -1, 0, 1, 1,
		1, 1, 0, 1, 0,
	};
	initVertexData.pSysMem = verts;

	D3D11_BUFFER_DESC vBufferDesc{};
	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vBufferDesc.ByteWidth = sizeof(verts);
	result = g_Device->CreateBuffer(&vBufferDesc, &initVertexData, &g_VertexBuffer);
	assert(!FAILED(result));

	// --------- PIXEL SHADER ---------

	// Render Texture SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTextureSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;
	renderTextureSrvDesc.Texture2D.MostDetailedMip = 0;
	result = g_Device->CreateShaderResourceView(g_RenderTexture, &renderTextureSrvDesc, &g_RenderTextureSRV);
	assert(!FAILED(result));

	// Render Texture Sampler
	D3D11_SAMPLER_DESC renderTextureSamplerDesc{};
	renderTextureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	renderTextureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	renderTextureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	renderTextureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	renderTextureSamplerDesc.MipLODBias = 0.0f;
	renderTextureSamplerDesc.MaxAnisotropy = 1;
	renderTextureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	renderTextureSamplerDesc.BorderColor[0] = 0;
	renderTextureSamplerDesc.BorderColor[1] = 0;
	renderTextureSamplerDesc.BorderColor[2] = 0;
	renderTextureSamplerDesc.BorderColor[3] = 0;
	renderTextureSamplerDesc.MinLOD = 0;
	renderTextureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = g_Device->CreateSamplerState(&renderTextureSamplerDesc, &g_Sampler);
	assert(!FAILED(result));

	// ----  UPDATE MAP -----
	UpdateGpuMapD3D11();
}

void EndD3D11()
{
	// device
	g_Device->Release();
	g_DeviceContext->Release();
	g_RenderTargetView->Release();
	g_Swapchain->Release();

	// compute shader
	g_ComputeShader->Release();
	g_CameraDataBuffer->Release();
	g_RenderTexture->Release();
	g_RenderTextureUAV->Release();
	g_LevelTexture->Release();
	g_LevelTextureSRV->Release();

	// vertex shader
	g_VertexShader->Release();
	g_VertexBuffer->Release();
	g_VertexInputLayout->Release();

	// pixel shader
	g_PixelShader->Release();
	g_RenderTextureSRV->Release();
	g_Sampler->Release();
}

void UpdateGpuMapD3D11()
{
	g_DeviceContext->UpdateSubresource(g_LevelTexture, 0, nullptr, GetMapPointer(), mapWidth, 0);
}

void UpdateCameraDataD3D11(Camera& camera)
{
	static uint32_t frame = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	g_DeviceContext->Map(g_CameraDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	CameraData cameraData{};
	cameraData.position[0] = camera.position.x;
	cameraData.position[1] = camera.position.y;
	cameraData.position[2] = camera.position.z;
	cameraData.angle = camera.yaw;
	cameraData.frame = frame++;

	memcpy(mappedResource.pData, &cameraData, sizeof(cameraData));

	g_DeviceContext->Unmap(g_CameraDataBuffer, 0);
}

void DrawFrameD3D11()
{
	// clear
	g_DeviceContext->RSSetViewports(1, &g_viewport);
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, ClearColor);

	// setup
	g_DeviceContext->OMSetRenderTargets(1, &g_RenderTargetView, nullptr);
	UpdateCameraDataD3D11(localPlayer.camera);

	// compute shader
	g_DeviceContext->CSSetShader(g_ComputeShader, NULL, 0);
	g_DeviceContext->CSSetConstantBuffers(0, 1, &g_CameraDataBuffer);
	g_DeviceContext->CSSetShaderResources(0, 1, &g_LevelTextureSRV);

	// Unbind any shader resources that might be bound to the pipeline
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	g_DeviceContext->PSSetShaderResources(0, 1, nullSRV);

	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, &g_RenderTextureUAV, nullptr);

	g_DeviceContext->Dispatch(32, 32, 1);

	ID3D11UnorderedAccessView* nullUAV[1] = { NULL };
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, NULL);

	// display texture
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(float) * 5;
	UINT offset = 0;
	g_DeviceContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	g_DeviceContext->IASetInputLayout(g_VertexInputLayout);

	g_DeviceContext->VSSetShader(g_VertexShader, NULL, 0);

	// Now you can bind the shader resource view to the pipeline
	g_DeviceContext->PSSetShaderResources(0, 1, &g_RenderTextureSRV);

	g_DeviceContext->PSSetShader(g_PixelShader, NULL, 0);
	g_DeviceContext->PSSetSamplers(0, 1, &g_Sampler);

	g_DeviceContext->Draw(4, 0);

	g_Swapchain->Present(0, 0);
}

bool GetBlobs(LPCWSTR pFileName, ID3DBlob** ppShaderBlob, LPCSTR profile)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(pFileName, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", profile, flags, 0, ppShaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		g_Device->Release();
		printf("Failed compiling shader %08X\n", hr);

		if (errorBlob)
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		return false;
	}

	return true;
}

void CompileComputeShader(LPCWSTR pFileName, ID3D11ComputeShader** ppComputeShader)
{
	ID3DBlob* shaderBlob;
	if (!GetBlobs(pFileName, &shaderBlob, "cs_5_0"))
	{
		return;
	}

	// Create shader
	HRESULT hr = g_Device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ppComputeShader);

	shaderBlob->Release();

	if (FAILED(hr))
	{
		g_Device->Release();
		return;
	}
}

void CompileVertexShader(LPCWSTR pFileName, ID3D11VertexShader** ppVertexShader)
{
	ID3DBlob* shaderBlob;
	if (!GetBlobs(pFileName, &shaderBlob, "vs_5_0"))
	{
		return;
	}

	// Create shader
	HRESULT hr = g_Device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ppVertexShader);

	if (FAILED(hr))
	{
		g_Device->Release();
		return;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layouts
	hr = g_Device->CreateInputLayout(vertexLayout, 2, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &g_VertexInputLayout);

	shaderBlob->Release();

	if (FAILED(hr))
	{
		return;
	}
}

void CompilePixelShader(LPCWSTR pFileName, ID3D11PixelShader** ppPixelShader)
{
	ID3DBlob* shaderBlob;
	if (!GetBlobs(pFileName, &shaderBlob, "ps_5_0"))
	{
		return;
	}

	// Create shader
	HRESULT hr = g_Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ppPixelShader);

	shaderBlob->Release();

	if (FAILED(hr))
	{
		g_Device->Release();
		return;
	}
}

void CompileShaders()
{
	CompileComputeShader(L"data/Test.hlsl", &g_ComputeShader);
	CompileVertexShader(L"data/QuadVS.hlsl", &g_VertexShader);
	CompilePixelShader(L"data/QuadPS.hlsl", &g_PixelShader);
}