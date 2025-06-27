#ifndef LevelLoader
#define LevelLoader

void loadLevel(char *file_name);

void UIKeyDown(UINT key, HWND hWnd);
int paused;

void Draw(HWND hWnd, int screen_width, int screen_height, int paused);

void Resize(HWND hWnd, int screen_width, int screen_height);

void StartGraphics(HWND hwnd);

void Update(float dt);

void onEnd();

#endif