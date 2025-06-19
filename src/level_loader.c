#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/images.h"
#include "headers/npc.h"

// Character collision object
SpriteGroup *collisions = NULL;

// Game Resolution Before Blit
size_t game_res[2] = {1280, 720};

// Screen buffering (fix for flickering)
HBITMAP hbmMem = NULL;
HDC hdcMem = NULL;
HBITMAP hbmOld = NULL;

// Checks file extension
int validExtension(char *file_name){
    size_t ext_size = 0;
    int pos = strlen(file_name)-1;
    while (file_name[pos] != '.'){
        pos--;
        ext_size++;
    }
    char *ext = (char *)malloc(ext_size);
    if (ext == NULL){
        printf("Error allocating space for memory!\n");
        return FALSE;
    }
    int curr_pos = 0;
    pos++;
    while (file_name[pos] != '\0'){
        ext[curr_pos++] = file_name[pos];
        pos++;
    }
    ext[curr_pos] = '\0';
    if (strcmp(ext, "lvl")!=0){
        free(ext);
        return FALSE;
    }
    free(ext);
    return TRUE;
}

// Converts char seq to int
int toInt(char *txt){
    int out = 0;
    int pos = 0;
    int txtLen = strlen(txt);
    int neg = FALSE;
    while (txt[pos] != '\0'){
        if (txt[pos] == '-') {
            neg = TRUE;
            pos++;
            continue;
        }
        double strength = (double)pow((double)10, (double)(txtLen-pos-1)) - neg;
        out = ((double)txt[pos]-48.0)*strength + out;
        pos++;
    }
    if (neg){
        out = -out;
    }
    return out;
}

// Structures for sprites/collisions/Enemies
typedef struct SpriteItem{
    SpriteGroup *group;
    Sprite *sprite;
    struct SpriteItem *next;
} SpriteItem;

// Structure for redirect type object
typedef struct RedirectItem{
    char *redirect;
    struct RedirectItem *next;
} RedirectItem;

// Redirect init
SpriteGroup *redirects = NULL;
RedirectItem *redirect_links = NULL;

Sprite *bg = NULL;

SpriteItem *loaded_sprites = NULL;

void delete_loaded_sprites(){
    SpriteItem *curr = loaded_sprites;
    while (curr != NULL){
        SpriteItem *prev = curr;
        if (curr->group) DeleteSpriteGroup(curr->group);
        if (curr->sprite) EraseSprite(curr->sprite);
        curr=curr->next;
        free(prev);
    }
}

char *GetNext(char *line, int* pos, char stopper){
    size_t outSize = 0;
    int currPos = *pos;
    while (line[currPos+outSize] != stopper && line[currPos+outSize] != '\0')outSize++;
    char *out = (char *)malloc(outSize+1);
    for (size_t i = 0; i < outSize; i++) out[i] = line[currPos+i];
    out[outSize] = '\0';
    *pos += outSize + 1;
    return out;
}

int unpackRect(char *line, int *x, int *y, int *cx, int *cy){
    *x = 0;
    *y = 0;
    *cx = 0;
    *cy = 0;
    int pos = 1;
    while (line[pos-1] != '=') pos++;
    while (line[pos] != '\0' && (*x == 0 || *y == 0 || *cx == 0 || *cy == 0)){
        char *num = GetNext(line, &pos, ',');
        int intNum = toInt(num);
        free(num);
        if (*x == 0){
            *x = intNum;
            continue;
        }
        if (*y == 0){
            *y = intNum;
            continue;
        }
        if (*cx == 0){
            *cx = intNum;
            continue;
        }
        if (*cy == 0){
            *cy = intNum;
            continue;
        }
    }
    return pos;
}

