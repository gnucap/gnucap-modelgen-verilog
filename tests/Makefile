GNUCAP_CONF ?= gnucap-conf

CXX = $(shell $(GNUCAP_CONF) --cxx)
GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION -DPIC
GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags) -std=c++11

CPPFLAGS = -DTRACE_UNTESTED -DDO_TRACE
CPPFLAGS = -DTRACE_UNTESTED
REFDIR = ref
TARGETDIR = check
GNUCAP = gnucap
MODELGEN_VERILOG = ../gnucap-mg-vams
CXX = g++

%.o: %.cc
	${CXX} ${CPPFLAGS} ${GNUCAP_CPPFLAGS} -g -O0 -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $< -o $@ -c

%.s: %.cc
	${CXX} -S ${CPPFLAGS} ${GNUCAP_CPPFLAGS} -g -O2 -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $< -o $@

%.s0: %.cc
	${CXX} -S ${CPPFLAGS} ${GNUCAP_CPPFLAGS} -g -O0 -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} -I../include $< -o $@

%.so: %.o
	${CXX} -shared ${CXXFLAGS} -I../include $< ${LIBS} -o $@

%: %.o
	${CXX} -g -rdynamic $< ../bd/lib/.libs/libgnucap.so -o $@

check: check_mg1 check_mg2 check_dump check_test check_v
check_mg1:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "mg1" ${TARGETDIR} ${REFDIR}
check_mg2:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "mg2" ${TARGETDIR} ${REFDIR}
check_dump:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "dump" ${TARGETDIR} ${REFDIR}
check_test:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "test" ${TARGETDIR} ${REFDIR}
check_v:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "v" ${TARGETDIR} ${REFDIR}

clean:
	rm -rf *.o *.cc *.so *.out *~
	rm -rf check

%.cc: %.vams
	${MODELGEN_VERILOG} -I .. --cc $< > $@

.PHONY: check check_mg check_dump check_test check_v clean
