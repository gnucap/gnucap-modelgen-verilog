# CPPFLAGS=-DTRACE_UNTESTED

include Make1

GNUCAP_CONF = gnucap-conf
INSTALL = install
MKDIR_P = mkdir -p
SUBDIRS = mgsim vams

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

all: all-recursive all-local

all-local: ${TARGET} ${MODULES}

all-recursive: ${TARGET}
	for i in ${SUBDIRS}; do \
		${MAKE} -C $${i} GNUCAP_CONF=${GNUCAP_CONF} CXXFLAGS="${CXXFLAGS}"; \
	done

check: all
	${MAKE} -C tests check GNUCAP_CONF=${GNUCAP_CONF}

clean: clean-recursive
	rm -rf *.o ${TARGET} ${MODULES} ${OBJS} ${CLEAN_OBJS} ${EMBED_HEADERS}

clean-recursive:
	${MAKE} -C tests clean
	for i in ${SUBDIRS}; do \
		${MAKE} -C $${i} clean; \
	done

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(OBJS) -o $@ $(GNUCAP_LDFLAGS) ${LDFLAGS} $(GNUCAP_LIBS)

include Make.depend

# temporary workaround, see source
%.raw: %.h
	sed -e 's/^/"/' -e 's/$$/\\n"/' $< > $@
mg_out_root.o: ${EMBED_HEADERS}

modelgen_0.o: $(OBJS)

bm_pulse.so: bm_pulse.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

depend: Make.depend
	for i in ${SUBDIRS}; do \
		${MAKE} -C $${i} depend; \
	done

Make.depend: $(SRCS) $(HDRS) ${EMBED_HEADERS}
	$(CXX) -MM $(GNUCAP_CPPFLAGS) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(SRCS) > Make.depend

install: install-recursive
	
install-recursive: install-here
	for i in ${SUBDIRS}; do \
		${MAKE} -C $${i} install GNUCAP_CONF=${GNUCAP_CONF}; \
	done

install-here: ${TARGET} ${MODULES}
	-${MKDIR_P} ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin
	${INSTALL} ${TARGET} ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin

	-${MKDIR_P} ${DESTDIR}${MANDIR}
	${INSTALL} ${MANPAGE} ${DESTDIR}${MANDIR}

	for i in ${SUBDIRS}; do \
		install -d $(DESTDIR)$(GNUCAP_PKGLIBDIR)/$${i}; \
	done

	-${MKDIR_P} $(DESTDIR)$(GNUCAP_INCLUDEDIR)
	install $(INSTALL_HDRS) $(DESTDIR)$(GNUCAP_INCLUDEDIR)

RECURSIVE_TARGETS = all-recursive clean-recursive install-recursive
.PHONY: clean depend ${RECURSIVE_TARGETS} install install-here all-local
