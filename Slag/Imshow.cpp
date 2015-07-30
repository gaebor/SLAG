#include "Imshow.h"

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include <windows.h>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

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
};

static std::list<WindowWrapper> _images;

static int bitdepth, planes;

struct NameFinder
{
	NameFinder(const std::string& name): _name(name){}
	const std::string _name;
	bool operator()(const WindowWrapper& w)
	{
		return w._name == _name;
	}
};

struct HwndFinder
{
	HwndFinder(const HWND& hwnd): _hwnd(hwnd){}
	const HWND _hwnd;
	bool operator()(const WindowWrapper& w)
	{
		return w._hwnd == _hwnd;
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
		auto it = std::find_if(_images.begin(), _images.end(), HwndFinder(hwnd));
		if (it == _images.end())
			MessageBox(NULL, "wanted to delete a non-existent window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		else
			_images.erase(it);
		}break;
	case WM_PAINT:
		{

		}break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

inline int GetCvType(ImageType type)
{
	switch (type)
	{
	case RGB:
	case BGR:  return CV_8UC3;
	case RGBA: return CV_8UC4;
	case GREY:
	default:   return CV_8UC1;
	}
}

inline int GetCvConversion(ImageType type)
{
	switch (type)
	{
	case RGB:  return cv::COLOR_RGB2BGRA;
	case BGR:  return cv::COLOR_BGR2BGRA;
	case RGBA: return 0;
	case GREY: 
	default:   return cv::COLOR_GRAY2BGRA;
	}
}

void Imshow( const char* window_name, const ImageContainer& imageContainer)
{
	auto it = std::find_if(_images.begin(), _images.end(), NameFinder(window_name));
	if (it == _images.end())
	{
		 _images.emplace_back(window_name);
		 it = _images.end();
		 --it;
	}
	auto& windowWrapper = *it;

	cv::Mat converted;
	cv::cvtColor(cv::Mat(imageContainer.h, imageContainer.w, GetCvType(imageContainer.type), (void*)imageContainer.data.data()), converted, GetCvConversion(imageContainer.type));

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(windowWrapper._hwnd, &ps);

	HDC hdcBuffer = CreateCompatibleDC(hdc);

	HBITMAP hbm = CreateBitmap(imageContainer.w,imageContainer.h, planes, bitdepth, converted.data);

	HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

	BitBlt(hdc, 0, 0, imageContainer.w,imageContainer.h, hdcBuffer, 0, 0, SRCCOPY);

	SelectObject(hdcBuffer, hbmOldBuffer);
	DeleteObject(hbm);
	DeleteDC(hdcBuffer);

	EndPaint(windowWrapper._hwnd, &ps);

}

int FeedImshow()
{
	MSG Msg;
	
	for (auto& i : _images)
		if (GetMessage(&Msg, i._hwnd, 0, 0) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}else
			return -1;
	return 1;
}

WindowWrapper::WindowWrapper(const char* name)
	: _name(name)
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

	ShowWindow(_hwnd, startupInfo.wShowWindow);
	UpdateWindow(_hwnd);

	//MSG Msg;
	//
	//while(GetMessage(&Msg, _hwnd, 0, 0) > 0)
	//{
	//	TranslateMessage(&Msg);
	//	DispatchMessage(&Msg);
	//}
}

WindowWrapper::~WindowWrapper()
{
	if (_hwnd)
		if (!DestroyWindow(_hwnd))
			MessageBox(NULL, "DestroyWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
}
