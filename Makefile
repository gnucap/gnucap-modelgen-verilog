# CPPFLAGS=-DTRACE_UNTESTED

include Make1

GNUCAP_CONF = gnucap-conf

CXX = $(shell $(GNUCAP_CONF) --cxx)
GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION -DPIC
GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
GNUCAP_LDFLAGS = $(shell $(GNUCAP_CONF) --ldflags)
GNUCAP_LIBDIR = $(shell $(GNUCAP_CONF) --libdir)
GNUCAP_LIBS = $(shell $(GNUCAP_CONF) --libs)
GNUCAP_PKGLIBDIR = $(shell $(GNUCAP_CONF) --pkglibdir)
GNUCAP_EXEC_PREFIX = $(shell $(GNUCAP_CONF) --exec-prefix)

LIBS = -lgnucap
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
	d_vapot.o

CXXFLAGS = -Wall -std=c++11

%.o: %.cc
	${CXX} $(GNUCAP_CPPFLAGS) $(CPPFLAGS) -shared -g -O0 -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@ -c

%.so: %.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $(OBJS) $< ${LIBS_} -o $@

all: ${TARGET} ${MODULES}

check: all
	${MAKE} -C tests check

clean:
	rm -rf *.o ${TARGET} ${MODULES} ${OBJS} ${CLEAN_OBJS}
	make -C tests clean

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(OBJS) -o $@ $(GNUCAP_LDFLAGS) ${LDFLAGS} $(GNUCAP_LIBS)

include Make.depend

modelgen_0.o: $(OBJS)
d_vasrc.so: d_vaflow.o d_vapot.o d_va_filter.o
	${CXX} $(GNUCAP_CPPFLAGS) -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $+ ${LIBS_} -o $@

lang_verilog.so: ${LANG_OBJS}
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

bm_pulse.so: bm_pulse.o
	g++ -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

depend: Make.depend
Make.depend: $(SRCS) $(HDRS)
	$(CXX) -MM ${GNUCAP_CXXFLAGS} $(CXXFLAGS) $(SRCS) > Make.depend

.PHONY: clean depend
