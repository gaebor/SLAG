
#include <windows.h>
#include <commctrl.h>

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

#define STATUS_BAR_ID 103
#define SLAG_WINDOW_CLASS_NAME TEXT("SlagImshow")

static ATOM windowAtom;
static std::mutex _mutex;
static HINSTANCE const _hInstance = GetModuleHandle(NULL);

typedef std::lock_guard<std::mutex> AutoLock;

struct ImageContainer
{
	ImageContainer():w(200), h(0), data(), type(SlagImageType::GREY) {}
	int w;
	int h;
	std::vector<unsigned char> data;
    SlagImageType type;
    std::mutex _mutex;
};

class WindowWrapper
{
public:
    WindowWrapper(const slag::ModuleIdentifier& id)
    :   _image(), _moduleId(id),
        _hwnd(NULL), _status(NULL),
        _scale(1), _barHeight(20), _done(true)
    {
    }
	~WindowWrapper()
    {
        PostMessage(_hwnd, WM_CLOSE, NULL, NULL);

        if (_thread.joinable())
            _thread.join();
    }
    //! restarts the WndProc loop only if it doesn't show any sign of running
    void Start()
    {
        if (_done)
        {   // joins dangling thread
            if (_thread.joinable())
                _thread.join();
            // rerun the whole thing
            _done = false;
            _thread = std::thread(&ThreadProc, this);
        }
    }

	ImageContainer _image;
    std::string _text;
    const slag::ModuleIdentifier _moduleId;
	HWND _hwnd, _status;
    RECT actual_rect;
    int bitdepth;
    int planes;
    std::atomic<float> _scale;
    int _barHeight;

    void LoadPos()
    {
        actual_rect.top = 0;
        actual_rect.left = 0;
        actual_rect.bottom = 260;
        actual_rect.right = 320;

        RECT rect;

        std::ifstream ifs(std::string(_moduleId) + ".img.pos");
        if (ifs)
        {
            ifs >> rect.top >> rect.left >> rect.bottom >> rect.right;
            if (rect.bottom > 0 && actual_rect.right > 0)
            {
                actual_rect = rect;
            }
        }
    }

