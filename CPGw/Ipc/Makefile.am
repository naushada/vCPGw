#/*!Name of the binary*/

noinst_LIBRARIES = libIpc.a

libIpc_a_SOURCES = Src/UniIPC.cc

libIpc_a_CPPFLAGS=-I$(srcdir)/Inc \
                  -I$(srcdir)/../Common/Inc \
                  -I/usr/local \
                  -I/usr/local/ACE-6.5.0/include

libIpc_a_CXXFLAGS=-g -Wall -std=gnu++11

libIpc_a_AR=ar rvs


