
#include <windows.h>

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include <memory>

#include <fstream>

#include "slag_interface.h"

#define SLAG_WINDOW_CLASS_NAME TEXT("SlagImshow")

static ATOM windowAtom;
static std::mutex _mutex;
static HINSTANCE const _hInstance = GetModuleHandle(NULL);
static double _scale = 1.0;

typedef std::lock_guard<std::mutex> AutoLock;

LRESULT CALLBACK SlagWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

struct ImageContainer
{
	ImageContainer():w(0), h(0), type(SlagImageType::GREY), data(){}
	int w;
	int h;
    SlagImageType type;
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
#ifdef UNICODE
	const std::wstring _name;
#else
    const std::string _name;
#endif
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
    WNDCLASSEX wc;
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
	windowAtom = RegisterClassEx(&wc);
	if (!windowAtom)
	{
		MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"),
			MB_ICONEXCLAMATION | MB_OK);
	}
	return 0;
}

static const int init_val = init();

struct NameFinder
{
	NameFinder(const std::string& name): _name(name.begin(), name.end()){}
	decltype(WindowWrapper::_name) _name;
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

inline size_t GetByteDepth(SlagImageType t)
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
			MessageBox(NULL, TEXT("wanted to delete a non-existent window!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
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
						(int)std::ceil(_scale * (it->_image.w)), (int)std::ceil(_scale * (it->_image.h)),
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

					//BitBlt(hdc, 0, 0, it->_image.w, it->_image.h, hdcBuffer, 0, 0, SRCCOPY);
					StretchBlt(
						hdc,       0, 0,
						(int)std::ceil(_scale * (it->_image.w)), (int)std::ceil(_scale * (it->_image.h)),
						hdcBuffer, 0, 0,
						it->_image.w                           , it->_image.h,
						SRCCOPY);

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
	: _name(name.begin(), name.end()), _hwnd(NULL), _run(true), actual_height(1), actual_width(1),
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
		RECT rect;
		if (GetWindowRect(self->_hwnd, &rect))
		{
			std::ofstream ofs(self->_name + TEXT(".img.pos"));
			if (ofs)
			{
				ofs << rect.top << " " << rect.left;
			}
		}

		if (!DestroyWindow(self->_hwnd))
			MessageBox(NULL, TEXT("DestroyWindow Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
		self->_hwnd = NULL;
	}
}, this)
{
}

void WindowWrapper::Init()
{
	std::ifstream ifs(_name + TEXT(".img.pos"));
	int top_corner = 0, left_corner = 0;
	if (ifs)
	{
		ifs >> top_corner >> left_corner;
	}

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
		TCHAR error_str[20];
		wsprintf(error_str, TEXT("Error %d!"), error);
		MessageBox(NULL, TEXT("CreateWindow Failed!"), error_str, MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	ShowWindow(_hwnd, SW_SHOWNORMAL);

	SetWindowPos(_hwnd, HWND_NOTOPMOST, left_corner, top_corner,
		actual_width, actual_height,
		SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER
	);
}

WindowWrapper::~WindowWrapper()
{
	_run = false;
	PostMessage(_hwnd, WM_NOTIFY, 0, 0);
	_thread.join();
}

void handle_output_image( const std::string& module_name_and_instance, int w, int h, SlagImageType type, const unsigned char* data )
{
	std::ptrdiff_t picture_size = w * h * GetByteDepth(type);

	if (data && w > 0 && h > 0 && picture_size > 0)
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

		it->_image.w = w;
		it->_image.h = h;
		it->_image.type = type;
		it->_image.data.assign(data, data + picture_size);

		if (it->_hwnd)
			InvalidateRect(it->_hwnd, 0, 0);
	}
}

void terminate_output_image()
{
	_images.clear();
}

void configure_output_image(const std::vector<std::string>& params)
{
	for (size_t i = 0; i < params.size(); ++i)
	{
		if (params[i] == "-s" || params[i] == "--scale" && i + 1 < params.size())
		{
			double scale = atof(params[i + 1].c_str());
			if (scale > 0)
				_scale= scale;
		}
	}
}