    void SavePos()const
    {
        WINDOWPLACEMENT wndpl;
        wndpl.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(_hwnd, &wndpl) != 0)
        {
            std::ofstream ofs((std::string)_moduleId + ".img.pos");
            if (ofs)
            {
                ofs << wndpl.rcNormalPosition.top << ' ' << wndpl.rcNormalPosition.left << ' ' <<
                    wndpl.rcNormalPosition.bottom << ' ' << wndpl.rcNormalPosition.right << std::endl;
            }
        }
    }

    static LARGE_INTEGER GetRequiredSize(int w, int h)
    {
        RECT rect;
        LARGE_INTEGER l;

        rect.top = 0;
        rect.left = 0;
        rect.bottom = l.LowPart = h;
        rect.right = l.HighPart = w;
        
        if (AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
        {
            l.LowPart = rect.bottom - rect.top;
            l.HighPart= rect.right - rect.left;
        }
        return l;
    }
private:
    std::thread _thread;
    std::atomic<bool> _done;
    //! Initialize and start WndProc message loop
    static void ThreadProc(WindowWrapper* self)
    {
        MSG Msg;
        if (self->Init())
        {
            while (GetMessage(&Msg, self->_hwnd, 0, 0) > 0)
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
        self->_done = true;
    }
    bool Init()
    {
        DWORD error;
        RECT rect;
        const std::string windowName(_moduleId);

        LoadPos();

        _hwnd = CreateWindowEx(
            0,
            SLAG_WINDOW_CLASS_NAME,
            windowName.c_str(),
            WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
            actual_rect.left, actual_rect.top,
            actual_rect.right - actual_rect.left, actual_rect.bottom - actual_rect.top,
            NULL, NULL, _hInstance, this);

        if (!_hwnd)
        {
            error = GetLastError();
            TCHAR error_str[20];
            wsprintf(error_str, TEXT("Error %d!"), error);
            MessageBox(NULL, TEXT("CreateWindow Failed!"), error_str, MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);

        ShowWindow(_hwnd, SW_SHOWNORMAL);

        _status = CreateStatusWindow(WS_CHILD | WS_VISIBLE, TEXT(""), _hwnd, STATUS_BAR_ID);
        if (!_status)
        {
            error = GetLastError();
            TCHAR error_str[20];
            wsprintf(error_str, TEXT("Error %d!"), error);
            MessageBox(NULL, TEXT("CreateStatusWindow Failed!"), error_str, MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        UpdateWindow(_hwnd);

        if (GetWindowRect(_status, &rect))
        {
            _barHeight = rect.bottom - rect.top;
        }
        return true;
    }
};

typedef std::unordered_map<slag::ModuleIdentifier, WindowWrapper> WindowsType;
static WindowsType _images;

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
    auto& wind = *reinterpret_cast<WindowWrapper*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg)
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
    break;
    case WM_CLOSE:
    {
        wind.SavePos();
        DestroyWindow(hwnd);
    }break;
    case WM_PAINT:
    {
        AutoLock imageLock(wind._image._mutex);
        const double scale = wind._scale;
        const auto img_w = (int)std::ceil(scale * (wind._image.w));
        const auto img_h = (int)std::ceil(scale * (wind._image.h));
        auto required = WindowWrapper::GetRequiredSize(img_w, img_h);
        required.LowPart += wind._barHeight;

        if (wind.actual_rect.bottom - wind.actual_rect.top != required.LowPart 
            || wind.actual_rect.right - wind.actual_rect.left != required.HighPart)
        {
            if (SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, required.HighPart, required.LowPart,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER
                ))
            {
                wind.actual_rect.bottom = wind.actual_rect.top + required.LowPart;
                wind.actual_rect.right = wind.actual_rect.left + required.HighPart;
            }
        }

        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC hdcBuffer = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateBitmap(wind._image.w, wind._image.h, wind.planes, wind.bitdepth, wind._image.data.data());
            HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

            const double scale = wind._scale;
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
    case WM_SIZE:
        if (wind._status)
            PostMessage(wind._status, WM_SIZE, wParam, lParam);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void handle_output_image( const slag::ModuleIdentifier& module_id, int w, int h, SlagImageType type, const unsigned char* data )
{
	const auto picture_size = (std::intmax_t)w * h * GetByteDepth(type);
    WindowWrapper* self;
	if (data && w > 0 && h > 0 && picture_size > 0)
	{
        {   // finds the image in the pool
            AutoLock lock(_mutex);
            auto it = _images.find(module_id);
            if (it == _images.end())
            {
                // starts ThreadProc
                it = _images.emplace(module_id, module_id).first;
            }
            self = &(it->second);
        }
        // starts ThreadProc in case it was closed but once it was alive
        self->Start();
        {   // copies the image to internal memory
            AutoLock lock(self->_image._mutex);
            self->_image.w = w;
            self->_image.h = h;
            self->_image.type = type;
            self->_image.data.assign(data, data + picture_size);
        }
		//if (self->_hwnd)
		InvalidateRect(self->_hwnd, 0, 0);
	}
}

void handle_output_text(const slag::ModuleIdentifier& module_id, const char* text, int length)
{
    WindowWrapper* self;
    if (text)
    {
        {   // finds the image in the pool
            AutoLock lock(_mutex);
            auto it = _images.find(module_id);
            if (it == _images.end())
            {
                // starts ThreadProc
                it = _images.emplace(module_id, module_id).first;
            }
            self = &(it->second);
        }
        // starts ThreadProc in case it was closed but once it was alive
        self->Start();
        SetDlgItemText(self->_hwnd, STATUS_BAR_ID, text);
    }
}

void handle_statistics(const slag::ModuleIdentifier& module_id, double cycle, double load, double wait)
{
}

//
//void configure_output_image(const std::vector<std::string>& params)
//{
//	for (size_t i = 0; i < params.size(); ++i)
//	{
//		if (params[i] == "-s" || params[i] == "--scale" && i + 1 < params.size())
//		{
//			double scale = atof(params[i + 1].c_str());
//			if (scale > 0)
//				_scale= scale;
//		}
//	}
//}

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
