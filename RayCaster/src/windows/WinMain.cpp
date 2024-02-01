#include <pch.h>

using namespace Gdiplus;

VOID OnPaint(HDC hdc)
{
	Graphics graphics(hdc);

	Pen pen(Color(255, 0, 0, 255));
	graphics.DrawLine(&pen, 0, 0, 200, 100);

	FontFamily  fontFamily(L"Times New Roman");
	Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	PointF      pointF(30.0f, 10.0f);
	SolidBrush  solidBrush(Color(255, 0, 0, 255));

	graphics.DrawString(L"Hello", -1, &font, pointF, &solidBrush);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR pCmdLine,
	_In_ int nCmdShow
)
{
	// set up window class
	WNDCLASS wc = {};

	const wchar_t CLASS_NAME[] = L"Main Window Class";

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,                              // optional window styles.
		CLASS_NAME,                     // window class
		L"Game",						// window text
		WS_OVERLAPPEDWINDOW,            // window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // parent window    
		NULL,       // menu
		hInstance,  // instance handle
		NULL        // additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// run the message loop.
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			OnPaint(hdc);

			EndPaint(hwnd, &ps);
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}