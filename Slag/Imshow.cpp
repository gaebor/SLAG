#include "Imshow.h"

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>

#include <windows.h>

#include "ExclusiveAccess.h"

#define SLAG_WINDOW_CLASS_NAME "SlagImshow"

static int init();
static const int init_val = init();

static WNDCLASSEX windowsClass;
static HINSTANCE hInstance;
static STARTUPINFO startupInfo;

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

int init()
{
	hInstance = GetModuleHandle(NULL);

	GetStartupInfo(&startupInfo);

	auto& wc = windowsClass;
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = SLAG_WINDOW_CLASS_NAME;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	//TODO UnregisterClass
	auto result = RegisterClassEx(&windowsClass);
	if (!result)
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		exit(-1);
	}
	return 0;
}

class WindowWrapper
{
public:
	WindowWrapper(const char*);
	~WindowWrapper();

	const std::string _name;
	HWND _hwnd;
	std::unique_ptr<std::thread> _thread;
	struct ImageData
	{
		int w,h;
		std::vector<unsigned char> data;
	}image_data;
	bool redraw;
};

typedef ExclusiveAccess<WindowWrapper> ManagedWindow;

static ExclusiveAccess<std::list<ManagedWindow>> _images;

static int bitdepth, planes;

struct NameFinder
{
	NameFinder(const std::string& name): _name(name){}
	const std::string _name;
	bool operator()(const ManagedWindow& w)
	{
		return w.Get()._name == _name;
	}
};

struct HwndFinder
{
	HwndFinder(const HWND& hwnd): _hwnd(hwnd){}
	const HWND _hwnd;
	bool operator()(const ManagedWindow& w)
	{
		return w.Get()._hwnd == _hwnd;
	}
};

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch(msg)
	{
	case WM_CREATE:
		{
			HDC hdc = GetDC(hwnd);

			static int* const bitdepth_ptr = &(bitdepth = GetDeviceCaps(hdc, BITSPIXEL));
			static int* const planes_ptr = &(planes = GetDeviceCaps(hdc, PLANES));

			ReleaseDC(hwnd, hdc);
		}
		break;
	case WM_CLOSE:
		{
		_images.NonEditable();
		auto it = std::find_if(_images.Set().begin(), _images.Set().end(), HwndFinder(hwnd));
		if (it == _images.Set().end())
			MessageBox(NULL, "wanted to delete a non-existent window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		else
			_images.Set().erase(it);

		_images.MakeEditable();
		}break;
	case WM_PAINT:
		{
			printf("painted");
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			HDC hdcBuffer = CreateCompatibleDC(hdc);

			_images.NonEditable();
			auto it = std::find_if(_images.Set().begin(), _images.Set().end(), HwndFinder(hwnd));
			if (it != _images.Set().end())
			{
				auto& image_data = *it;
				_images.MakeEditable();
				image_data.NonEditable();

				HBITMAP hbm = CreateBitmap(image_data.Get().image_data.w,image_data.Get().image_data.h, planes, bitdepth, image_data.Get().image_data.data.data());

				HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);
				
				BitBlt(hdc, 0, 0, image_data.Get().image_data.w, image_data.Get().image_data.h, hdcBuffer, 0, 0, SRCCOPY);
				image_data.MakeEditable();

				SelectObject(hdcBuffer, hbmOldBuffer);
				DeleteObject(hbm);
			}
			_images.MakeEditable();
			DeleteDC(hdcBuffer);

			EndPaint(hwnd, &ps);
		}break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void Imshow( const char* window_name, int width, int height, const unsigned char* bits )
{
	_images.NonEditable();
	auto it = std::find_if(_images.Set().begin(), _images.Set().end(), NameFinder(window_name));
	if (it == _images.Set().end())
	{
		 _images.Set().emplace_back(window_name);
		 it = _images.Set().end();
		 --it;
	}
	it->NonEditable();
	auto& windowWrapper = it->Set();
	_images.MakeEditable();
	windowWrapper.image_data.w = width;
	windowWrapper.image_data.h = height;
	windowWrapper.image_data.data.assign(bits, bits + (bitdepth/8)*width*height);
	it->Set().redraw = true;
	it->MakeEditable();

}

WindowWrapper::WindowWrapper(const char* name)
	: _name(name), redraw(false)
{
	_thread.reset(new std::thread([&]()
	{
		_hwnd = CreateWindowEx(
			0,
			SLAG_WINDOW_CLASS_NAME,
			_name.c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, //default size
			NULL, NULL, hInstance, NULL);

		if (!_hwnd)
		{
			auto error = GetLastError();
			MessageBox(NULL, "CreateWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		MSG Msg;
		ShowWindow(_hwnd, startupInfo.wShowWindow);
		UpdateWindow(_hwnd);

		while(GetMessage(&Msg, _hwnd, 0, 0) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
			if (redraw)
			{
				UpdateWindow(_hwnd);
				redraw = false;
			}
		}

	}));
}

WindowWrapper::~WindowWrapper()
{
	if (_hwnd)
		if (!DestroyWindow(_hwnd))
			MessageBox(NULL, "DestroyWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

	_thread->join();
}
