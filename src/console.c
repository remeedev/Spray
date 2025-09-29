#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/drawing.h"
#include "headers/generalvars.h"
#include "headers/console_cmd.h"
#include "headers/level_loader.h"
#include "headers/savefile.h"

POINT console_pos = {0, 0};

char *console_out = NULL;
char *console_in = NULL;

int console_on = FALSE;

int blip_pos = 0;

int blip_counter = 0;
int max_blip = 50;
int blip_on = FALSE;

int recommendation = -1;

void InitConsoleVars(){
    if (console_in == NULL){
        console_in = (char *)malloc(1);
        if (!console_in) return;
        console_in[0] = '\0';
    }
    if (console_out == NULL) {
        console_out = (char *)malloc(1);
        if (!console_out) return;
        console_out[0] = '\0';
    }
}

void PrintToConsole(char *text){
    if (console_out == NULL) InitConsoleVars();
    int startT;
    if (console_out[0] == '\0'){
        startT = 0;
    }else{
        startT = strlen(console_out);
    }
    char *tmp_new = (char *)realloc(console_out, startT + strlen(text) + 1);
    if (tmp_new == NULL){
        printf("Error printing to game console!\n");
    }
    console_out = tmp_new;
    for (int i = 0; text[i] != '\0'; i++){
        console_out[startT + i] = text[i];
    }
    console_out[startT + strlen(text)] = '\0';
}

int console_allow = TRUE;

void save_console_allow(){
    save_int("console_allow", &console_allow);
}

void InitConsole(){
    InitConsoleVars();
    PrintToConsole("Welcome gamer!\nUse any command you want!\nAnd if you need help with a command just run the command with help as the first argument!\n");
    PrintToConsole("Run the 'list_commands' command to check out available commands!\n");
}

void ClearConsole(){
    free(console_out);
    console_out = NULL;
    InitConsoleVars();
}

void DrawConsoleIfNeeded(HDC hdcMem){
    if (!console_on) return;

    // Get font information
    TEXTMETRIC tm;
    SelectObject(hdcMem, ConsoleFont);

    GetTextMetrics(hdcMem, &tm);
    int fontWidth = tm.tmAveCharWidth;
    int fontHeight = tm.tmHeight;
    int spacing = 4;

    RECT background_rect = {
        console_pos.x, // Left
        console_pos.y, // Top
        console_pos.x + game_res[0], // Right
        console_pos.y + 10 * (fontHeight + spacing) // Bottom
    };
    FillRect(hdcMem, &background_rect, CreateNewColorBrush(RGB(0, 0, 0))->brush);

    RECT input_rect = {
        background_rect.left,
        background_rect.bottom,
        background_rect.right,
        background_rect.bottom + fontHeight + spacing
    };
    FillRect(hdcMem, &input_rect, CreateNewColorBrush(RGB(50, 50, 50))->brush);

    // Start writing the words
    SetTextColor(hdcMem, RGB(255, 255, 255));
    SetTextAlign(hdcMem, TA_BASELINE | TA_LEFT);
    SetBkMode(hdcMem, TRANSPARENT);
    int yAlign = (input_rect.top + input_rect.bottom - fontHeight);
    yAlign /= 2;
    yAlign += tm.tmAscent;
    TextOut(hdcMem, spacing/2, yAlign, console_in, strlen(console_in));

    // DRAW TEXT CURSOR
    blip_counter++;
    if (blip_counter >= max_blip) {
        blip_counter = 0;
        blip_on = !blip_on;
    }
    if (blip_on){
        int caretWidth = 2;
        RECT blipRect;
        blipRect.top = input_rect.top + spacing/2;
        blipRect.bottom = blipRect.top + fontHeight;
        blipRect.left = spacing/2 + blip_pos*(fontWidth);
        blipRect.right = blipRect.left + caretWidth;
        FillRect(hdcMem, &blipRect, CreateNewColorBrush(RGB(255, 255, 255))->brush);
    }

    // DRAW RECOMMENDATION
    if (recommendation != -1){
        RECT recRect;
        recRect.left = input_rect.left;
        recRect.top = input_rect.bottom;
        recRect.bottom = recRect.top + fontHeight + spacing;
        recRect.right = recRect.left + fontWidth*(strlen(cmds[recommendation]) + 2);

        FillRect(hdcMem, &recRect, CreateNewColorBrush(RGB(50, 50, 50))->brush);
        int x = (recRect.right + recRect.left) / 2;
        int y = (recRect.bottom + recRect.top) / 2;
        SetTextAlign(hdcMem, TA_BASELINE | TA_CENTER);
        y+= -(fontHeight / 2) + tm.tmAscent;
        TextOut(hdcMem, x, y, cmds[recommendation], strlen(cmds[recommendation]));
    }
    SetTextAlign(hdcMem, TA_BASELINE | TA_LEFT);

    // DRAW CONSOLE OUTPUT
    int currPos = strlen(console_out) - 1;
    int linesDrawn = 0;
    
    // TODO: Repare console for clean printing!
    // Mightve been fixed, needs testing
    SetTextAlign(hdcMem, TA_BASELINE | TA_LEFT);
    int ignoreFirst = FALSE;
    int curr_width = background_rect.right - background_rect.left;
    int max_size = (int)((float)curr_width/(float)fontWidth);
    while (currPos > 0 && linesDrawn < 10){
        // Get last line
        size_t size = 0;
        while (console_out[currPos - size] != '\n' && currPos - size > 0) size++;
        while (size > max_size){
            size = size - max_size ;
        }
        if (currPos - size == 0) size++;
        if (size <= 0) {
            // In case read is invalid still assume line was drawn
            linesDrawn++;
            currPos--;
            continue;
        }
        char *console_text = (char *)malloc(size + 1);
        if (console_text == NULL){
            return;
        }
        for (int i = 0; i < size; i++){
            console_text[size - i - 1] = console_out[currPos - i];
        }
        console_text[size] = '\0';

        // Blatantly ignores the first line that was written, assumed to be a '\n' character
        if (!ignoreFirst){
            linesDrawn--;
            ignoreFirst = TRUE;
        }

        int yAlign = background_rect.bottom - (linesDrawn) * ( fontHeight + spacing );
        yAlign -= (spacing+fontHeight)/2;
        yAlign += -(fontHeight/2) + tm.tmAscent;
        TextOut(hdcMem, background_rect.left + spacing/2, yAlign, console_text, strlen(console_text));
        if (console_out[currPos - size] == '\n'){
            currPos = currPos - size - 1;
        }else{
            currPos = currPos - size;
        }
        free(console_text);
        linesDrawn++;
    }
}

