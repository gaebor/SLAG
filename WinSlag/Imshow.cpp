
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
#include "SlagTypes.h"

#define STATUS_BAR_ID 103
#define SLAG_WINDOW_CLASS_NAME TEXT("SlagImshow")

#ifdef max
#undef max
#endif

static ATOM windowAtom;
static std::mutex _mutex;
static HINSTANCE const _hInstance = GetModuleHandle(NULL);

typedef std::lock_guard<std::mutex> AutoLock;

// https://stackoverflow.com/a/3999597/3583290
std::string utf8_encode(const wchar_t* wstr, int len = 0)
{
    std::string result;
    if (len == 0)
        len = (int)wcslen(wstr);
    if (len > 0)
    {
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, len, NULL, 0, NULL, NULL);
        result.resize(size_needed);
        if (WideCharToMultiByte(CP_UTF8, 0, wstr, len, &result[0], size_needed, NULL, NULL) == 0)
            result.clear();
    }
    return result;
}

std::string utf8_encode(const std::wstring& wstr)
{
    return utf8_encode(wstr.c_str(), (int)wstr.size());
}

std::wstring utf8_decode(const char* str, int len = 0)
{
    std::wstring result;
    if (len == 0)
        len = (int)strlen(str);
    if (len > 0)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
        result.resize(size_needed);
        MultiByteToWideChar(CP_UTF8, 0, str, len, &result[0], size_needed);
    }
    return result;
}

std::wstring utf8_decode(const std::string& str)
{
    return utf8_decode(str.c_str(), (int)str.size());
}

struct ImageContainer
{
	ImageContainer():w(0), h(0), data(), type(SlagImageType::GREY) {}
	int w;
	int h;
	std::vector<unsigned char> data;
    SlagImageType type;
};

struct Statistics
{
    Statistics(): cycle(0), wait(0), load(0), n(0){}
    double cycle, wait, load;
    size_t n;
    void add(double c, double w, double l)
    {
        const double frac = 1.0 / (n + 1); 
        const double ratio = n*frac;
        
        cycle *= ratio; cycle += frac * c;
        wait *= ratio; wait += frac * w;
        load *= ratio; load += frac * l;
        ++n;
    }
    const char* get_speed()
    {
        sprintf_s(text, "%8.2esec|", cycle);
        return text;
    }
    void reset()
    {
        cycle = load = wait = 0;
        n = 0;
    }
    char text[50];
};

class WindowWrapper
{
public:
    WindowWrapper(const slag::ModuleIdentifier& id)
    :   _image(), _moduleId(id),
        _hwnd(NULL),
        _scale(1), _barHeight(22), _barWidth(100), _done(true)
    {
    }
	~WindowWrapper()
    {
        PostMessage(_hwnd, WM_CLOSE, NULL, NULL);

        if (_thread.joinable())
            _thread.join();
        DeleteObject(_font);
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

    std::mutex _mutex;
    std::string _text;
	ImageContainer _image;
    Statistics _stats;
    const slag::ModuleIdentifier _moduleId;
	HWND _hwnd;
    HFONT _font;
    RECT actual_rect;
    int bitdepth;
    int planes;
    std::atomic<float> _scale;
    int _barHeight, _barWidth;

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

        UpdateWindow(_hwnd);

        _font = CreateFontW(_barHeight-2, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Consolas");
        if (_font)
        {
            RECT r = { 0, 0, 0, 0 };
            char str[] = "9.99e+00sec|";
            HDC hDC = GetDC(_hwnd);
            auto oldFont = SelectObject(hDC, _font);
                        
            DrawTextA(hDC, str, (int)strlen(str), &r, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
            SelectObject(hDC, oldFont);
            ReleaseDC(_hwnd, hDC);

            _barWidth = r.right - r.left;

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
    static HBRUSH hBsh_yellow = ::CreateSolidBrush(RGB(255, 255, 0));
    static HBRUSH hBsh_green = ::CreateSolidBrush(RGB(0, 255, 0));
    static HBRUSH hBsh_white = ::CreateSolidBrush(RGB(255, 255, 255));

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
        AutoLock imageLock(wind._mutex);
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
            RECT box;
            std::wstring wtext;

            HDC hdc = BeginPaint(hwnd, &ps);
            if (!wind._image.data.empty())
            {
                HDC hdcBuffer = CreateCompatibleDC(hdc);
                HBITMAP hbm = CreateBitmap(wind._image.w, wind._image.h, wind.planes, wind.bitdepth, wind._image.data.data());
                HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbm);

                const double scale = wind._scale;
                StretchBlt(
                    hdc, 0, wind._barHeight,
                    (int)std::ceil(scale * (wind._image.w)), (int)std::ceil(scale * (wind._image.h)),
                    hdcBuffer, 0, 0,
                    wind._image.w, wind._image.h,
                    SRCCOPY);

                SelectObject(hdcBuffer, hbmOldBuffer);
                DeleteObject(hbm);
                DeleteDC(hdcBuffer);
            }
            box.top = 0; box.bottom = wind._barHeight;
            box.left = 0;
            box.right = int(wind._barWidth * wind._stats.wait / wind._stats.cycle);
            FillRect(hdc, &box, hBsh_yellow);

            box.left = box.right;
            box.right += int(wind._barWidth * wind._stats.load / wind._stats.cycle);
            FillRect(hdc, &box, hBsh_green);
            
            box.left = box.right;
            box.right = ps.rcPaint.right;
            FillRect(hdc, &box, hBsh_white);

            auto oldFont = SelectObject(hdc, wind._font);
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 0, 1, wind._stats.get_speed(), 19);
            wind._stats.reset();

            //SetBkMode(hdc, OPAQUE);
            wtext = utf8_decode(wind._text);
            TextOutW(hdc, wind._barWidth + 1, 1, wtext.c_str(), (int)wtext.size());
            SelectObject(hdc, oldFont);

            EndPaint(hwnd, &ps);
        }
    }break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    //case WM_SIZE:
    //    if (wind._status)
    //        PostMessage(wind._status, WM_SIZE, wParam, lParam);
    //    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//! this inserts a new Wrapper if needed or fetches an already existing one
/*!
locks until the elements is inserted
*/
static WindowWrapper& insert_wrapper(const slag::ModuleIdentifier& module_id)
{
    AutoLock lock(_mutex);
    return _images.emplace(module_id, module_id).first->second;
}

void handle_output_image(
    const slag::ModuleIdentifier& module_id,
    const SlagTextOut& text,
    const SlagImageOut& image,
    const slag::Stats& stats)
{
    WindowWrapper& wind = insert_wrapper(module_id);
    // starts ThreadProc in case it was closed but once it was alive
    wind.Start();
    {   // copies the image to internal memory, locks WM_PAINT until that
        AutoLock lock(wind._mutex);
        const auto picture_size = (std::intmax_t)image.w * image.h * GetByteDepth(image.type);
        if (image.data && picture_size > 0)
        {
            wind._image.w = image.w;
            wind._image.h = image.h;
            wind._image.type = image.type;
            wind._image.data.assign(image.data, image.data + picture_size);
            // Queues a PAINT to message loop
        }
        if (text.str)
            wind._text.assign(text.str, text.str + text.size);
        wind._stats.add(stats.cycle, stats.wait, stats.load);
        InvalidateRect(wind._hwnd, 0, 0);
    }
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
