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
	${CXX} -g -rdynamic $< -lgnucap -o $@

CHECK_SECTIONS = mg0 mg1 mg2 mg3 dump test v
CHECK_TARGETS = ${CHECK_SECTIONS:%=check/%.diffs}

check: check/.diffs
check/.diffs: ${CHECK_TARGETS}
	cat $+ > $@

# BUG. "+" breaks -n
${CHECK_TARGETS}: check/%.diffs:
	+GNUCAP=${GNUCAP} ./test ${MODELGEN_VERILOG} "$*" ${TARGETDIR} ${REFDIR}

clean:
	rm -rf *.o *.cc *.so *.out *~
	rm -rf check

%.cc: %.vams
	${MODELGEN_VERILOG} -I .. --cc $< > $@

.PHONY: check ${CHECK_TARGETS} clean