COLORREF readColor(char *color){
    int pos = 0;

    // RED
    size_t rSize = 0;
    while (color[pos+rSize] != ',')rSize++;
    char *red_val = (char *)malloc(rSize+1);
    if (red_val == NULL) return RGB(255,125,125);
    for (int i = 0; i < rSize; i++) red_val[i] = color[pos+i];
    pos+=rSize+1;
    red_val[rSize] = '\0';
    int r = toInt(red_val);

    // GREEN
    size_t gSize = 0;
    while (color[pos+gSize] != ',')gSize++;
    char *green_val = (char *)malloc(gSize+1);
    if (green_val == NULL) return RGB(255,125,125);
    for (int i = 0; i < gSize; i++) green_val[i] = color[pos+i];
    pos+=gSize+1;
    green_val[gSize] = '\0';
    int g = toInt(green_val);

    // BLUE
    size_t bSize = 0;
    while (color[pos+bSize] != ',')bSize++;
    char *blue_val = (char *)malloc(bSize+1);
    if (blue_val == NULL) return RGB(255,125,125);
    for (int i = 0; i < bSize; i++) blue_val[i] = color[pos+i];
    pos+=bSize+1;
    blue_val[bSize] = '\0';
    int b = toInt(blue_val);
    free(red_val);
    free(green_val);
    free(blue_val);
    return RGB(r, g, b);
}

void ProcessData(char *name, int argc, char **argv){
    if (strcmp(name, "COL") == 0){
        if (argc < 5){
            printf("Badly formatted collision box!\n");
            for (int i = 0; i < argc; i++) free(argv[i]); // Free all arguments
            return;
        }
        int x, y, cx, cy;
        x = toInt(argv[0]);
        y = toInt(argv[1]);
        cx = toInt(argv[2]);
        cy = toInt(argv[3]);
        if (argc == 5){
            COLORREF color = readColor(argv[4]);
            CreateSpriteInGroup(collisions, x, y, cx, cy, color);
        }
        if (argc == 6){
            char *file_name = argv[4];
            int scaling = toInt(argv[5]);
            Sprite *newSprite = (Sprite *)malloc(sizeof(Sprite));
            if (newSprite == NULL){
                printf("Error allocating memory!\n");
                return;
            }
            CreateAnimatedSprite(newSprite, x, y, cx, cy, file_name, "basic", 12, scaling);
            AddSpriteToGroup(collisions, newSprite);
        }
    }
    if (strcmp(name, "BGI") == 0){
        if (argc == 2){
            char *file_name = argv[0];
            int upscale = toInt(argv[1]);
            if (bg != NULL){
                EraseSprite(bg);
                bg = NULL;
            }
            bg = (Sprite *)malloc(sizeof(Sprite));
            CreateAnimatedSprite(bg, 0, 0, game_res[0], game_res[1], file_name, "basic", 12, upscale);
        }
    }
    if (strcmp(name, "REF") == 0){
        if (argc == 5){
            int x, y, cx, cy;
            x = toInt(argv[0]);
            y = toInt(argv[1]);
            cx = toInt(argv[2]);
            cy = toInt(argv[3]);
            RedirectItem *redirect_object = (RedirectItem *)malloc(sizeof(RedirectItem));
            redirect_object->next = NULL;
            redirect_object->redirect = (char *)malloc( strlen(argv[4]) + 1 );
            for (int i = 0; i < strlen(argv[4]); i++) (redirect_object->redirect)[i] = argv[4][i];
            (redirect_object->redirect)[strlen(argv[4])] = '\0';
            if (redirects == NULL){
                redirects = CreateSpriteGroup(x, y, cx, cy, RGB(125, 255, 255));
                redirect_links = redirect_object;
            }else{
                RedirectItem *curr = redirect_links;
                while (curr->next != NULL) curr = curr->next;
                curr->next = redirect_object;
                CreateSpriteInGroup(redirects, x, y, cx, cy, RGB(125, 255, 255));
            }
        }  
    }
    if (strcmp(name, "ENE") == 0){
        if (argc >= 6){
            int x, y, cx, cy;
            x = toInt(argv[0]);
            y = toInt(argv[1]);
            cx = toInt(argv[2]);
            cy = toInt(argv[3]);
            char *path = argv[4];
            int upscale = toInt(argv[5]);
            loadEnemyNPC(path, x, y, cx, cy, upscale);
        }
    }
    if (strcmp(name, "FRN") == 0){
        if (argc >= 6){
            int x, y, cx, cy;
            x = toInt(argv[0]);
            y = toInt(argv[1]);
            cx = toInt(argv[2]);
            cy = toInt(argv[3]);
            char *path = argv[4];
            int upscale = toInt(argv[5]);
            loadFriendlyNPC(path, x, y, cx, cy, upscale);
        }
    }
    if (strcmp(name, "CNV") == 0){
        if (argc >= 1){
            char *file_name = argv[0];
            printf("Dialogue str: %s\n", file_name);
        }
    }
    for (int i = 0; i < argc; i++) free(argv[i]); // Free all arguments
    free(argv);
}

