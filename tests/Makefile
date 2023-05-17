GNUCAP_CONF = gnucap-conf

CXX = $(shell $(GNUCAP_CONF) --cxx)
GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION -DPIC

CPPFLAGS = -DTRACE_UNTESTED -DDO_TRACE
CPPFLAGS = -DTRACE_UNTESTED
REFDIR = ref
TARGETDIR = check
GNUCAP = gnucap
MODELGEN_VERILOG = ../modelgen-verilog
CXX = g++
# CXXFLAGS=-O2

%.o: %.cc
	${CXX} ${GNUCAP_CPPFLAGS} $(CPPFLAGS) -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@ -c

%.s: %.cc
	${CXX} -S ${GNUCAP_CPPFLAGS} $(CPPFLAGS) -g -O2 -fPIC ${CXXFLAGS} -I../include $< -o $@

%.s0: %.cc
	${CXX} -S ${GNUCAP_CPPFLAGS} $(CPPFLAGS) -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@

%.so: %.o
	${CXX} -shared ${CXXFLAGS} -I../include $< ${LIBS} -o $@

%: %.o
	${CXX} -g -rdynamic $< ../bd/lib/.libs/libgnucap.so -o $@

check: check_mg check_dump check_test check_v
check_mg:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "mg" ${TARGETDIR} ${REFDIR}
check_dump:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "dump" ${TARGETDIR} ${REFDIR}
check_test:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "test" ${TARGETDIR} ${REFDIR}
check_v:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "v" ${TARGETDIR} ${REFDIR}

clean:
	rm -rf *.o *.cc *.so *.out *~

%.cc: %.vams
	${MODELGEN_VERILOG} -I .. --cc $< > $@

.PHONY: check check_mg check_dump check_test check_v clean