char *cmd_mem = NULL;

void runCommand(){
    if (console_in[0] == '\0') return;
    if (cmd_mem) free(cmd_mem);
    cmd_mem = (char *)malloc(strlen(console_in) + 1);
    if (cmd_mem == NULL){
        printf("Error allocating command memory!\n");
        return;
    }
    strcpy(cmd_mem, console_in);
    // Processing
    size_t baseSize = 0;
    while (console_in[baseSize] != ' ' && console_in[baseSize] != '\0') baseSize++;
    char *base_cmd = (char *)malloc(baseSize + 1);
    for (size_t i = 0; i < baseSize; i++){
        base_cmd[i] = console_in[i];
    }
    base_cmd[baseSize] = '\0';

    int argc = 0;
    char **argv = NULL;
    int reading = FALSE;
    int pos = baseSize;
    while (console_in[pos] != '\0'){
        char stopper = ' ';
        if (console_in[pos] == stopper){
            pos++;
            continue;
        }
        if (console_in[pos] == '"') {
            stopper = '"';
            pos++;
        }
        size_t argSize = 0;
        while (console_in[pos + argSize] != stopper && console_in[pos + argSize] != '\0') argSize++;
        if (argSize == 0){
            pos++;
            continue;
        }
        char *arg = (char *)malloc(argSize + 1);
        for (int i = 0; i < argSize; i++){
            arg[i] = console_in[pos + i];
        }
        arg[argSize] = '\0';
        argc++;
        pos = pos + argSize;
        if (stopper == '"') pos++;
        if (argv == NULL){
            argv = (char **)malloc(sizeof(char *));
            if (argv == NULL){
                printf("Error allocating space for variables!\n");
                return;
            }
            argv[0] = arg;
        }else{
            char **tmp = (char **)realloc(argv, sizeof(char *)*argc);
            if (tmp == NULL){
                printf("Error allocating space for variables!\n");
                return;
            }
            argv = tmp;
            argv[argc - 1] = arg;
        }
    }

    // Actually run command
    int found = FALSE;
    int cmdPos = 0;
    while (cmds[cmdPos] != NULL && !found){
        if (strcmp(base_cmd, cmds[cmdPos]) == 0){
            found = TRUE;
            funcs[cmdPos](argc, argv);
        }
        cmdPos++;
    }
    if (!found){
        PrintToConsole("Command '");
        PrintToConsole(base_cmd);
        PrintToConsole("' was not found!\n");
    }

    // Clear arguments stored
    free(base_cmd);
    for (int i = 0; i < argc; i++){
        free(argv[i]);
    }
    free(argv);
    recommendation = -1;
    // Clear input after processing
    free(console_in);
    console_in = NULL;
    blip_pos = 0;
    InitConsoleVars();
}