// Load lvl file
void loadLevel(char *level_name){
    collisions = CreateMapBoundaries(0, game_res[0], game_res[1]);
    if (validExtension(level_name)){
        FILE *level = fopen(level_name, "rb");
        fclose(level);
    }else{
        FILE *level_raw = fopen(level_name, "r");
        if (level_raw == NULL){
            printf("Error opening [%s]!\n", level_name);
            return;
        }
        char next_line[512];
        while (fgets(next_line, 512, level_raw) > 0){
            if (next_line[0] == '!')continue;
            char **argv = NULL;
            int argc = 0;
            char *name = (char *)malloc(4);
            int pos = 0;
            for (int i = 0; i < 3; i++) name[pos++] = next_line[i];
            name[pos++] = '\0';
            char stopper = ',';
            while (next_line[pos] != '\0' && next_line[pos] != '\n'){
                size_t currElem = 0;
                if (next_line[pos] == '#' || next_line[pos] == '"' || next_line[pos] == ',') {
                    stopper = next_line[pos];
                    pos++;
                }
                while (next_line[pos + currElem] != stopper && next_line[pos + currElem] != '\0' && next_line[pos+currElem] != '\n')currElem++;
                char *argument = (char *)malloc(currElem + 1);
                if (argument == NULL){
                    printf("Error allocating data!\n");
                    return;
                }
                for (int i = 0; i < currElem; i++) argument[i] = next_line[pos+i];
                argument[currElem] = '\0';
                if (argv == NULL){
                    argv = (char **)malloc(sizeof(char *)*(argc+1));
                }else{
                    char **tmp = (char **)realloc(argv, sizeof(char *)*(argc+1));
                    if (tmp == NULL){
                        printf("Error reallocating memory!\n");
                        return;
                    }
                    argv = tmp;
                }
                argv[argc++] = argument;
                pos+=currElem + 1;
                if (next_line[pos] == stopper) pos++;
            }
            ProcessData(name, argc, argv);
        }
        fclose(level_raw);
    }
}

void StartGraphics(HWND hWnd){
    HDC hdc = GetDC(hWnd);
    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, game_res[0], game_res[1]);
    hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
    ReleaseDC(hWnd, hdc);
}

int closest_width, closest_height;
int offsetX, offsetY;
int resized_ticks = 0;

int game_paused = FALSE;

