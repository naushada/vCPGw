#ifndef __READLINE_IF_H__
#define __READLINE_IF_H__

/*
This file interfaces with readline library and provides the implementation
for command/response to/from hostapd.
Copyright (C) <2019>  <Mohammed Naushad Ahmed>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

#include "Vtysh.h"

class ReadlineIF
{
  public:
    struct command
    {
      const char *cmd;
      const char *argv[16];
      const char *usage;
    };

    static command m_command[256];

    static int m_offset;
    static int m_len;
    static char *m_cmdName;
    static int m_argOffset;

  public:
    ReadlineIF(VtyshCtrlIF *inst);
    virtual ~ReadlineIF();
    int init(void);

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

    int start(void);
    VtyshCtrlIF *vtyshCtrlIF(void);

  private:
    char        *m_prompt;
    bool        m_continueStatus;
    VtyshCtrlIF *m_vtyshCtrlIF;
};

#endif
