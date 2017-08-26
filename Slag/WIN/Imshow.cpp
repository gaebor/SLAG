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

	void Init();

	bool _run;
	ImageContainer _image;
	const std::string _name;
	HWND _hwnd;
	std::mutex _mutex;
	int actual_width;
	int actual_height;
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

//! wor windows, this is always the same
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
			it->_run = false;
		}break;
	case WM_PAINT:
		{
			_mutex.lock();
			auto it = std::find_if(_images.begin(), _images.end(), HwndFinder(hwnd));
			if (it != _images.end())
			{
				AutoLock imageLock(it->_mutex);
				_mutex.unlock();

				if (it->actual_width != it->_image.w || it->actual_height != it->_image.h)
				{
					SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0,
						it->_image.w, it->_image.h,
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER
					);

					it->actual_width = it->_image.w;
					it->actual_height = it->_image.h;
				}
				else
				{
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hwnd, &ps);

					HDC hdcBuffer = CreateCompatibleDC(hdc);

					HBITMAP hbm = CreateBitmap(it->_image.w, it->_image.h, planes, bitdepth, it->_image.data.data());

					HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

					BitBlt(hdc, 0, 0, it->_image.w, it->_image.h, hdcBuffer, 0, 0, SRCCOPY);

					SelectObject(hdcBuffer, hbmOldBuffer);
					DeleteObject(hbm);
					DeleteDC(hdcBuffer);

					EndPaint(hwnd, &ps);
				}
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
	: _name(name), _hwnd(NULL), _run(true), actual_height(1), actual_width(1),
	_thread([](WindowWrapper* self)
{
	MSG Msg;

	while (self->_run)
	{
		if (self->_hwnd == NULL)
		{
			self->Init();
		}else
		{
			if (GetMessage(&Msg, self->_hwnd, 0, 0) > 0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
			else
				break;
		}
	}
	if (self->_hwnd)
	{
		if (!DestroyWindow(self->_hwnd))
			MessageBox(NULL, "DestroyWindow Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		self->_hwnd = NULL;
	}
}, this)
{
}

void WindowWrapper::Init()
{
	_hwnd = CreateWindowEx(
		0,
		SLAG_WINDOW_CLASS_NAME,
		_name.c_str(),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, actual_width, actual_height,
		NULL, NULL, _hInstance, NULL);

	if (!_hwnd)
	{
		auto error = GetLastError();
		char error_str[20];
		sprintf_s(error_str, "Error %d!", error);
		MessageBox(NULL, "CreateWindow Failed!", error_str, MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	ShowWindow(_hwnd, SW_SHOWNORMAL);
}

WindowWrapper::~WindowWrapper()
{
	_run = false;
	_thread.join();
}

void handle_output_image( const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data )
{
	_mutex.lock();
	auto it = _images.begin();
	for (; it != _images.end(); it = (it->_run ? ++it : _images.erase(it)))
	{
		// Purge windows that are not running
	}
	it = std::find_if(_images.begin(), _images.end(), NameFinder(module_name_and_instance));
	if (it == _images.end())
	{
		_images.emplace_back(module_name_and_instance.c_str()); // starts WndProc
		it = _images.end();
		--it;
	}

	AutoLock imageLock(it->_mutex);
	_mutex.unlock();

	size_t picure_size;
	if (data && w > 0 && h > 0 && (picure_size = w * h * GetByteDepth(type)) > 0)
	{
		it->_image.w = w;
		it->_image.h = h;
		it->_image.type = type;
		it->_image.data.assign(data, data + picure_size);

		if (it->_hwnd)
			InvalidateRect(it->_hwnd, 0, 0);
	}
}

void terminate_output_image()
{
	_images.clear();
}
