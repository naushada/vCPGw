#ifndef __CLI_H__
#define __CLI_H__

#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

class TclientTask;



class Cli
{
public:
    struct Command
    {
        const char *cmd;
        const char *argv[16];
        const char *usage;
    };

    static Command m_command[256];
    static int m_offset;
    static int m_len;
    static char *m_cmdName;
    static int m_argOffset;

public:
    Cli();
    virtual ~Cli();
    friend char **commandCompletion(const char *text, int start, int end);
    friend char *commandGenerator(const char *text, int state);
    friend char *commandArgGenerator(const char *text, int state);
    friend char *commandArgListGenerator(const char *text, int state);

    void prompt(char *prompt);
    char *prompt(void);
    int executeLine(char *line);
    int processCommand(char *cmd, int len);
    int processResponse(char *rsp, int len);
    char *stripwhite(char *string);
    bool isValid(char *cmd);
    void help(char *cmd);
    void quit(void);
    bool continueStatus(void);
    void continueStatus(bool status);
    static void cmdName(char *cmdName);
    static char *cmdName(void);

    TclientTask &tclientTask(void);
    void tclientTask(TclientTask *tt);
    int init(void);
    int main(void);

private:
    char *m_prompt;
    bool m_continueStatus;
    TclientTask *m_tclientTask;
};

#endif /*__CLI_H__*/
