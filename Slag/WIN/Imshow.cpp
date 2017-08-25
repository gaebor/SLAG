#include "..\OS_dependent.h"

#include <windows.h>

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include <memory>

#include "slag/slag_interface.h"

#define SLAG_WINDOW_CLASS_NAME "SlagImshow"

static WNDCLASSEX windowsClass;
static std::mutex _mutex;
static HINSTANCE const _hInstance = GetModuleHandle(NULL);

typedef std::lock_guard<std::mutex> AutoLock;

LRESULT CALLBACK SlagWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

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

	bool _run;
	ImageContainer _image;
	const std::string _name;
	HWND _hwnd;
	bool _validity;
	std::mutex _mutex;
private:
	std::thread _thread;
};

static std::list<WindowWrapper> _images;

static int bitdepth, planes;

int init()
{
	auto& wc = windowsClass;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = SlagWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SLAG_WINDOW_CLASS_NAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//TODO UnregisterClass
	auto result = RegisterClassEx(&windowsClass);
	if (!result)
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
	}
	return 0;
}

static const int init_val = init();

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

//inline int GetCvType(ImageType type)
//{
//	switch (type)
//	{
//	case RGB:
//	case BGR:  return CV_8UC3;
//	case RGBA: return CV_8UC4;
//	case GREY:
//	default:   return CV_8UC1;
//	}
//}
//
//inline int GetCvConversion(ImageType type)
//{
//	switch (bitdepth)
//	{
//	case 32:
//		switch (type)
//		{
//		case RGB:  return cv::COLOR_RGB2BGRA;
//		case BGR:  return cv::COLOR_BGR2BGRA;
//		case BGRA:  return -1;
//		case RGBA: return cv::COLOR_RGBA2BGRA;
//		case GREY: 
//		default:   return cv::COLOR_GRAY2BGRA;
//		}
//	case 24:
//		switch (type)
//		{
//		case RGB:  return cv::COLOR_RGB2BGR;
//		case BGR:  return -1;
//		case RGBA: return cv::COLOR_RGBA2BGR;
//		case BGRA: return cv::COLOR_BGRA2BGR;
//		case GREY: 
//		default:   return cv::COLOR_GRAY2BGR;
//		}
//	default: return -1;
//	}
//}

ImageType get_image_type(void)
{
	return ImageType::RGBA;
}
LRESULT CALLBACK SlagWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		{
			_mutex.lock();
			auto it = std::find_if(_images.begin(), _images.end(), HwndFinder(hwnd));
			if (it != _images.end())
			{
				_mutex.unlock();

				//cv::Mat converted;
				//cv::Mat header(it->_image.h, it->_image.w, GetCvType(it->_image.type), (void*)it->_image.data.data());
				//if (GetCvConversion(it->_image.type) >= 0)
				//	cv::cvtColor(header, converted, GetCvConversion(it->_image.type));
				//else
				//	converted = header;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(it->_hwnd, &ps);

				HDC hdcBuffer = CreateCompatibleDC(hdc);

				HBITMAP hbm = CreateBitmap(it->_image.w, it->_image.h, planes, bitdepth, it->_image.data.data());

				HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

				BitBlt(hdc, 0, 0, it->_image.w, it->_image.h, hdcBuffer, 0, 0, SRCCOPY);

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

WindowWrapper::WindowWrapper(const std::string& name)
	: _name(name), _validity(false), _hwnd(NULL), _run(true), _thread(
	[&]()
{
	MSG Msg;

	while (_run)
	{
		{
			AutoLock lock(_mutex);
			if (_hwnd == NULL && _image.w > 0 && _image.h > 0)
			{
				_hwnd = CreateWindowEx(
					0,
					SLAG_WINDOW_CLASS_NAME,
					_name.c_str(),
					WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
					CW_USEDEFAULT, CW_USEDEFAULT, _image.w, _image.h,
					NULL, NULL, _hInstance, NULL);

				if (!_hwnd)
				{
					auto error = GetLastError();
					char error_str[20];
					sprintf_s(error_str, "Error %d!", error);
					MessageBox(NULL, "CreateWindow Failed!", error_str, MB_ICONEXCLAMATION | MB_OK);
					break;
				}

				ShowWindow(_hwnd, SW_SHOWNORMAL);
				UpdateWindow(_hwnd);
				_validity = true;
			}
			if (_hwnd != NULL && !_validity)
			{
				InvalidateRect(_hwnd, NULL, FALSE);
				_validity = true;
			}
		}
		if (_hwnd != NULL)
		{
			const auto result = GetMessage(&Msg, _hwnd, 0, 0);
			if (result >= 0)
			{
				AutoLock lock(_mutex);
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}
	if (_hwnd)
		if (!DestroyWindow(_hwnd))
			MessageBox(NULL, "DestroyWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
}
	)
{
}

WindowWrapper::~WindowWrapper()
{
	_run = false;
	_thread.join();
}

void handle_output_image( const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data )
{
	_mutex.lock();
	auto it = std::find_if(_images.begin(), _images.end(), NameFinder(module_name_and_instance));
	if (it == _images.end())
	{
		_images.emplace_back(module_name_and_instance.c_str());
		it = _images.end();
		--it;
	}

	AutoLock imageLock(it->_mutex);
	_mutex.unlock();

	size_t picure_size;
	if (w > 0 && h > 0 && (picure_size = w * h * GetByteDepth(type)) > 0)
	{
		it->_image.w = w;
		it->_image.h = h;
		it->_image.type = type;
		it->_image.data.assign(data, data + picure_size);

		it->_validity = false;
	}
}

void terminate_output_image()
{
	//_images.clear();
}