// TODO: Make recommendations for the insides of each command
void genRecommendation(){
    if (console_in[0] == '\0'){
        recommendation = -1;
        return;
    }
    int found = FALSE;
    int cmdPos = 0;
    while (cmds[cmdPos] != NULL && !found){
        int currPos = 0;
        while(cmds[cmdPos][currPos] != '\0' && console_in[currPos] != '\0' && console_in[currPos] == cmds[cmdPos][currPos]) currPos++;
        if (console_in[currPos] == '\0' && cmds[cmdPos][currPos] != '\0'){
            found = TRUE;
            recommendation = cmdPos;
        }
        cmdPos++;
    }
    if (!found) recommendation = -1;
}

void removeChar(){
    if (console_in[0] == '\0')return;
    if (blip_pos == 0) return;
    size_t newSize = strlen(console_in);
    for (int i = blip_pos - 1; i < newSize - 1; i++){
        console_in[i] = console_in[i + 1];
    }
    console_in[newSize - 1] = '\0';
    char *tmp = (char *)realloc(console_in, newSize);
    if (tmp == NULL){
        printf("Error removing char from input!\n");
        return;
    }
    console_in = tmp;
    blip_pos--;
}

void addChar(char in){
    size_t prevSize = strlen(console_in);
    char *tmp = (char *)realloc(console_in, prevSize + 2);
    if (tmp == NULL){
        printf("Error adding char to input!\n");
        return;
    }
    console_in = tmp;
    char move = console_in[blip_pos];
    for (int i = prevSize; i > blip_pos; i--){
        console_in[i] = console_in[i - 1];
    }
    console_in[blip_pos] = in;
    console_in[prevSize + 1] = '\0';
    blip_pos++;
}

void handleCharConsole(UINT key){
    if (!console_on) return;
    if (key == '\n' || key == '\0' || key == '\r' || key == 8 || key == '\t' || key == 27) return;
    addChar((char) key);
    genRecommendation();
}

void handleKeyConsole(UINT key){
    if (console_allow == FALSE) return; // Have to add option to add the console to game save
    if (key == VK_TAB){
        if (!console_on){
            console_on = !console_on;
            return;
        }
        if (recommendation == -1){
            console_on = !console_on;
        }else{
            char *tmp = (char *)realloc(console_in, strlen(cmds[recommendation]) + 1);
            if (tmp == NULL){
                printf("Error reallocating space for variables!\n");
                return;
            }
            console_in = tmp;
            strcpy(console_in, cmds[recommendation]);
            blip_pos = strlen(console_in);
        }
    }
    if (key == VK_ESCAPE && console_on){
        console_on = FALSE;
    }
    if (!console_on) return;
    if (key == VK_RETURN){
        if (console_in[0] == '\0') {
            console_on = FALSE;
            return;
        }
        runCommand();
        return;
    }
    if (key == VK_BACK){
        removeChar();
        genRecommendation();
        return;
    }
    if (key == VK_LEFT){
        if (blip_pos > 0) blip_pos--;
        return;
    }
    if (key == VK_RIGHT){
        if (blip_pos < strlen(console_in)) blip_pos++;
        return;
    }
    if (key == VK_UP){
        if (console_in) free(console_in);
        console_in = (char *)malloc(strlen(cmd_mem) + 1);
        blip_pos = strlen(cmd_mem);
        if (console_in == NULL){
            printf("Error writing previous command!\n");
            return;
        }
        strcpy(console_in, cmd_mem);
    }
}

void DeleteConsoleIfNeeded(){
    if (console_in) free(console_in);
    if (console_out) free(console_out);
}
