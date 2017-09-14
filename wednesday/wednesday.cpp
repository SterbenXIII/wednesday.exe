#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include "wednesday.h"
#include "bitmap.h"
#include "resources.h"

#include <map>
#include <string>
#include <time.h>
#include <algorithm>
#include <windows.h>

const wchar_t kClassName[] = L"Wednesday";

const double kUnitTime = 60000 / 145.0;

const double kStep = 5.5 / 72.0;

const double kXOff[] = { 0, 1 * kStep, 2 * kStep, 3 * kStep, 4 * kStep, 5 * kStep,
                         6 * kStep, 7 * kStep, 8 * kStep, 9 * kStep, 10 * kStep,
                         10 * kStep, 9 * kStep, 8 * kStep, 7 * kStep, 6 * kStep,
                         5 * kStep, 4 * kStep, 3 * kStep, 2 * kStep, 1 * kStep, 0,
                         0, 0, 3 * kStep, 3 * kStep, 6 * kStep, 6 * kStep,
                         8 * kStep, 8 * kStep, 10 * kStep,
                         -1, -1, -1, -1, -1, -1, -1, -1, -1 };

const double kYOff[] = { 0, 1 * kStep, 2 * kStep, 3 * kStep, 4 * kStep, 5 * kStep,
                         6 * kStep, 7 * kStep, 8 * kStep, 9 * kStep, 10 * kStep,
                         0, 1 * kStep, 2 * kStep, 3 * kStep, 4 * kStep, 5 * kStep,
                         6 * kStep, 7 * kStep, 8 * kStep, 9 * kStep, 10 * kStep,
                         2 * kStep, 6 * kStep, 4 * kStep, 8 * kStep, 2 * kStep, 6 * kStep,
                         4 * kStep, 8 * kStep, 5 * kStep,
                         -1, -1, -1, -1, -1, -1, -1, -1, -1 };

const double timings[] = { 2.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.5,
                           2.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.5,
                           2.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 2.5,
                           2.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 2.5 };

const double kSizeX[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1.3, 1.3, 1.7, 1.7, 2.2, 2.2, 2.8, 2.8 };

const double kSizeY[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1.3, 1.3, 1.7, 1.7, 2.2, 2.2, 2.8 };

const int kBaseSizeX = (int) (GetSystemMetrics(SM_CXSCREEN) / 6.0);
const int kBaseSizeY = (int) (GetSystemMetrics(SM_CXSCREEN) / 8.0);

const int kFrameCount = 11;
const int kFrameInterval = 33;

static HBITMAP bitmaps[kFrameCount];

std::map<HWND, int> lastFrameIndex;
std::map<HWND, long> lastSwitch;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    unsigned char* bitmapBytes[] = { __0_bmp, __1_bmp, __2_bmp, __3_bmp, __4_bmp, __5_bmp, __6_bmp, __7_bmp, __8_bmp, __9_bmp, __10_bmp };
    std::transform(bitmapBytes, bitmapBytes + kFrameCount, bitmaps, LoadBitmapFromBytes);

    // Register the window class.
    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kClassName;

    RegisterClass(&wc);

    int resX = GetSystemMetrics(SM_CXSCREEN);
    int resY = GetSystemMetrics(SM_CYSCREEN);

    const int totalWindows = sizeof(timings) / sizeof(double);
    HWND windows[totalWindows] = {};

    long lastWindow = 0;

    int i = 0;
    while (true) {
        if (i == 0 || TimeMillis() - lastWindow >= timings[i - 1] * kUnitTime) {
            if (i == totalWindows) {
                break;
            }

            int width = (int) (kSizeX[i] * kBaseSizeX);
            int height = (int) (kSizeY[i] * kBaseSizeY);

            int x;
            int y;
            if (kXOff[i] == -1) {
                x = resX / 2 - width / 2;
            } else {
                x = (int) (kXOff[i] * resX);
            }
            if (kYOff[i] == -1) {
                y = resY / 2 - height / 2;
            } else {
                y = (int) (kYOff[i] * resY);
            }
            windows[i] = MakeFrog(hInst, kClassName, x, y, width, height, nShowCmd);
            lastWindow = TimeMillis();
            i++;
        }

        for (int i = 0; i < totalWindows; i++) {
            HWND hwnd = windows[i];
            if (hwnd != NULL) {
                if (lastFrameIndex[hwnd] < kFrameCount - 1 && TimeMillis() - lastSwitch[hwnd] >= kFrameInterval) {
                    AdvanceFrame(hwnd);
                }
            }
        }
    }

    return 0;
}

inline long TimeMillis() {
    SYSTEMTIME time;
    GetSystemTime(&time);
    return (time.wSecond * 1000) + time.wMilliseconds;
}

HWND MakeFrog(HINSTANCE hInst, LPCTSTR className, int x, int y, int w, int h, int nCmdShow) {
    HWND hwnd = CreateWindowEx(NULL, kClassName, L"My Dudes", WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, hInst, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, std::to_wstring(GetLastError()).c_str(), L"Failed to Launch", MB_ICONERROR);
        return NULL;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return hwnd;
}

void AdvanceFrame(HWND hwnd) {
    lastFrameIndex[hwnd] = lastFrameIndex[hwnd] + 1;
    lastSwitch[hwnd] = TimeMillis();
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        lastFrameIndex[hwnd] = 0;
        lastSwitch[hwnd] = TimeMillis();

        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT     ps;
        HDC             hdc;
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;

        hdc = BeginPaint(hwnd, &ps);

        HBITMAP hbmp = bitmaps[lastFrameIndex[hwnd]];

        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hbmp);

        GetObject(hbmp, sizeof(bitmap), &bitmap);
        RECT rect;
        GetWindowRect(hwnd, &rect);
        StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top - 38, hdcMem, 0, 0,
                bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

        EndPaint(hwnd, &ps);

        break;
    }
    default:
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }

    return 0;
}
