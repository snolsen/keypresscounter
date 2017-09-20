#include <Windows.h>
#include <strsafe.h>

#include "KeyPressStats.h"


// Global identifiers
#define AUTO_SAVE_MS 1800000			// Auto save interval
#define IDT_TIMER1 100U

// Global initialization / instantiation
KeyPressStats keyPressStats;			// The only KeyPressStats object, has to live for the duration of the program


// Forward function declarations
USHORT getVKfromRaw(HRAWINPUT hRawInput);


// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_INPUT:			// RAW input received on top level collector
	{		
		USHORT vKey = getVKfromRaw((HRAWINPUT)lParam);
		if (vKey != NULL) {
			keyPressStats.regKeypress(vKey);		// Register keypress in statistics object	
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}
		break;
	}
	case WM_TIMER:
		if (wParam == IDT_TIMER1)
			keyPressStats.saveStats();				// Save keyboard statistics
	case WM_PAINT:
	{
		WCHAR			windowText[1000] = L"";
		RECT			winRect;
		PAINTSTRUCT		ps;
		HDC				hdc;
		//HDC				hdcMem;
		//HBITMAP			hbmMem;
		//HANDLE			hOld;

		GetClientRect(hwnd, &winRect);


		StringCchPrintf(windowText, sizeof windowText,
			L"%s %-12d\n"
			L"%s %-12d\n"
			L"%s %-12d\n"
			L"%s %#04x (%03d)",
			L"Total number of keystrokes:", keyPressStats.getTotalKeys(),
			L"Mouseclicks (left):", keyPressStats.getTotalLMB(),
			L"Mouseclicks (right):", keyPressStats.getTotalRMB(),
			L"VKey of last input:", keyPressStats.getLast(), keyPressStats.getLast()
		);

		// Get DC
		hdc = BeginPaint(hwnd, &ps);
		//// Create an off-screen DC for double-buffering
		//hdcMem = CreateCompatibleDC(hdc);
		//hbmMem = CreateCompatibleBitmap(hdc, winRect.right, winRect.bottom);
		//hOld = SelectObject(hdcMem, hbmMem);
		// Draw in the off-window DC		
		//FloodFill(hdcMem, 0, 0, RGB(255, 255, 255));
		DrawText(hdc, windowText, static_cast<int>(wcslen(windowText)), &winRect, DT_LEFT | DT_TOP);
		// Transfer the off-screen DC to the screen
		//BitBlt(hdc, 0, 0, winRect.right, winRect.bottom, hdcMem, 0, 0, SRCCOPY);
		
		EndPaint(hwnd, &ps);

		//// Free-up the off-screen DC
		//SelectObject(hdcMem, hOld);
		//DeleteObject(hbmMem);
		//DeleteDC(hdcMem);
		break;
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(hPrevInstance);

	LPCWSTR wndClassName = L"myWindowClass";
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	// Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	INT
		nWidth = 260,
		nHeight = 100,
		x = (GetSystemMetrics(SM_CXSCREEN) - nWidth),
		y = 0
	;
	// Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		wndClassName,
		L"Keypress counter", 
		//WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX,
		WS_SYSMENU | WS_MINIMIZEBOX,
		x, y, nWidth, nHeight,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// Minimize window
	WINDOWPLACEMENT wndPlace;
	GetWindowPlacement(hwnd, &wndPlace);
	wndPlace.showCmd = SW_SHOWMINIMIZED;
	SetWindowPlacement(hwnd, &wndPlace);

	// Get size of minimized window and move to RT corner
	RECT winRect;
	POINT minPos = {0L, 0L};
	GetWindowPlacement(hwnd, &wndPlace);
	GetWindowRect(hwnd, &winRect);
	minPos.x = (LONG)( GetSystemMetrics(SM_CXSCREEN) - winRect.right + winRect.left );
	wndPlace.ptMinPosition = minPos;
	wndPlace.flags = WPF_SETMINPOSITION;
	SetWindowPlacement(hwnd, &wndPlace);

	// Create structure for registering raw keyboard input
	RAWINPUTDEVICE rawInputDevice[2];
	// Keyboard
	rawInputDevice[0].usUsagePage = 1;
	rawInputDevice[0].usUsage = 6;
	rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;		// Allow caller to receive input in the background
	rawInputDevice[0].hwndTarget = hwnd;				// Handle to target window, must be specified for background operation
	// Mouse
	rawInputDevice[1].usUsagePage = 1;
	rawInputDevice[1].usUsage = 2;
	rawInputDevice[1].dwFlags = RIDEV_INPUTSINK;		// Allow caller to receive input in the background
	rawInputDevice[1].hwndTarget = hwnd;				// Handle to target window, must be specified for background operation


	
	// Register the top level collection (will send WM_INPUT msgs for this device to the window).
	RegisterRawInputDevices(rawInputDevice, 2, sizeof RAWINPUTDEVICE);	

	// Start timer to save data every hour
	if (SetTimer(hwnd, IDT_TIMER1, AUTO_SAVE_MS, NULL) == NULL)
		OutputDebugString(L"Could not create timer!");

	// The Message Loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return (INT)Msg.wParam;
}


USHORT getVKfromRaw(HRAWINPUT hRawInput)
{
	USHORT vKey = NULL;
	LPBYTE lpb = nullptr;
	RAWINPUT* raw = nullptr;
	UINT dwSize = 0;

	// Check size of data and allocate space
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

	lpb = new BYTE[dwSize];
	if (lpb == NULL)
		return NULL;

	// Read data  into allocated space
	if (GetRawInputData(hRawInput, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		OutputDebugString(L"GetRawInputData does not return correct size !\n");
	raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEKEYBOARD) // Check that it is a keyboard event
	{
#ifdef _DEBUG
		HRESULT hResult = 0;
		WCHAR szTempOutput[1024] = { L"" };

		hResult = StringCchPrintf(szTempOutput, sizeof szTempOUtput, L" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n",
			raw->data.keyboard.MakeCode,
			raw->data.keyboard.Flags,
			raw->data.keyboard.Reserved,
			raw->data.keyboard.ExtraInformation,
			raw->data.keyboard.Message,
			raw->data.keyboard.VKey);

		if (FAILED(hResult)) {
			// TODO: write error handler
		}
		OutputDebugString(szTempOutput);
#endif // _DEBUG

		if (raw->data.keyboard.Message == WM_KEYUP) // Count only key releases, auto-repeat causes many keydown events when a key is held.
			vKey = raw->data.keyboard.VKey;
	}
	else if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP)
			vKey = VK_LBUTTON;
		else if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP)
			vKey = VK_RBUTTON;
	}
	delete[] lpb;

	return vKey;
}