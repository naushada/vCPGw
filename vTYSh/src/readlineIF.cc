#ifndef __VTYSH_CC__
#define __VTYSH_CC__


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

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <ace/Reactor.h>
#include <ace/Basic_Types.h>
#include <ace/Event_Handler.h>
#include <ace/Task.h>
#include <ace/INET_Addr.h>
#include <ace/UNIX_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/LSOCK_CODgram.h>
#include <ace/LSOCK_Dgram.h>
#include <ace/Task_T.h>
#include <ace/UNIX_Addr.h>

#include "readlineIF.h"

/*These are static data member*/
int ReadlineIF::m_offset = 0;
int ReadlineIF::m_len = 0;
int ReadlineIF::m_argOffset = 0;

/*This static data member stores the selected command*/
char *ReadlineIF::m_cmdName = NULL;

ReadlineIF::command ReadlineIF::m_command[256] =
{
  /* command name */           /* command argument(s) */                 /* command description */
  { "ping",                    {/*argument list*/ NULL},                 "ping to vApg" },
  /* command name */           /* command argument(s) */                 /* command description */
  { "set",  {"port-name=",
             "bind-address=",
             "net-mask=",
             "start-ip=",
             "end-ip=",
             "domain-name=",
             "primary-dns=",
             "secondary-dns=",
             "mtu=",
             NULL},
                                                        "<name>=<value>" },
  /*get with arguments*/
  { "get",                     {"list","port-name", "bind-address", NULL}, "<name> = get runtime info" },
  { NULL,                      {NULL},                  NULL }
};

char *ReadlineIF::cmdName(void)
{
  return(m_cmdName);
}

void ReadlineIF::cmdName(char *cmdName)
{
  if(cmdName)
  {
    m_cmdName = strdup(cmdName);
  }
}

/*
 * @brief
 * @param
 * @return
 */
void ReadlineIF::prompt(char *prompt)
{
  m_prompt = strdup(prompt);
}

/*
 * @brief
 * @param
 * @return
 */
char *ReadlineIF::prompt()
{
  if(NULL == m_prompt)
  {
    m_prompt = strdup("(Bismillah)% ");
  }
  return(m_prompt);
}

char *commandArgListGenerator(const char *text, int state)
{
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
    ReadlineIF::m_argOffset = 0;
  }

  int &idx = ReadlineIF::m_offset;
  int &inner = ReadlineIF::m_argOffset;

  if(NULL == ReadlineIF::m_command[idx].argv[inner])
    return (char *)NULL;

  return(strdup(ReadlineIF::m_command[idx].argv[inner++]));
}


char *commandArgGenerator(const char *text, int state)
{
  const char *name;
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
    ReadlineIF::m_argOffset = 0;
  }

  int &idx = ReadlineIF::m_offset;
  int &inner = ReadlineIF::m_argOffset;
  /* Return the next name which partially matches from the command list. */
  while(NULL != (name = ReadlineIF::m_command[idx].argv[inner]))
  {
    inner += 1;
    if(strncmp (name, text, ACE_OS::strlen(text)) == 0)
    {
      return(strdup(name));
    }
  }

  /*Reset to default either for next command or command argument(s).*/
  ReadlineIF::m_argOffset = 0;

  /* If no names matched, then return NULL. */
  return((char *)NULL);
}

/* Generator function for command completion.  STATE lets us know whether
 * to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list. 
 */
char *commandGenerator(const char *text, int state)
{
  const char *name;
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
    ReadlineIF::m_offset = 0;
    ReadlineIF::m_len = strlen(text);
  }

  int &idx = ReadlineIF::m_offset;
  /* Return the next name which partially matches from the command list. */
  while(NULL != (name = ReadlineIF::m_command[idx].cmd))
  {
    idx += 1;
    if(strncmp (name, text, ReadlineIF::m_len) == 0)
    {
      return(strdup(name));
    }
  }

  /*Reset to default either for next command or command argument(s).*/
  ReadlineIF::m_offset = 0;

  /* If no names matched, then return NULL. */
  return((char *)NULL);
}

/* Attempt to complete on the contents of TEXT.  START and END show the
 * region of TEXT that contains the word to complete.  We can use the
 * entire line in case we want to do some simple parsing.  Return the
 * array of matches, or NULL if there aren't any.
 */
char **commandCompletion(const char *text, int start, int end)
{
  char **matches;
  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if(start == 0)
  {
    rl_attempted_completion_over = 1;
    matches = rl_completion_matches(text, commandGenerator);
  }
  else
  {
    /*user has hit the space bar*/
    if(start == end)
    {
      int idx = 0;
      /*remember it into its context - this is the command whose argument(s) to be listed.*/
      ReadlineIF::cmdName(rl_line_buffer);

      /*Return the entire arguments - list must have last element as NULL.*/
      for(idx = 0; ReadlineIF::m_command[idx].cmd; idx++)
      {
        if(!strncmp(rl_line_buffer,
                    ReadlineIF::m_command[idx].cmd,
                    ACE_OS::strlen(ReadlineIF::m_command[idx].cmd)))
        {
          /*remember this offset and will be used while looping through command arguments.*/
          ReadlineIF::m_offset = idx;
          rl_attempted_completion_over = 1;
          matches = rl_completion_matches(text, commandArgListGenerator);
          /*break the while loop.*/
          break;
        }
      }
    }
    else
    {
      /*user has entered the initials of argument*/
      rl_attempted_completion_over = 1;
      matches = rl_completion_matches(text, commandArgGenerator);
    }
  }

  return(matches);
}

