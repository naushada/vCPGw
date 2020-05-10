#ifndef __PROCMGR_CHILDHANDLER_CC__
#define __PROCMGR_CHILDHANDLER_CC__

#include "ProcMgr.h"
#include "CommonIF.h"


ChildHandler::ChildHandler(ACE_Thread_Manager *thrMgr, ProcMgr *parent) :
  ACE_Task<ACE_MT_SYNCH>(thrMgr)
{
  procMgr(parent);
  /*Create a thread in SUSPENDED Mode.*/
  procMgr().open();

  /*Now Make the thread running.*/
  procMgr().resume();

  pid_t cPid;
  pid_t pPid;

  /*spawn SYSMGR now.*/
  ACE_CString key("SYSMGR");
  ACE_CString ip("127.0.0.1");
  ACE_CString nodeTag("primary");

  switch((cPid = ACE_OS::fork()))
  {
  case 0:
    /*child process.*/
    ACE_OS::execlp((const char *)key.c_str(), key.c_str(), (const char *)ip.c_str(),
                   CommonIF::ENT_SYSMGR, CommonIF::INST1, (const char *)nodeTag.c_str());
    ACE_OS::exit(1);

  case -1:
    /*Error case.*/
    break;

  default:
    /*parent Process.*/
    pPid = ACE_OS::getppid();
    //buildSpawnRsp(in, cPid, pPid, mb);
    break;
  }
}

ChildHandler::~ChildHandler()
{
}

int ChildHandler::open(void *args)
{
  activate(THR_NEW_LWP | THR_SUSPENDED);
  return(0);
}

int ChildHandler::svc(void)
{
  pid_t childPid;
  int childStatus;

  for(;;)
  {
    childStatus = 0;

    childPid = ACE_OS::waitpid(-1, &childStatus, WNOHANG);

    if(childPid <= 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l childPid is less than zero\n")));
      break;
    }

    if(WIFEXITED(childStatus))
    {
      /*Child is terminated.*/
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD exited with childPid %u\n"), childPid));
      /*build and send notify to SYSMGR*/
    }
    else if(WIFSIGNALED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Terminated childPid %u\n"), childPid));

    }
    else if(WIFSTOPPED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Stopped childPid %u\n"), childPid));
    }
    else if(WIFCONTINUED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Continued childPid %u\n"), childPid));
    }
    else
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Unknown childPid %u\n"), childPid));
    }
  }

  return(0);
}

ProcMgr &ChildHandler::procMgr(void)
{
  return(*m_procMgr);
}

void ChildHandler::procMgr(ProcMgr *parent)
{
  m_procMgr = parent;
}











#endif /*__PROCMGR_CHILDHANDLER_CC__*/
