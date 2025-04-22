#include <windows.h>

#define ID_CLEAR 201
#define ID_CANCEL 202
#define ID_ENTER 203
#define ID_OK     204

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddControls(HWND);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"ATMWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc)) return -1;

    CreateWindowW(L"ATMWindowClass", L"MUAZ Bank ATM", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  100, 100, 500, 700, NULL, NULL, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void CreateButton(HWND hWnd, LPCWSTR text, int x, int y, int width, int height, int id, COLORREF color = RGB(255,255,255)) {
    HWND hBtn = CreateWindowW(L"Button", text, WS_VISIBLE | WS_CHILD,
                              x, y, width, height, hWnd, (HMENU)id, NULL, NULL);
    SendMessage(hBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void AddControls(HWND hWnd) {
    // Title
    CreateWindowW(L"Static", L"MUAZ Bank", WS_VISIBLE | WS_CHILD | SS_CENTER,
                  150, 20, 200, 40, hWnd, NULL, NULL, NULL);

    // Display screen
    CreateWindowW(L"Static", L"", WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
                  90, 80, 300, 200, hWnd, NULL, NULL, NULL);

    // Left buttons
    int yPos = 100;
    for (int i = 0; i < 4; i++) {
        CreateButton(hWnd, L"", 50, yPos, 30, 30, 100 + i);
        yPos += 40;
    }

    // Right buttons
    yPos = 100;
    for (int i = 0; i < 4; i++) {
        CreateButton(hWnd, L"", 400, yPos, 30, 30, 200 + i);
        yPos += 40;
    }

    // Numeric Keypad
    int id = 1, xStart = 100, yStart = 320;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            wchar_t num[2] = {L'0' + id, 0};
            CreateButton(hWnd, num, xStart + col * 60, yStart + row * 50, 50, 40, id);
            id++;
        }
    }

    // Last row
    CreateButton(hWnd, L".", xStart, yStart + 3 * 50, 50, 40, 250);
    CreateButton(hWnd, L"0", xStart + 60, yStart + 3 * 50, 50, 40, 0);
    CreateButton(hWnd, L"", xStart + 120, yStart + 3 * 50, 50, 40, 260); // empty

    // Function Buttons
    CreateButton(hWnd, L"Clear", 300, 320, 80, 40, ID_CLEAR);
    CreateButton(hWnd, L"Cancel", 300, 370, 80, 40, ID_CANCEL);
    CreateButton(hWnd, L"Enter", 300, 420, 80, 40, ID_ENTER);
    CreateButton(hWnd, L"OK",    300, 470, 80, 40, ID_OK);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            AddControls(hWnd);
            break;
        case WM_COMMAND:
            switch (wp) {
                case ID_CLEAR:
                    MessageBoxW(hWnd, L"Clear pressed", L"Info", MB_OK);
                    break;
                case ID_CANCEL:
                    MessageBoxW(hWnd, L"Transaction Cancelled", L"Info", MB_OK);
                    break;
                case ID_ENTER:
                    MessageBoxW(hWnd, L"Enter pressed", L"Info", MB_OK);
                    break;
                case ID_OK:
                    MessageBoxW(hWnd, L"OK pressed", L"Info", MB_OK);
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}