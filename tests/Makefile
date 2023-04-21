CPPFLAGS = -DTRACE_UNTESTED -DDO_TRACE
CPPFLAGS = -DTRACE_UNTESTED
REFDIR = ref
TARGETDIR = check
GNUCAP = gnucap
GNUCAP_VERILOG = ../gnucap-verilog
CXX = g++
# CXXFLAGS=-O2

%.o: %.cc
	${CXX} $(CPPFLAGS) -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@ -c

%.s: %.cc
	${CXX} -S $(CPPFLAGS) -g -O2 -fPIC ${CXXFLAGS} -I../include $< -o $@

%.s0: %.cc
	${CXX} -S $(CPPFLAGS) -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@

%.so: %.o
	${CXX} -shared ${CXXFLAGS} -I../include $< ${LIBS} -o $@

%: %.o
	${CXX} -g -rdynamic $< ../bd/lib/.libs/libgnucap.so -o $@

check: check_mg check_dump check_test
check_mg:
	+GNUCAP=${GNUCAP} ./test ${GNUCAP_VERILOG} "mg" ${TARGETDIR} ${REFDIR}
check_dump:
	+GNUCAP=${GNUCAP} ./test ${GNUCAP_VERILOG} "dump" ${TARGETDIR} ${REFDIR}
check_test:
	+GNUCAP=${GNUCAP} ./test ${GNUCAP_VERILOG} "test" ${TARGETDIR} ${REFDIR}

clean:
	rm -rf *.o *.cc *.so *.out *~

%.cc: %.vams
	${GNUCAP_VERILOG} -I .. --cc $< > $@

.PHONY: check check_mg check_dump check_test clean
