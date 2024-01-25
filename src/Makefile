
GNUCAP_CONF = gnucap-conf
INSTALL = install
MKDIR_P = mkdir -p

include Make1
#
CXX = $(shell $(GNUCAP_CONF) --cxx)
GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DPIC
GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
GNUCAP_LDFLAGS = $(shell $(GNUCAP_CONF) --ldflags)
GNUCAP_LIBDIR = $(shell $(GNUCAP_CONF) --libdir)
GNUCAP_LIBS = $(shell $(GNUCAP_CONF) --libs)
GNUCAP_PKGLIBDIR = $(shell $(GNUCAP_CONF) --pkglibdir)
GNUCAP_INCLUDEDIR = $(shell $(GNUCAP_CONF) --includedir)
GNUCAP_EXEC_PREFIX = $(shell $(GNUCAP_CONF) --exec-prefix)
GNUCAP_DATA = $(shell $(GNUCAP_CONF) --data)

MANDIR = ${GNUCAP_DATA}/man/man1
EMBED_HEADERS = e_va.raw m_va.raw

all: ${TARGET}

%.o: %.cc
	${CXX} $(GNUCAP_CPPFLAGS) $(CPPFLAGS) -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@ -c

# %.so: %.o
# 	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $(OBJS) $< ${TARGET_LIBS} -o $@


LIBS =

# these are hacks, only used in tests, not installed, ignore.
MODULES = \
  bm_pulse.so \
  modelgen_0.so

CXXFLAGS = -Wall -std=c++11

%.o: %.cc
	${CXX} $(GNUCAP_CPPFLAGS) $(CPPFLAGS) -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@ -c

%.so: %.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $(OBJS) $< ${LIBS_} -o $@

config.h: config.h.in
	sed -e "s#@pkglibdir_expanded@#${GNUCAP_PKGLIBDIR}#" < $< > $@

# temporary workaround, see source
%.raw: %.h
	sed -e 's/^/"/' -e 's/$$/\\n"/' $< > $@
mg_out_root.o: ${EMBED_HEADERS}

mg_main.o: config.h

pkglibdir='${prefix}/lib/gnucap'
pkglibdir_expanded="${prefix}/lib/gnucap"

CLEANFILES = config.h ${TARGET} *.o

modelgen_0.o: $(OBJS)

bm_pulse.so: bm_pulse.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(OBJS) -o $@ $(GNUCAP_LDFLAGS) ${LDFLAGS} $(GNUCAP_LIBS)

install:
	-${MKDIR_P} ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin
	${INSTALL} ${TARGET} ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin

depend: Make.depend
Make.depend: $(SRCS) $(HDRS) ${EMBED_HEADERS} config.h
	$(CXX) -MM -isystem$(GNUCAP_INCLUDEDIR) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(SRCS) > Make.depend

.PHONY: depend
include Make.depend
include ../Make3
