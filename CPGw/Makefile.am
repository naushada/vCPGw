#/*!Name of the binary*/

SUBDIRS=Common Ipc Protocol

bin_PROGRAMS=CPGateway

CPGateway_SOURCES=Src/CPGateway.cc \
	 					      Src/CPGatewayStateActivated.cc \
						      Src/CPGatewayState.cc \
						      Src/CPGatewayStateLocked.cc \
						      Src/CPGatewayStateUnLocked.cc

CPGateway_CPPFLAGS=-I$(srcdir)/Inc \
                   -I$(srcdir)/Common/Inc \
                   -I$(srcdir)/Ipc/Inc \
                   -I$(srcdir)/Protocol/Inc \
                   -I/usr/local \
                   -I/usr/local/ACE-6.5.0/include

CPGateway_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
                  -L$(CURDIR)/../Common \
                  -L$(CURDIR)/../Ipc \
                  -L$(CURDIR)/../Protocol

CPGateway_CXXFLAGS=-g -Wall -std=gnu++11

CPGateway_LDADD=-lACE -lreadline -lpthread -ltinfo -lIpc -lCommon -lDhcps


