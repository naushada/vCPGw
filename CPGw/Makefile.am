#/*!Name of the binary*/

bin_PROGRAMS = CPGateway

CPGateway_SOURCES = src/CPGateway.cc \
										src/CPGatewayStateActivated.cc \
										src/CPGatewayState.cc \
										src/CPGatewayStateLocked.cc \
										src/CPGatewayStateUnLocked.cc

CPGateway_CPPFLAGS=-I$(srcdir)/inc \
                  -I$(top_srcdir)/common/inc \
                  -I$(top_srcdir)/vdhcp/inc \
                  -I$(top_srcdir)/ipc/inc \
                  -I/usr/local \
                  -I/usr/local/ACE-6.5.0/include

CPGateway_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
             -L$(CURDIR)/../common \
             -L$(CURDIR)/../ipc \
             -L$(CURDIR)/../vdhcp

CPGateway_CXXFLAGS=-g -Wall -std=gnu++11

CPGateway_LDADD=-lACE -lreadline -lpthread -ltinfo -lipc -lcommon -ldhcps


