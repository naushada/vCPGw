#/*!Name of the binary*/

SUBDIRS=Arp Dns

noinst_LIBRARIES = libDhcps.a

libDhcps_a_SOURCES = Src/DhcpServer.cc \
										 Src/DhcpServerUser.cc \
										 Src/DhcpServerState.cc \
										 Src/DhcpServerStateDecline.cc \
										 Src/DhcpServerStateDiscover.cc \
										 Src/DhcpServerStateInit.cc \
										 Src/DhcpServerStateIPAllocated.cc \
										 Src/DhcpServerStateLeaseExpire.cc \
										 Src/DhcpServerStateOffer.cc \
										 Src/DhcpServerStateRelease.cc \
										 Src/DhcpServerStateRequestAck.cc \
										 Src/DhcpServerStateRequest.cc



libDhcps_a_CPPFLAGS=-I$(srcdir)/Inc \
										-I$(srcdir)/../Ipc/Inc \
                    -I$(srcdir)/../Common/Inc \
                    -I$(srcdir)/../Inc \
                    -I/usr/local \
                    -I/usr/local/ACE-6.5.0/include

libDhcps_a_CXXFLAGS=-g -Wall -std=gnu++11

libDhcps_a_AR=ar rvs


