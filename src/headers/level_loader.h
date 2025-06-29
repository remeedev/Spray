#ifndef LevelLoader
#define LevelLoader

// ADMIN VARS

int showCollisions;
int showDebug;
COLORREF collisionColor;
COLORREF characterColor;

// ADMIN VAR END

void loadLevel(char *file_name);

void UIKeyDown(UINT key, HWND hWnd);
int paused;

void Draw(HWND hWnd, int screen_width, int screen_height, int paused);

void Resize(HWND hWnd, int screen_width, int screen_height);

void StartGraphics(HWND hwnd);

void Update(float dt);

void onEnd();

#endif