#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "headers/generalvars.h"
#include "headers/console.h"
#include "headers/movement.h"
#include "headers/level_loader.h"
#include "headers/npc.h"

typedef void (*cmdFunc)(int, char **);

char *cmds[] = {"log", "get", "set", "clear", "game_ver", "quit_game", "list_commands", "toggle_debug", "setHarm", NULL};

void listCommands(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function lists all available commands\nUsage:\nlc\n");
            return;
        }
    }
    PrintToConsole("Here are all available commands: ");
    int cmdCount = 0;
    while (cmds[cmdCount] != NULL){
        if (cmdCount != 0){
            PrintToConsole(", ");
        }
        PrintToConsole(cmds[cmdCount]);
        cmdCount++;
    }
    PrintToConsole("\nPrinted a total of ");
    char cmdCountChar[100];
    PrintToConsole(itoa(cmdCount, cmdCountChar, 100));
    PrintToConsole(" available commands.\n");
}

void logToConsole(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function prints to console\nUsage:\nlog <arg1> <arg2>...");
            return;
        }
    }
    for (int i = 0; i < argc; i++){
        PrintToConsole(argv[i]);
        PrintToConsole("\n");
    }
}

void clearConsole(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function clears the console\nUsage:\nclear\n");
            return;
        }
    }
    ClearConsole();
}

void printGameVersion(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function prints the current game version\nUsage:\ngame_ver\n");
            return;
        }
    }
    PrintToConsole("The game is currently in the following version: ");
    PrintToConsole(gameVersion);
    PrintToConsole("\n");
}

void quitGame(int argc, char **argv){
    PostMessage(mainWindow, WM_QUIT, 0, 0);
}

float getFloatFromString(char * text){
    float val = 0;
    int dotWeight = 1;
    int pos = strlen(text) - 1;
    int power = 1;
    while (pos >= 0){
        if (text[pos] == '.' || text[pos] == ','){
            dotWeight = power;
            pos--;
            continue;
        }
        val += power*(int)(text[pos] - '0');
        power*=10;
        pos--;
    }
    val/=dotWeight;

    return val;
}

char *variables[] = {"f_gravity", "b_see_collisions", "r_collisionColor", "r_spriteColor", "b_showDebug", NULL};
void *variableVal[] = {&gravity, &showCollisions, &collisionColor, &characterColor, &showDebug};

void getVar(int argc, char **argv){
    if (argc < 1){
        PrintToConsole("Wrong usage of the command.\nRun 'get help' to learn usage.\n");
        return;
    }
    if (strcmp(argv[0], "help") == 0){
        PrintToConsole("This function gets the value of a variable within the game system.\nUsage: 'get <var_name | -a>'\nRun 'get -a' to see variables available\n");
        return;
    }
    if (strcmp(argv[0], "-a") == 0){
        int pos = 0;
        PrintToConsole("The following variables were discovered: \n");
        PrintToConsole("TYPE  NAME  VALUE \n");
        while (variables[pos] != NULL){
            char val[100];
            
            if (variables[pos][0] == 'i'){
                sprintf(val, "%d", *((int *)variableVal[pos]));
                PrintToConsole("INT   ");
            }
            if (variables[pos][0] == 'b'){
                sprintf(val, "%s", *((int *)variableVal[pos]) ? "TRUE" : "FALSE");
                PrintToConsole("BOOL  ");
            }
            if (variables[pos][0] == 'f'){
                sprintf(val, "%f", *((float *)variableVal[pos]));
                PrintToConsole("FLOAT ");
            }
            if (variables[pos][0] == 'r'){
                COLORREF color_val = *((COLORREF *)variableVal[pos]);
                printf("0x%08X\n", color_val);
                sprintf(val, "%d %d %d", GetRValue(color_val), GetGValue(color_val), GetBValue(color_val));
                PrintToConsole("COLOR ");
            }
            PrintToConsole(variables[pos]);
            PrintToConsole(" ");
            PrintToConsole(val);
            PrintToConsole(" \n");
            pos++;
        }
    }
}

void setVar(int argc, char **argv){
    if (argc < 1){
        PrintToConsole("Wrong usage of the command.\nRun 'set help' to learn usage.\n");
        return;
    }
    if (strcmp(argv[0], "help") == 0){
        PrintToConsole("This function sets a variable within the game system.\nUsage: 'set <var_name> <value>'\nRun 'get -a' to see variables available\n");
        return;
    }
    int pos = 0;
    int found = FALSE;
    while (variables[pos] != NULL && !found){
        if (strcmp(variables[pos], argv[0]) == 0){
            if (argv[0][0] == 'b'){
                if (argc < 2) return;
                *((int *)variableVal[pos]) = strcmp(argv[1], "TRUE") == 0 ? 1 : 0;
            }
            if (argv[0][0] == 'i'){
                if (argc < 2) return;
                int val = (int)getFloatFromString(argv[1]);
                *((int *)variableVal[pos]) = val;
            }
            if (argv[0][0] == 'f'){
                if (argc < 2) return;
                float val = getFloatFromString(argv[1]);
                *((float *)variableVal[pos]) = val;
            }
            if (argv[0][0] == 'r'){
                if (argc < 4) return;
                printf("%d arguments passed to color!\n", argc);
                int r = (int)getFloatFromString(argv[1]);
                int g = (int)getFloatFromString(argv[2]);
                int b = (int)getFloatFromString(argv[3]);
                r = r > 255 ? 255 : r;
                g = g > 255 ? 255 : g;
                b = b > 255 ? 255 : b;
                printf("Trying to create from: (%d, %d, %d)\n", r, g, b);
                *((COLORREF *)(variableVal[pos])) = RGB(r, g, b);
                printf("Done!\n");
            }
            found = TRUE;
            PrintToConsole(variables[pos]);
            PrintToConsole("'s value has changed!\n");
        }
        pos++;
    }
    if (!found){
        PrintToConsole("No such variable name found: '");
        PrintToConsole(argv[0]);
        PrintToConsole("'! \n");
    }
}

int toggleDebug = FALSE;

void fullDebug(int argc, char **argv){
    if (argc >= 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function sets a variable within the game system.\nUsage: 'set <var_name> <value>'\nRun 'get -a' to see variables available\n");
            return;
        }
        return;
    }
    toggleDebug = !toggleDebug;
    char *val = toggleDebug ? "TRUE" : "FALSE";
    char *falseArgv1[] = {"b_see_collisions", val};
    char *falseArgv2[] = {"b_showDebug", val};
    setVar(2, falseArgv1);
    setVar(2, falseArgv2);
}

void changeSpriteStatus(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function makes a sprite, with it's id a friendly or an enemy\n");
            PrintToConsole("Usage:\n'setHarm <id> <TRUE | FALSE>'\n");
            return;
        }
    }
    if (argc < 2){
        PrintToConsole("Wrong usage!\nRun 'setHarm help' to understand usage.\n");
        return;
    }
    int id = getFloatFromString(argv[0]);
    if (strcmp(argv[1], "TRUE") != 0 && strcmp(argv[1], "FALSE") != 0) {
        PrintToConsole("Value must be TRUE or FALSE!\n");
        return;
    }
    int value = strcmp(argv[1], "TRUE") ? TRUE : FALSE;
    changeHarmLevel(id, value);
}

// Corresponding funcs
cmdFunc funcs[] = {&logToConsole, &getVar, &setVar, &clearConsole, &printGameVersion, &quitGame, &listCommands, &fullDebug, &changeSpriteStatus};