# CPPFLAGS=-DTRACE_UNTESTED

include Make1
LIBS = -lgnucap
LDFLAGS = -L/usr/local/lib # ask gnucap-conf?
MODULES = \
  modelgen_0.so \
  d_vasrc.so \
  lang_verilog.so

# stuff them all into on plugin, for now.
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
	g++ $(CPPFLAGS) -shared -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@ -c

%.so: %.o
	g++ -shared ${CXXFLAGS} $(OBJS) -I../include $< ${LIBS_} -o $@

all: ${TARGET} ${MODULES}

check: all
	${MAKE} -C tests check

clean:
	rm -rf *.o ${TARGET} ${MODULES} ${OBJS} ${CLEAN_OBJS}
	make -C tests clean

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS) $(LDFLAGS)

include Make.depend

modelgen_0.o: $(OBJS)
d_vasrc.so: d_vaflow.o d_vapot.o
	g++ -shared ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

lang_verilog.so: ${LANG_OBJS}
	g++ -shared ${CXXFLAGS} -I../include $+ ${LIBS_} -o $@

depend: Make.depend
Make.depend: $(SRCS) $(HDRS)
	$(CXX) -MM $(CXXFLAGS) $(SRCS) > Make.depend

.PHONY: clean depend
