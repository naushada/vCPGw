noinst_LIBRARIES = libDns.a

libDns_a_SOURCES = Src/Dns.cc


libDns_a_CPPFLAGS=-I$(srcdir)/Inc \
									-I$(srcdir)/../../Ipc/Inc \
                  -I$(srcdir)/../../Common/Inc \
                  -I$(srcdir)/../../Protocol/Inc \
                  -I$(srcdir)/../../../CPGw/Inc \
                  -I/usr/local \
                  -I/usr/local/ACE-6.5.0/include

libDns_a_CXXFLAGS=-g -Wall -std=gnu++11

libDns_a_AR=ar rvs
