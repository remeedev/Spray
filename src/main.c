#include <windows.h>
#include <stdio.h>
#include <sys/time.h>

#include "headers/drawing.h"
#include "headers/movement.h"

void get_mouse_pos(HWND hWnd, LPPOINT out){
    // Adding default values
    out->x = 0;
    out->y = 0;

    POINT mouse_pos;
    if (!GetCursorPos(&mouse_pos)){
        return;
    }
    if (!ScreenToClient(hWnd, &mouse_pos)){
        return;
    }
    out->x = mouse_pos.x;
    out->y = mouse_pos.y;
}

// Inner game loop
int running = TRUE;

// Delete later (move to another file)

SpriteGroup *collisions = NULL;
POINT p1, p2;

// =================================

// Screen buffering (fix for flickering)
HBITMAP hbmMem = NULL;
HDC hdcMem = NULL;
HBITMAP hbmOld = NULL;

// Store screen size
int screen_width = 1080;
int screen_height = 720;

void get_screen_dimensions(HWND hwnd){
    RECT rect;
    if (GetClientRect(hwnd, &rect)){
        screen_height = rect.bottom-rect.top;
        screen_width = rect.right-rect.left;
        UpdateMapBoundaries(collisions, screen_width, screen_height);
        UpdatePositionOnResize(screen_width, screen_height);
    }
}

// Define the window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: ;
            // Delete Later
            p1.x = -1;
            p2.x = -1;
            // ================
            printf("Creating screen...\n");
            // Screen buffering fix
            printf("Creating character...\n");
            InitPlayer();
            printf("Loading brushes!\n");
            LoadBrushes();
            printf("Creating map boundaries!\n");
            collisions = CreateMapBoundaries(100, screen_width, screen_height);
            printf("Loading level assets!\n");
            AppendToGroup(collisions, CreateStairsWithCoords(100, screen_height-100, 400, 400));
            CreateSpriteInGroup(collisions, screen_width-500, 300, 100, 200, RGB(0, 0, 255));
            CreateSpriteInGroup(collisions, screen_width-100, 300, 100, screen_height-600, RGB(0, 0, 255));
            printf("Setting double buffer...\n");
            HDC hdc = GetDC(hWnd);
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, screen_width, screen_height);
            hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            ReleaseDC(hWnd, hdc);
            break;
        case WM_SIZE:
            get_screen_dimensions(hWnd);
            if (hdcMem) {
                // Clean up old bitmap
                SelectObject(hdcMem, hbmOld);
                DeleteObject(hbmMem);

                HDC hdc = GetDC(hWnd);
                hbmMem = CreateCompatibleBitmap(hdc, screen_width, screen_height);
                hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
                ReleaseDC(hWnd, hdc);
            }
            break;
        case WM_CLOSE:
            printf("Closing window...\n");
            running = FALSE;
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            printf("Destroying the window!\n");
            running = FALSE;
            if (hdcMem && hbmOld) SelectObject(hdcMem, hbmOld);
            if (hdcMem) DeleteDC(hdcMem);
            if (hbmMem) DeleteObject(hbmMem);
            EndPlayer();
            deleteBrushes();
            DeleteSpriteGroup(collisions);
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: ;
            PAINTSTRUCT ps;
            HDC hdcW = BeginPaint(hWnd, &ps);
            FillRect(hdcMem, &ps.rcPaint, (HBRUSH)(BLACK_BRUSH));
            PaintSpriteGroup(hdcMem, collisions);
            PaintSprite(hdcMem, GetPlayerPtr());
            BitBlt(hdcW, 0, 0, screen_width, screen_height, hdcMem, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
            break;
        case WM_LBUTTONDOWN: ;
            // Messing with staircase function
            // POINT mouse_pos;
            // get_mouse_pos(hWnd, &mouse_pos);
            // if (p1.x == -1){
            //     p1 = mouse_pos;
            // }else{
            //     if (p2.x == -1){
            //         p2 = mouse_pos;
            //         int dy = p2.y - p1.y;
            //         p2.y -= dy%25;
            //         AppendToGroup(collisions, CreateStairCase(p1, p2));
            //     }
            // }
            break;
        case WM_KEYDOWN: ;
            HandleKeyDown(wParam);
            break;
        case WM_KEYUP: ;
            HandleKeyUp(wParam);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;

    printf("Registering classes...\n");
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";

    if (!RegisterClassEx(&wc)) {
        printf("Could't register class\n");
        return 0;
    }

    printf("Creating window...\n");
    HWND hWnd = CreateWindowEx(
        0,
        "WindowClass",
        "My Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1080,
        720,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        printf("Could not create window!\n");
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    MSG msg;
    struct timeval end, start;
    gettimeofday(&start, NULL);
    while(running){
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT){
                running = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running){
            break;
        }
        gettimeofday(&end, NULL);
        float dt = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0f;
        UpdatePosition(dt, collisions);
        // Redraw
        InvalidateRect(hWnd, NULL, FALSE);
        gettimeofday(&start, NULL);
    }
    printf("Exiting the code...\n");
    return (int)msg.wParam;
}