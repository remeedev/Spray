#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "headers/generalvars.h"
#include "headers/console.h"
#include "headers/movement.h"
#include "headers/level_loader.h"
#include "headers/npc.h"
#include "headers/bicycle.h"
#include "headers/ui.h"
#include "headers/savefile.h"
#include "headers/throwables.h"
#include "headers/the_chronic.h"

typedef void (*cmdFunc)(int, char **);

char *cmds[] = {"log", "get", "set", "clear", "game_ver", "quit_game", "list_commands", "toggle_debug", "setHarm", "setTime", "saveFile", "reset_values", NULL};

typedef struct rec_node{
    char *recommendation;
    struct rec_node **rec_list;
} rec_node;

char *get_recommendation(char *curr_search, int path_length, char **paths){
    // Missing proper implementation
    // Arguments expected: <user_in>, <len(paths_before_user_in)> <paths_before_user_in>
    // eg. : "he", 1,"list_commands" -> list_commands he
    // Should return "help"

}

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

double getDoubleFromString(char *text){
    double val = 0;
    int dotWeight = 1;
    int pos = strlen(text) - 1;
    int power = 1;
    int negative = FALSE;
    while (pos >= 0){
        if (text[pos] == '-'){
            negative = TRUE;
            pos--;
            continue;
        }
        if (text[pos] == '.' || text[pos] == ','){
            dotWeight = power;
            pos--;
            continue;
        }
        if (text[pos] < '0' || text[pos] > '9'){
            pos--;
            continue;
        }
        val += power*(int)(text[pos] - '0');
        power*=10;
        pos--;
    }
    val/=dotWeight;

    if (negative) val = -val;
    return val;

}

float getFloatFromString(char * text){
    float val = (float)getDoubleFromString(text);
    return val;
}

char *variables[] = {"f_gravity", "b_see_collisions", "r_collisionColor", "r_spriteColor", "b_showDebug", "d_gametime", "b_artistOptions", NULL};
void *variableVal[] = {&gravity, &showCollisions, &collisionColor, &characterColor, &showDebug, &game_time, &artistOptions};

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
        PrintToConsole("TYPE   NAME  VALUE \n");
        while (variables[pos] != NULL){
            char val[100];
            
            if (variables[pos][0] == 'i'){
                sprintf(val, "%d", *((int *)variableVal[pos]));
                PrintToConsole("INT   ");
            }
            if (variables[pos][0] == 'b'){
                sprintf(val, "%s", *((int *)variableVal[pos]) ? "TRUE" : "FALSE");
                PrintToConsole("BOOL   ");
            }
            if (variables[pos][0] == 'f'){
                sprintf(val, "%f", *((float *)variableVal[pos]));
                PrintToConsole("FLOAT  ");
            }
            if (variables[pos][0] == 'r'){
                COLORREF color_val = *((COLORREF *)variableVal[pos]);
                sprintf(val, "%d %d %d", GetRValue(color_val), GetGValue(color_val), GetBValue(color_val));
                PrintToConsole("COLOR  ");
            }
            if (variables[pos][0] == 'd'){
                sprintf(val, "%.2f", (double)(*(double *)variableVal[pos]));
                PrintToConsole("DOUBLE ");
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
            if (argv[0][0] == 'd'){
                if (argc < 2) return;
                double val = getFloatFromString(argv[1]);
                *((double *)variableVal[pos]) = val;
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
        for (int i = 0; i < argc; i++){
            printf("%d : %s [%d]\n", i, argv[i], argv[i][0]);
            PrintToConsole(argv[i]);
            PrintToConsole(", ");
        }
        PrintToConsole("\n");
        return;
    }
    toggleDebug = !toggleDebug;
    char *val = toggleDebug ? "TRUE" : "FALSE";
    PrintToConsole("Setting all values to: ");
    PrintToConsole(val);
    PrintToConsole("\n");
    char *falseArgv1[] = {"b_see_collisions", val};
    char *falseArgv2[] = {"b_showDebug", val};
    char *falseArgv3[] = {"b_artistOptions", val};
    setVar(2, falseArgv1);
    setVar(2, falseArgv2);
    setVar(2, falseArgv3);
}

void changeSpriteStatus(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function makes a sprite, with it's id, a friendly or an enemy\n");
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

void setTime(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function sets the current time\n");
            PrintToConsole("Usage:\n'setTime <time | midnight | morning>'\n");
            return;
        }
    }
    if (argc < 1){
        PrintToConsole("Incorrect usage of function\nRun 'setTime help' to learn how to use it\n");
        return;
    }
    int time_set = FALSE;
    if (strcmp(argv[0], "midnight") == 0){
        game_time = max_time / 2 - 50;
        time_set = TRUE;
    }
    if (strcmp(argv[0], "morning") == 0 && !time_set){
        game_time = 0;
        time_set = TRUE;
    }
    if (!time_set){
        game_time = getDoubleFromString(argv[0]);
    }
    char time_txt[10];
    sprintf(time_txt, "%.2f", game_time);
    PrintToConsole("Succesfully set game time to: ");
    PrintToConsole(time_txt);
    PrintToConsole("\n");
}

void reset_health(int argc, char **argv){
    if (argc == 1){
        if (strcmp(argv[0], "help") == 0){
            PrintToConsole("This function sets the values to the max\n");
            PrintToConsole("Usage:\n'reset_values'\n");
            return;
        }
    }
    GetPlayerPtr()->health = GetPlayerPtr()->maxHealth;
    grenade_count = 3;
    weed_bags = 5;
    
    PrintToConsole("Succesfully reset values!\n");
}

// Corresponding funcs
cmdFunc funcs[] = {&logToConsole, &getVar, &setVar, &clearConsole, &printGameVersion, &quitGame, &listCommands, &fullDebug, &changeSpriteStatus, &setTime, &write_save, &reset_health};