void Draw(HWND hWnd, int screen_width, int screen_height, int paused){
    // printf("Drawing!\n");
    PAINTSTRUCT ps;
    HDC hdcW = BeginPaint(hWnd, &ps);
    RECT rcPaint;
    rcPaint.top = 0;
    rcPaint.left = 0;
    rcPaint.bottom = game_res[1];
    rcPaint.right = game_res[0];
    FillRect(hdcMem, &rcPaint, (HBRUSH)(BLACK_BRUSH));
    if (!paused){
        if (bg != NULL) PaintSprite(hdcMem, bg);
        PaintSpriteGroup(hdcMem, collisions);
        // Shows redirect collision boxes
        // PaintSpriteGroup(hdcMem, redirects);
        drawAllNPCs(hdcMem);
        PaintSprite(hdcMem, GetPlayerPtr());
        game_paused = FALSE;
    }else{
        if (!game_paused){
            HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdcW, &ps.rcPaint, black_brush);
            DeleteObject(black_brush);
            game_paused = TRUE;
        }
        char *title = "Sprayz";
        RECT title_rect;
        title_rect.left = 100;
        title_rect.top = 20;
        DrawText(hdcMem, title, -1, &title_rect, DT_TOP);
    }
    
    if (resized_ticks > 0.01){
        resized_ticks++;
        if (resized_ticks > 30){
            HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdcW, &ps.rcPaint, black_brush);
            DeleteObject(black_brush);
            resized_ticks = 0;
        }
    }
    StretchBlt(hdcW, offsetX, offsetY, closest_width, closest_height, hdcMem, 0, 0, game_res[0], game_res[1], SRCCOPY);
    EndPaint(hWnd, &ps);
}

// Action on window resize
void Resize(HWND hWnd, int screen_width, int screen_height){
    float aspect_ratio = (float)game_res[0]/game_res[1];
    closest_width = screen_width;
    closest_height = (int)((float)screen_width/aspect_ratio);
    if (closest_height > screen_height){
        closest_width = (int)((float)screen_height*aspect_ratio);
        closest_height = screen_height;
    }
    offsetX = (int)((float)(screen_width-closest_width)/2.0);
    offsetY = (int)((float)(screen_height-closest_height)/2.0);
    resized_ticks = 1;
}

void EndLastLevel(){
    clearSprites();
    if (bg) EraseSprite(bg);
    if (collisions) DeleteSpriteGroup(collisions);
    if (redirects) DeleteSpriteGroup(redirects);
    collisions = NULL;
    redirects = NULL;
    bg = NULL;

    // Redirect cleanup
    RedirectItem *curr = redirect_links;
    while (curr != NULL){
        free(curr->redirect);
        RedirectItem *next = curr->next;
        free(curr);
        curr = next;
    }
}

void Update(float dt){
    UpdatePosition(dt, collisions);
    UpdateAnimatedSprites(collisions, dt);
    SpriteGroup *curr = redirects;
    RedirectItem *curr_redirect = redirect_links;
    int redirected = FALSE;
    updateNPCs(collisions);
    while (curr != NULL && !redirected){
        int *CollisionsReg = (int *)malloc(sizeof(int) * 8);
        if (GetCollision(curr->sprite, CollisionsReg, GetPlayerPtr()) > 0){
            char *redirect_copy = (char *)malloc(strlen(curr_redirect->redirect)+1);
            if (redirect_copy == NULL){
                printf("Error allocating space for data!\n");
                return;
            }
            for (int i = 0; i < strlen(curr_redirect->redirect)+1; i++)redirect_copy[i] = curr_redirect->redirect[i];
            EndLastLevel();
            // Move character to position before loading level
            POINT player_pos = GetPlayerPos();

            if (CollisionsReg[0] && CollisionsReg[1]){
                player_pos.y = game_res[1]-200;
            }
            if (CollisionsReg[2] && CollisionsReg[3]){
                player_pos.y = 0;
            }
            if (CollisionsReg[0] && CollisionsReg[2]){
                player_pos.x = game_res[2]-GetPlayerSize().cx;
            }
            if (CollisionsReg[1] && CollisionsReg[3]){
                player_pos.x = 0;
            }
            SetPlayerPos(player_pos);

            loadLevel(redirect_copy);
            free(redirect_copy);
            free(CollisionsReg);
            redirected = TRUE;
            curr = NULL;
        }
        if (!redirected){
            curr_redirect = curr_redirect->next;
            curr = curr->next;
        }
    }
}

void onEnd(){
    EndLastLevel();
    EndPlayer();
    deleteBrushes();
    if (hdcMem && hbmOld) SelectObject(hdcMem, hbmOld);
    if (hdcMem) DeleteDC(hdcMem);
    if (hbmMem) DeleteObject(hbmMem);
}