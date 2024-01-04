#include <Windows.h>
#include <deque>
#include <iostream>
#include <random>
#include "kalman_demo.cpp"

// ���ڱ������λ�ú�ʱ��
struct MousePosition {
    int x;
    int y;
    ULONGLONG timestamp;  // �����ȡ���λ��ʱ��ʱ���
};

std::deque<MousePosition> mousePositions;  // ������������λ����ʷ��¼

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0, 5);  // ƽ��ֵΪ0����׼��Ϊ5

KalmanFilter kf_x(0, 0, 100, 100, 0.5, 25);
KalmanFilter kf_y(0, 0, 100, 100, 0.5, 25);
// ���Ժ���
void test_function(ULONGLONG greenTimestamp, int& guessedRedX, int& guessedRedY) {

    // ���Ժ�������
    kf_x.update(mousePositions.back().x, 0.01);
    kf_y.update(mousePositions.back().y, 0.01);
    guessedRedX = kf_x.getPosition() + 0.2 * kf_x.getVelocity();
    guessedRedY = kf_y.getPosition() + 0.2 * kf_y.getVelocity();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 10, nullptr);  // ���ö�ʱ����ÿ10���봥��һ��
        break;
    case WM_TIMER: {
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hwnd, &cursorPos);

        // ���ɸ�˹����
        int noiseX = static_cast<int>(distribution(generator));
        int noiseY = static_cast<int>(distribution(generator));

        // ����ǰ���λ�ú͸�˹����������ʷ��¼
        mousePositions.push_front({ cursorPos.x + noiseX, cursorPos.y + noiseY, GetTickCount64() });

        // ɾ������1�����ʷ��¼
        while (!mousePositions.empty() && GetTickCount64() - mousePositions.back().timestamp > 300) {
            mousePositions.pop_back();
        }

        InvalidateRect(hwnd, nullptr, TRUE);  // ʹ������Ч������WM_PAINT��Ϣ
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // ��ȡ���ڿͻ�����Ĵ�С
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        // �ñ���ɫ��������ͻ�����
        HBRUSH bgBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        FillRect(hdc, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        // ���û�ˢΪ��ɫ
        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(hdc, redBrush);

        // ���û���ģʽΪʵ��
        SetBkMode(hdc, TRANSPARENT);

        // ��һ����ɫ��ʵ�ĵ��ڵ�ǰ���λ��
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hwnd, &cursorPos);
        Ellipse(hdc, cursorPos.x - 5, cursorPos.y - 5, cursorPos.x + 5, cursorPos.y + 5);

        // ���û�ˢΪ��ɫ
        HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
        SelectObject(hdc, greenBrush);

        // ��ȡ0.3��ǰ�����λ�ã���һ����ɫ��ʵ�ĵ�
        if (!mousePositions.empty()) {
            MousePosition greenPos = mousePositions.back();
            Ellipse(hdc, greenPos.x - 5, greenPos.y - 5, greenPos.x + 5, greenPos.y + 5);

            // ���ò��Ժ�����ȡԤ��ĺ��λ��
            int guessedRedX, guessedRedY;
            test_function(greenPos.timestamp, guessedRedX, guessedRedY);

            // ���û�ˢΪ��ɫ
            HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
            SelectObject(hdc, blueBrush);

            // ��һ����ɫ��ʵ�ĵ���Ԥ��ĺ��λ��
            Ellipse(hdc, guessedRedX - 5, guessedRedY - 5, guessedRedX + 5, guessedRedY + 5);

            // �ָ���ˢ
            DeleteObject(SelectObject(hdc, GetStockObject(DC_BRUSH)));
        }

        // �ָ���ˢ�ͻ���ģʽ
        DeleteObject(SelectObject(hdc, GetStockObject(DC_BRUSH)));
        SetBkMode(hdc, OPAQUE);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1);  // ���ٶ�ʱ��
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
        L"�˲�������",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 200,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (hwnd == nullptr) {
        MessageBox(nullptr, L"���ڴ���ʧ�ܣ�", L"����", MB_OK | MB_ICONERROR);
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
