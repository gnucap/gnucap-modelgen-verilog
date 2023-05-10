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

CHECK_TARGETS = check_mg0 check_mg1 check_mg2 check_mg3 check_dump check_test check_v
check: ${CHECK_TARGETS}
${CHECK_TARGETS}: check_%:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "$*" ${TARGETDIR} ${REFDIR}

clean:
	rm -rf *.o *.cc *.so *.out *~
	rm -rf check

%.cc: %.vams
	${MODELGEN_VERILOG} -I .. --cc $< > $@

.PHONY: check ${CHECK_TARGETS} clean
