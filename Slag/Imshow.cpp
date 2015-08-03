#include "OS_dependent.h"

#include <windows.h>

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include "slag/slag_interface.h"

#define SLAG_WINDOW_CLASS_NAME "SlagImshow"

static int init();
static const int init_val = init();

static WNDCLASSEX windowsClass;
static HINSTANCE hInstance;
static STARTUPINFO startupInfo;
static std::mutex _mutex;
static bool _run = true;

typedef std::lock_guard<std::mutex> AutoLock;

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

struct ImageContainer
{
	ImageContainer():w(0), h(0), type(ImageType::GREY), data(){}
	int w;
	int h;
	ImageType type;
	std::vector<unsigned char> data;
};

class WindowWrapper
{
public:
	WindowWrapper(const std::string& name);
	~WindowWrapper();

	ImageContainer _image;
	const std::string _name;
	HWND _hwnd;
	bool _valid;
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

inline size_t GetByteDepth(ImageType t)
{
	switch (t)
	{
	case RGB:
	case BGR:  return 3;
	case RGBA: return 4;
	case GREY:
	default:   return 4;
	}
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
	switch (bitdepth)
	{
	case 32:
		switch (type)
		{
		case RGB:  return cv::COLOR_RGB2BGRA;
		case BGR:  return cv::COLOR_BGR2BGRA;
		case RGBA: return cv::COLOR_RGBA2BGRA;
		case GREY: 
		default:   return cv::COLOR_GRAY2BGRA;
		}
	case 24:
		switch (type)
		{
		case RGB:  return cv::COLOR_RGB2BGR;
		case BGR:  return -1;
		case RGBA: return cv::COLOR_RGBA2BGR;
		case GREY: 
		default:   return cv::COLOR_GRAY2BGR;
		}
	default: return -1;
	}
}

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
		AutoLock lock(_mutex);
		auto it = std::find_if(_images.begin(), _images.end(), HwndFinder(hwnd));
		if (it == _images.end())
			MessageBox(NULL, "wanted to delete a non-existent window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		else
			_images.erase(it);
		}break;
	case WM_PAINT:
	case WM_TIMER:
		{
			_mutex.lock();
			auto it = std::find_if(_images.begin(), _images.end(), HwndFinder(hwnd));
			if (it != _images.end())
			{
				cv::Mat converted;
				cv::Mat header(it->_image.h, it->_image.w, GetCvType(it->_image.type), (void*)it->_image.data.data());
				if (GetCvConversion(it->_image.type) >= 0)
					cv::cvtColor(header, converted, GetCvConversion(it->_image.type));
				else
					converted = header;
				
				_mutex.unlock();
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(it->_hwnd, &ps);

				HDC hdcBuffer = CreateCompatibleDC(hdc);

				HBITMAP hbm = CreateBitmap(converted.cols,converted.rows, planes, bitdepth, converted.data);

				HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

				BitBlt(hdc, 0, 0, converted.cols,converted.rows, hdcBuffer, 0, 0, SRCCOPY);

				SelectObject(hdcBuffer, hbmOldBuffer);
				DeleteObject(hbm);
				DeleteDC(hdcBuffer);

				EndPaint(it->_hwnd, &ps);
			}else
			{
				_mutex.unlock();
			}
		}break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void FeedImshow()
{
	static MSG Msg;

	if (!_images.empty())
	{
		const auto result = GetMessage(&Msg, NULL, 0, 0);
		if (result >= 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}	
}

WindowWrapper::WindowWrapper(const std::string& name)
	: _name(name), _valid(true), _hwnd(NULL)
{
}

WindowWrapper::~WindowWrapper()
{
	if (_hwnd)
		if (!DestroyWindow(_hwnd))
			MessageBox(NULL, "DestroyWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
}

//_hwnd = CreateWindowEx(
//	0,
//	SLAG_WINDOW_CLASS_NAME,
//	_name.c_str(),
//	WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
//	CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, //default size
//	NULL, NULL, hInstance, NULL);
//
//if (!_hwnd)
//{
//	auto error = GetLastError();
//	MessageBox(NULL, "CreateWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
//	return;
//}
//
//ShowWindow(_hwnd, SW_SHOWNORMAL);
//UpdateWindow(_hwnd);

void handle_output_image( const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data )
{
	AutoLock lock(_mutex);
	auto it = std::find_if(_images.begin(), _images.end(), NameFinder(module_name_and_instance));
	if (it == _images.end())
	{
		_images.emplace_back(module_name_and_instance.c_str());
		it = _images.end();
		--it;
	}

	size_t picure_size;
	if (w > 0 && h > 0 && (picure_size = w * h * GetByteDepth(type)) > 0)
	{
		it->_image.w = w;
		it->_image.h = h;
		it->_image.type = type;
		it->_image.data.assign(data, data + picure_size);

		it->_valid = false;
	}
}

static std::thread _thread([]()
{
	while (_run)
	{

	}
});

void terminate_output_image()
{
	_images.clear();
	_run = false;
	_thread.join();
}
