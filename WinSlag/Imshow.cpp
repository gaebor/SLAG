
#include <windows.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include <memory>

#include <fstream>
#include <atomic>

#include "slag_interface.h"
#include "Identifiers.h"

#define SLAG_WINDOW_CLASS_NAME TEXT("SlagImshow")

static ATOM windowAtom;
static std::mutex _mutex;
static HINSTANCE const _hInstance = GetModuleHandle(NULL);
static volatile std::atomic<double> _scale = 1.0;

typedef std::lock_guard<std::mutex> AutoLock;

LRESULT CALLBACK SlagWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

struct ImageContainer
{
	ImageContainer():w(0), h(0), data(), type(SlagImageType::GREY) {}
	int w;
	int h;
	std::vector<unsigned char> data;
    SlagImageType type;
};

class WindowWrapper
{
public:
	WindowWrapper(const std::string& name)
    :   _image(), _moduleId(name.c_str()), _name(name.begin(), name.end()),
        _hwnd(NULL), actual_height(1), actual_width(1), _running(true)
    {
        _thread.reset(new std::thread(&ThreadProc, this));
    }
    WindowWrapper(const slag::ModuleIdentifier& id)
        : _image(), _moduleId(id), _name((std::string)id),
        _hwnd(NULL), actual_height(1), actual_width(1), _running(true)
    {
        _thread.reset(new std::thread(&ThreadProc, this));
    }
	~WindowWrapper();

    void Start()
    {
        if (!_running)
        {
            _running = true;
            if (_thread && _thread->joinable())
                _thread->join();
            _thread.reset(new std::thread(&ThreadProc, this));
        }
    }

	ImageContainer _image;
    const slag::ModuleIdentifier _moduleId;
#ifdef UNICODE
	const std::wstring _name;
#else
    const std::string _name;
#endif
	HWND _hwnd;
	std::mutex _mutex;
	int actual_width;
	int actual_height;
    int bitdepth;
    int planes;
    bool _running;
    void SavePos()
    {
        RECT rect;
        if (GetWindowRect(_hwnd, &rect))
        {
            std::ofstream ofs(_name + TEXT(".img.pos"));
            if (ofs)
            {
                ofs << rect.top << " " << rect.left;
            }
        }
    }
private:
    std::unique_ptr<std::thread> _thread;
    //! Initialize and start WndProc message loop
    static void ThreadProc(WindowWrapper* self);
};

typedef std::unordered_map<slag::ModuleIdentifier, WindowWrapper> WindowsType;
static WindowsType _images;

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
            CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            const auto self = reinterpret_cast<WindowWrapper*>(pCreate->lpCreateParams);
            if (self)
            {
                HDC hdc = GetDC(hwnd);
                self->bitdepth = GetDeviceCaps(hdc, BITSPIXEL);
                self->planes = GetDeviceCaps(hdc, PLANES);
                ReleaseDC(hwnd, hdc);
            }
		}
        return 0;
	case WM_CLOSE:
		{
        const auto self = reinterpret_cast<WindowWrapper*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        AutoLock imageLock(self->_mutex);
        self->SavePos();
        DestroyWindow(hwnd);
		}break;
	case WM_PAINT:
		{
        const auto self = reinterpret_cast<WindowWrapper*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        auto& wind = *self;
        AutoLock imageLock(wind._mutex);

        if (wind.actual_width != wind._image.w || wind.actual_height != wind._image.h)
        {
            const double scale = _scale;
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0,
                (int)std::ceil(scale * (wind._image.w)), (int)std::ceil(scale * (wind._image.h)),
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER
            );

            wind.actual_width = wind._image.w;
            wind.actual_height = wind._image.h;
        }
        else
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HDC hdcBuffer = CreateCompatibleDC(hdc);

            HBITMAP hbm = CreateBitmap(wind._image.w, wind._image.h, wind.planes, wind.bitdepth, wind._image.data.data());

            HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);
            const double scale = _scale;
            StretchBlt(
                hdc, 0, 0,
                (int)std::ceil(scale * (wind._image.w)), (int)std::ceil(scale * (wind._image.h)),
                hdcBuffer, 0, 0,
                wind._image.w, wind._image.h,
                SRCCOPY);

            SelectObject(hdcBuffer, hbmOldBuffer);
            DeleteObject(hbm);
            DeleteDC(hdcBuffer);

            EndPaint(hwnd, &ps);
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

void WindowWrapper::ThreadProc(WindowWrapper* self)
{
    MSG Msg;
    std::ifstream ifs(self->_name + TEXT(".img.pos"));
    int top_corner = 0, left_corner = 0;
    if (ifs)
    {
        ifs >> top_corner >> left_corner;
    }

    self->_hwnd = CreateWindowEx(
        0,
        SLAG_WINDOW_CLASS_NAME,
        self->_name.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, self->actual_width, self->actual_height,
        NULL, NULL, _hInstance, self);

    if (!self->_hwnd)
    {
        auto error = GetLastError();
        TCHAR error_str[20];
        wsprintf(error_str, TEXT("Error %d!"), error);
        MessageBox(NULL, TEXT("CreateWindow Failed!"), error_str, MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    SetWindowLongPtr(self->_hwnd, GWLP_USERDATA, (LONG_PTR)self);

    ShowWindow(self->_hwnd, SW_SHOWNORMAL);

    SetWindowPos(self->_hwnd, HWND_NOTOPMOST, left_corner, top_corner,
        self->actual_width, self->actual_height,
        SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER
    );

    while (GetMessage(&Msg, self->_hwnd, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    self->_hwnd = NULL;
    self->_running = false;
}

WindowWrapper::~WindowWrapper()
{
    // prevents the _thread to begin an other loop

    // if GetMessage is still waiting, then wakes it up
	PostMessage(_hwnd, WM_NOTIFY, 0, 0);
    if (_thread &&_thread->joinable())
    	_thread->join();
}

void handle_output_image( const slag::ModuleIdentifier& module_id, int w, int h, SlagImageType type, const unsigned char* data )
{
	const auto picture_size = (std::intmax_t)w * h * GetByteDepth(type);

	if (data && w > 0 && h > 0 && picture_size > 0)
	{
		_mutex.lock();
		//for (auto it = _images.begin(); it != _images.end(); it = (it->second._run ? ++it : _images.erase(it)))
		//{
		//	// Purge windows that are not running
		//}
        auto it = _images.find(module_id);
		if (it == _images.end())
		{
            // starts WndProc
			it = _images.emplace(module_id, module_id).first;
		}

        auto& wind = it->second;
		AutoLock imageLock(wind._mutex);
		_mutex.unlock();
        
        wind.Start();

        wind._image.w = w;
        wind._image.h = h;
        wind._image.type = type;
        wind._image.data.assign(data, data + picture_size);

		if (wind._hwnd)
			InvalidateRect(wind._hwnd, 0, 0);
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
