# CPPFLAGS=-DTRACE_UNTESTED

%.o: %.cc
	g++ $(CPPFLAGS) -shared -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@ -c

%.so: %.o
	g++ -shared ${CXXFLAGS} -I../include $< ${LIBS} -o $@

all: modelgen_0.so

check: all
	${MAKE} -C tests check
