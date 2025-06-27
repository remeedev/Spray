#include <windows.h>
#include <stdio.h>
#include <sys/time.h>

#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/level_loader.h"

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

// Store screen size
int screen_width;
int screen_height;

void get_screen_dimensions(HWND hwnd){
    RECT rect;
    if (GetClientRect(hwnd, &rect)){
        screen_height = rect.bottom-rect.top;
        screen_width = rect.right-rect.left;
        // UpdateMapBoundaries(collisions, screen_width, screen_height);
    }
}

// Define the window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: ;
            // Screen buffering fix
            StartGraphics(hWnd);
            InitPlayer();
            LoadBrushes();
            Resize(hWnd, screen_width, screen_height);
            loadLevel("./levels/dojo.txt");
            break;
        case WM_SIZE:
            get_screen_dimensions(hWnd);
            Resize(hWnd, screen_width, screen_height);
            break;
        case WM_CLOSE:
            printf("Closing window...\n");
            onEnd();
            running = FALSE;
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            printf("Destroying the window!\n");
            running = FALSE;
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: ;
            // 1280x720
            Draw(hWnd, screen_width, screen_height, paused);
            break;
        case WM_KEYDOWN: ;
            HandleKeyDown(wParam);
            UIKeyDown(wParam, hWnd);
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

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";

    if (!RegisterClassEx(&wc)) {
        printf("Could't register class\n");
        return 0;
    }

    RECT screen_dimensions;
    GetClientRect(GetDesktopWindow(), &screen_dimensions);
    screen_width = (int)((((float)screen_dimensions.right)/1920.0f)*1080.0f);
    screen_height = (int)((((float)screen_width)/16.0) * 9.0);

    HWND hWnd = CreateWindowEx(
        0,
        "WindowClass",
        "My Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        screen_width,
        screen_height,
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
        // Redraw
        if (!paused) Update(dt);
        InvalidateRect(hWnd, NULL, FALSE);
        gettimeofday(&start, NULL);
    }
    printf("Exiting the code...\n");
    return (int)msg.wParam;
}