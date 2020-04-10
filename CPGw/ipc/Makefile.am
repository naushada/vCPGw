#/*!Name of the binary*/

noinst_LIBRARIES = libipc.a

libipc_a_SOURCES = src/ipc.cc

libipc_a_CPPFLAGS=-I$(srcdir)/inc \
                  -I$(top_srcdir)/common/inc \
                  -I/usr/local \
                  -I/usr/local/ACE-6.5.0/include

libipc_a_CXXFLAGS=-g -Wall -std=gnu++11

libipc_a_AR=ar rvs
#libipc_a_LDFLAGS=-L/usr/localACE-6.5.0/lib


