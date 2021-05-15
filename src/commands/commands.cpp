#include "commands.h"
#include "commands/run.h"

// commands structure
struct
{
    const char *cmd;
    void (*fn)(int argc, char *argv[]);
    const char *about;
} cmds[] = {
    {"run", run, "run: starts data collection"},
};

void exec(int argc, char *argv[])
{
    for (unsigned int i = 0; i < sizeof cmds / sizeof cmds[0]; i++)
        if (strcmp(cmds[i].cmd, argv[0]) == 0)
        {
            cmds[i].fn(argc, argv);
            return;
        }
    printf("%s: No such command.\n", argv[0]);
}

void parse_command(char *c)
{
    char *argv[20];
    int argc = 0;
    int skipspace = 1;
    for (; *c; c++)
    {
        if (skipspace)
        {
            if (*c != ' ' && *c != '\t')
            {
                argv[argc++] = c;
                skipspace = 0;
            }
        }
        else
        {
            if (*c == ' ' || *c == '\t')
            {
                *c = '\0';
                skipspace = 1;
            }
        }
    }
    if (argc > 0)
    {
        argv[argc] = "";
        exec(argc, argv);
    }
}

void command_shell(void)
{
    char line[100];
    int i = 0;
    int len = 0;
    char c = 0;

    for (;;)
    {
        //reinitialize on new command
        i = 0;
        len = 0;
        c = 0;
        printf("> ");

        while (c != '\n')
        {
            if (Serial.available())
            {
                c = Serial.read();
                len++;
                printf("%c", c); //echoback character
                line[i++] = c;
            }
        }
        line[len - 2] = '\0'; //replace \n with \0

        parse_command(line);
    }
}