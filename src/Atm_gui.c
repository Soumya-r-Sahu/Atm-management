#include <windows.h>
#include <wchar.h>
#include <stdio.h>

#define ID_CLEAR 201
#define ID_CANCEL 202
#define ID_ENTER 203
#define ID_OK     204

// IDs for left and right buttons
#define ID_LB1 301
#define ID_LB2 302
#define ID_LB3 303
#define ID_LB4 304
#define ID_RB1 401
#define ID_RB2 402
#define ID_RB3 403
#define ID_RB4 404

// Global variables
wchar_t inputBuffer[20] = L""; // Buffer to store input from the numpad
HWND hDisplay;                // Display screen for showing input

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddControls(HWND);
void HandleNumpadInput(HWND, int value);
void UpdateDisplay(const wchar_t *message);
void HandleClearButton();
void HandleCancelButton();
void HandleEnterButton();
void HandleOkButton();
void HandleLeftButton(int id);
void HandleRightButton(int id);

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

void CreateButton(HWND hWnd, LPCWSTR text, int x, int y, int width, int height, int id) {
    HWND hBtn = CreateWindowW(L"Button", text, WS_VISIBLE | WS_CHILD,
                              x, y, width, height, hWnd, (HMENU)id, NULL, NULL);
    SendMessage(hBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void AddControls(HWND hWnd) {
    // Title
    CreateWindowW(L"Static", L"MUAZ Bank", WS_VISIBLE | WS_CHILD | SS_CENTER,
                  150, 20, 200, 40, hWnd, NULL, NULL, NULL);

    // Display screen
    hDisplay = CreateWindowW(L"Static", L"", WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
                             90, 80, 300, 200, hWnd, NULL, NULL, NULL);

    // Left buttons
    int yPos = 100;
    for (int i = 0; i < 4; i++) {
        CreateButton(hWnd, L"", 50, yPos, 30, 30, ID_LB1 + i);
        yPos += 40;
    }

    // Right buttons
    yPos = 100;
    for (int i = 0; i < 4; i++) {
        CreateButton(hWnd, L"", 400, yPos, 30, 30, ID_RB1 + i);
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

void HandleNumpadInput(HWND hWnd, int value) {
    if (wcslen(inputBuffer) < 19) { // Ensure the buffer doesn't overflow
        wchar_t num[2];
        swprintf(num, sizeof(num) / sizeof(wchar_t), L"%d", value); // Corrected usage
        wcscat(inputBuffer, num); // Append the pressed number to the buffer
        UpdateDisplay(inputBuffer);
        wprintf(L"Numpad Input: %s\n", inputBuffer);
    }
}

void UpdateDisplay(const wchar_t *message) {
    SetWindowTextW(hDisplay, message); // Update the display with the current message
}

void HandleClearButton() {
    wcscpy(inputBuffer, L""); // Clear the input buffer
    UpdateDisplay(L"Input cleared.");
    wprintf(L"Clear Button Pressed\n");
}

void HandleCancelButton() {
    UpdateDisplay(L"Transaction cancelled.");
    wprintf(L"Cancel Button Pressed\n");
}

void HandleEnterButton() {
    UpdateDisplay(L"Enter pressed.");
    wprintf(L"Enter Button Pressed\n");
}

void HandleOkButton() {
    UpdateDisplay(L"OK pressed.");
    wprintf(L"OK Button Pressed\n");
}

void HandleLeftButton(int id) {
    wchar_t message[50]; // Increased buffer size to avoid overflow
    swprintf(message, sizeof(message) / sizeof(wchar_t), L"Left Button %d pressed", id - ID_LB1 + 1); // Corrected usage
    UpdateDisplay(message);
    wprintf(L"%s\n", message);
}

void HandleRightButton(int id) {
    wchar_t message[50]; // Increased buffer size to avoid overflow
    swprintf(message, sizeof(message) / sizeof(wchar_t), L"Right Button %d pressed", id - ID_RB1 + 1); // Corrected usage
    UpdateDisplay(message);
    wprintf(L"%s\n", message);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            AddControls(hWnd);
            break;
        case WM_COMMAND:
            if (wp >= ID_LB1 && wp <= ID_LB4) {
                HandleLeftButton(wp);
            } else if (wp >= ID_RB1 && wp <= ID_RB4) {
                HandleRightButton(wp);
            } else if (wp >= 0 && wp <= 9) { // Handle numeric keypad input
                HandleNumpadInput(hWnd, wp);
            } else {
                switch (wp) {
                    case ID_CLEAR:
                        HandleClearButton();
                        break;
                    case ID_CANCEL:
                        HandleCancelButton();
                        break;
                    case ID_ENTER:
                        HandleEnterButton();
                        break;
                    case ID_OK:
                        HandleOkButton();
                        break;
                }
            }
            break;
        case WM_DESTROY:
            // Prevent the application from exiting
            MessageBoxW(hWnd, L"The application cannot be closed.", L"Info", MB_OK | MB_ICONINFORMATION);
            break;
        default:
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}