#include <Windows.h>
#include <deque>
#include <iostream>
#include <random>
#include "kalman_demo.cpp"

// 用于保存鼠标位置和时间
struct MousePosition {
    int x;
    int y;
    ULONGLONG timestamp;  // 保存获取鼠标位置时的时间戳
};

std::deque<MousePosition> mousePositions;  // 保存最近的鼠标位置历史记录

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0, 5);  // 平均值为0，标准差为5

KalmanFilter kf_x(0, 0, 100, 100, 0.5, 25);
KalmanFilter kf_y(0, 0, 100, 100, 0.5, 25);
// 测试函数
void test_function(ULONGLONG greenTimestamp, int& guessedRedX, int& guessedRedY) {

    // 测试函数运行
    kf_x.update(mousePositions.back().x, 0.01);
    kf_y.update(mousePositions.back().y, 0.01);
    guessedRedX = kf_x.getPosition() + 0.2 * kf_x.getVelocity();
    guessedRedY = kf_y.getPosition() + 0.2 * kf_y.getVelocity();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 10, nullptr);  // 设置定时器，每10毫秒触发一次
        break;
    case WM_TIMER: {
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hwnd, &cursorPos);

        // 生成高斯噪声
        int noiseX = static_cast<int>(distribution(generator));
        int noiseY = static_cast<int>(distribution(generator));

        // 将当前鼠标位置和高斯噪声加入历史记录
        mousePositions.push_front({ cursorPos.x + noiseX, cursorPos.y + noiseY, GetTickCount64() });

        // 删除超过1秒的历史记录
        while (!mousePositions.empty() && GetTickCount64() - mousePositions.back().timestamp > 300) {
            mousePositions.pop_back();
        }

        InvalidateRect(hwnd, nullptr, TRUE);  // 使窗口无效，触发WM_PAINT消息
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 获取窗口客户区域的大小
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        // 用背景色填充整个客户区域
        HBRUSH bgBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        FillRect(hdc, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        // 设置画刷为红色
        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(hdc, redBrush);

        // 设置绘制模式为实心
        SetBkMode(hdc, TRANSPARENT);

        // 画一个红色的实心点在当前鼠标位置
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hwnd, &cursorPos);
        Ellipse(hdc, cursorPos.x - 5, cursorPos.y - 5, cursorPos.x + 5, cursorPos.y + 5);

        // 设置画刷为绿色
        HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
        SelectObject(hdc, greenBrush);

        // 获取0.3秒前的鼠标位置，画一个绿色的实心点
        if (!mousePositions.empty()) {
            MousePosition greenPos = mousePositions.back();
            Ellipse(hdc, greenPos.x - 5, greenPos.y - 5, greenPos.x + 5, greenPos.y + 5);

            // 调用测试函数获取预测的红点位置
            int guessedRedX, guessedRedY;
            test_function(greenPos.timestamp, guessedRedX, guessedRedY);

            // 设置画刷为蓝色
            HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
            SelectObject(hdc, blueBrush);

            // 画一个蓝色的实心点在预测的红点位置
            Ellipse(hdc, guessedRedX - 5, guessedRedY - 5, guessedRedX + 5, guessedRedY + 5);

            // 恢复画刷
            DeleteObject(SelectObject(hdc, GetStockObject(DC_BRUSH)));
        }

        // 恢复画刷和绘制模式
        DeleteObject(SelectObject(hdc, GetStockObject(DC_BRUSH)));
        SetBkMode(hdc, OPAQUE);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1);  // 销毁定时器
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int main() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"PointsWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        L"PointsWindowClass",
        L"滤波器测试",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 200,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (hwnd == nullptr) {
        MessageBox(nullptr, L"窗口创建失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
