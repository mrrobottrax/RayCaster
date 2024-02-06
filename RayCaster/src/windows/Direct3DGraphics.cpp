#include "pch.h"
#include "Direct3DGraphics.h"

// Globals
// For initialization and utility
ID3D11Device* g_Device;
IDXGISwapChain* g_Swapchain;
ID3D11DeviceContext* g_DeviceContext;
float g_aspectRatio;

// For drawing  -> New stuff right here
ID3D11RenderTargetView* g_RenderTargetView;
D3D11_VIEWPORT g_viewport;

// Shader
ID3D11ComputeShader* g_ComputeShader;

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

	result = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
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

	ID3D11Texture2D* backbuffer = nullptr;
	result = g_Swapchain->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	assert(backbuffer != 0);
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
}

void EndD3D11()
{
	g_Device->Release();
	g_DeviceContext->Release();
	g_RenderTargetView->Release();
	g_Swapchain->Release();
	g_ComputeShader->Release();
}

void DrawFrameD3D11()
{
	// clear
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, ClearColor);

	// viewport
	g_DeviceContext->RSSetViewports(1, &g_viewport);

	// shader
	g_DeviceContext->CSSetShader(g_ComputeShader, nullptr, 0);

	// g_DeviceContext->CSSetShaderResources(0, 2, nullptr);
	g_DeviceContext->Dispatch(1, 1, 1);

	ID3D11ShaderResourceView* view;
	g_DeviceContext->CSGetShaderResources(0, 2, &view);

	g_Swapchain->Present(0, 0);
}

void CompileShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif
	// Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
	LPCSTR profile = (g_Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(L"data/Test.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"CSMain", profile,
		flags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		g_Device->Release();
		printf("Failed compiling shader %08X\n", hr);
		return;
	}

	// Create shader
	hr = g_Device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &g_ComputeShader);

	shaderBlob->Release();

	if (FAILED(hr))
	{
		g_Device->Release();

		std::cout << errorBlob->GetBufferPointer() << "\n";

		errorBlob->Release();
		return;
	}
}