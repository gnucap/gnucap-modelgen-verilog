# CPPFLAGS=-DTRACE_UNTESTED

include Make1

GNUCAP_CONF = gnucap-conf
INSTALL = install

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
MODULES = \
  bm_pulse.so \
  d_vasrc.so \
  lang_verilog.so \
  modelgen_0.so

# stuff them all into one plugin, for now.
LANG_OBJS = \
	c_param.o \
	lang_verilog.o \
	v_instance.o \
	v_paramset.o \
	v_module.o

CLEAN_OBJS = \
	${LANG_OBJS} \
	d_vaflow.o \
	d_vapot_br.o \
	d_vapot.o

CXXFLAGS = -Wall -std=c++11

%.o: %.cc
	${CXX} $(GNUCAP_CPPFLAGS) $(CPPFLAGS) -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@ -c

%.so: %.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $(OBJS) $< ${LIBS_} -o $@

all: all-recursive all-local

all-local: ${TARGET} ${MODULES}

all-recursive: ${TARGET}
	${MAKE} -C vams GNUCAP_CONF=${GNUCAP_CONF} CXXFLAGS="${CXXFLAGS}"

check: all
	${MAKE} -C tests check GNUCAP_CONF=${GNUCAP_CONF}

clean: clean-recursive
	rm -rf *.o ${TARGET} ${MODULES} ${OBJS} ${CLEAN_OBJS} ${EMBED_HEADERS}

clean-recursive:
	${MAKE} -C tests clean
	${MAKE} -C vams clean

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(OBJS) -o $@ $(GNUCAP_LDFLAGS) ${LDFLAGS} $(GNUCAP_LIBS)

include Make.depend

# temporary workaround, see source
%.raw: %.h
	sed -e 's/^/"/' -e 's/$$/\\n"/' $< > $@
mg_out_root.o: ${EMBED_HEADERS}

modelgen_0.o: $(OBJS)
d_vasrc.so: d_vaflow.o d_vapot.o d_va_filter.o d_vapot_br.o d_va_slew.o
	${CXX} $(GNUCAP_CPPFLAGS) -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $+ ${LIBS_} -o $@

d_vapot.o: d_va.h
d_vaflow.o: d_va.h
d_vapot_br.o: d_va.h

lang_verilog.so: ${LANG_OBJS}
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

bm_pulse.so: bm_pulse.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

depend: Make.depend
Make.depend: $(SRCS) $(HDRS) ${EMBED_HEADERS}
	$(CXX) -MM $(GNUCAP_CPPFLAGS) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(SRCS) > Make.depend

install: install-recursive
	
install-recursive: install-here
	${MAKE} -C vams install GNUCAP_CONF=${GNUCAP_CONF}

install-here: ${TARGET}
	-${INSTALL} -d ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin
	${INSTALL} ${TARGET} ${DESTDIR}${GNUCAP_EXEC_PREFIX}/bin

	-${INSTALL} -d ${DESTDIR}${MANDIR}
	${INSTALL} ${MANPAGE} ${DESTDIR}${MANDIR}

	install -d $(DESTDIR)$(GNUCAP_PKGLIBDIR)/vams
	install $(MODULES) $(DESTDIR)$(GNUCAP_PKGLIBDIR)/vams

	install $(INSTALL_HDRS) $(DESTDIR)$(GNUCAP_INCLUDEDIR)

RECURSIVE_TARGETS = all-recursive clean-recursive install-recursive
.PHONY: clean depend ${RECURSIVE_TARGETS} install install-here all-local
