#/*!Name of the binary*/

noinst_LIBRARIES = libdhcps.a

libdhcps_a_SOURCES = src/DhcpServer.cc \
										 src/DhcpServerUser.cc \
										 src/DhcpServerState.cc \
										 src/DhcpServerStateDecline.cc \
										 src/DhcpServerStateDiscover.cc \
										 src/DhcpServerStateInit.cc \
										 src/DhcpServerStateIPAllocated.cc \
										 src/DhcpServerStateLeaseExpire.cc \
										 src/DhcpServerStateOffer.cc \
										 src/DhcpServerStateRelease.cc \
										 src/DhcpServerStateRequestAck.cc \
										 src/DhcpServerStateRequest.cc



libdhcps_a_CPPFLAGS=-I$(srcdir)/inc \
										-I$(top_srcdir)/ipc/inc \
                    -I$(top_srcdir)/common/inc \
                    -I$(top_srcdir)/cpgateway/inc \
                    -I/usr/local \
                    -I/usr/local/ACE-6.5.0/include

libdhcps_a_CXXFLAGS=-g -Wall -std=gnu++11

libdhcps_a_AR=ar rvs
#libipc_a_LDFLAGS=-L/usr/localACE-6.5.0/lib