int ReadlineIF::init(void)
{

  rl_attempted_completion_over = 1;
  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = commandCompletion;
  return(0);
}

ReadlineIF::~ReadlineIF()
{
  delete m_prompt;
  /*set to default - NULL*/
  m_prompt = NULL;
  m_vtyshCtrlIF = NULL;
}

ReadlineIF::ReadlineIF(VtyshCtrlIF *inst)
{
  m_prompt         = NULL;
  m_continueStatus = false;
  m_vtyshCtrlIF = inst;
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */

bool ReadlineIF::isValid(char *cmd)
{
  int i;
  bool result = false;
  do
  {
    for(i = 0; ReadlineIF::m_command[i].cmd; i++)
    {
      if(cmd && !strcmp(cmd, ReadlineIF::m_command[i].cmd))
      {
        result = true;
        break;
      }
    }
  }while(0);

  return(result);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *ReadlineIF::stripwhite (char *string)
{
  char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

int ReadlineIF::executeLine(char *req)
{
  int i;
  char *cmd = NULL;
  char *cmdArg = NULL;
  bool isFound = false;
  char *line = strdup(req);
  int status = 1;

  /* Isolate the command word. */
  i = 0;
  while(line[i] && whitespace(line[i]))
    i++;
  cmd = line + i;

  while(line[i] && !whitespace(line[i]))
    i++;

  if(line[i])
    cmd[i++] = '\0';

  cmdArg = line + i;
  (void)cmdArg;

  isFound = isValid(cmd);

  do
  {
    if(!strncmp(cmd, "help", 4) || (!strncmp(cmd, "?", 1)))
    {
      help(cmd);
    }
    else if((!strncmp(cmd, "quit", 4)) || (!strncmp(cmd, "q", 1)))
    {
      ACE_DEBUG((LM_INFO, "The cmd is %s\n", cmd));
      quit();
    }
    else
    {
      /* Call the function. */
      //processCommand(origLine, strlen(origLine));
      status = 0;

      if(!isFound)
      {
        ACE_ERROR((LM_ERROR, "%s: No such command.\n", cmd));
        help(cmd);
        /*bypass the following statement.*/
        break;
      }
    }

  }while(0);

  free(line);
  return(status);
}

/* Print out help for cmd, or for all of the commands if cmd is
 * not present.
 */
void ReadlineIF::help(char *cmd)
{
  int i;
  int printed = 0;

  for (i = 0; ReadlineIF::m_command[i].cmd; i++)
  {
    if(!*cmd || (!strcmp(cmd, ReadlineIF::m_command[i].cmd)))
    {
      printf("%s\t\t%s.\n",
             ReadlineIF::m_command[i].cmd,
             ReadlineIF::m_command[i].usage);
      printed++;
    }
  }

  if(!printed)
  {
    printf ("No commands match `%s'.  Possibilties are:\n", cmd);

    for (i = 0; ReadlineIF::m_command[i].cmd; i++)
    {
      /* Print in six columns. */
      if (printed == 10)
      {
        printed = 0;
        printf ("\n");
      }

      printf ("%s\t", ReadlineIF::m_command[i].cmd);
      printed++;
    }

    if(printed)
      printf ("\n");
  }

}

void ReadlineIF::quit(void)
{
  continueStatus(true);
}

void ReadlineIF::continueStatus(bool status)
{
  m_continueStatus = status;
}

bool ReadlineIF::continueStatus(void)
{
  return(m_continueStatus);
}

int ReadlineIF::processCommand(char *cmd, int len)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The Command is %s\n"), cmd));
  return(0);
}

int ReadlineIF::processResponse(char *rsp, int len)
{
  return(0);
}

int ReadlineIF::start(void)
{
  char *line = NULL;
  char *s = NULL;

  /* Loop reading and executing lines until the user quits. */
  for( ; !continueStatus(); )
  {
    line = readline(prompt());

    if(!line)
      break;

    /* Remove leading and trailing whitespace from the line.
       Then, if there is anything left, add it to the history list
       and execute it. */
    s = stripwhite(line);

    if(*s)
    {
      add_history(s);
      if(!executeLine(s))
      {
        /*Send this command to vAPG via control interface.*/
        if(-1 == vtyshCtrlIF()->transmit(s))
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Send to vAPG Failed : %m\n")));
        }
      }
    }

    free (line);
  }

  size_t ss = vtyshCtrlIF()->thr_mgr()->count_threads();
  ACE_DEBUG((LM_INFO, "%D %M %N:%l the thread count is %d\n", ss));
  vtyshCtrlIF()->thr_mgr()->exit();
  exit(0);

}

VtyshCtrlIF *ReadlineIF::vtyshCtrlIF(void)
{
  return(m_vtyshCtrlIF);
}

#endif
