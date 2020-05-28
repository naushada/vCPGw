
bin_PROGRAMS=PROCMGR

PROCMGR_SOURCES=Src/ProcMgr.cc \
								Src/ProcMgrMain.cc

PROCMGR_CPPFLAGS=-I$(srcdir)/Inc \
								 -I$(srcdir)/../Ipc/Inc \
								 -I$(srcdir)/../Common/Inc \
								 -I/usr/local \
								 -I/usr/local/ACE-6.5.0/include


PROCMGR_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
							  -L$(CURDIR)/../Ipc \
							  -L$(CURDIR)/../Common

PROCMGR_CXXFLAGS=-g -Wall -std=gnu++11

PROCMGR_LDADD=-lACE -lreadline -lpthread -ltinfo -lIpc -lCommon
