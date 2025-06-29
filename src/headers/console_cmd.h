#ifndef console_cmd
#define console_cmd

typedef void (*cmdFunc)(int, char **);

void logToConsole(int argc, char **argv);
void clearConsole(int argc, char **argv);
void printGameVersion(int argc, char **argv);
void quitGame(int argc, char **argv);
void setGravity(int argc, char **argv);

extern char *cmds[];
extern cmdFunc funcs[];

#endif