# CPPFLAGS=-DTRACE_UNTESTED

include Make1
LIBS = -lgnucap
LDFLAGS = -L/usr/local/lib # ask gnucap-conf?

%.o: %.cc
	g++ $(CPPFLAGS) -shared -g -O0 -fPIC ${CXXFLAGS} -I../include $< -o $@ -c

%.so: %.o
	g++ -shared ${CXXFLAGS} $(OBJS) -I../include $< ${LIBS_} -o $@


all: ${TARGET} modelgen_0.so

check: all
	${MAKE} -C tests check

clean:
	rm -rf *.o ${TARGET}

$(TARGET): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS) $(LDFLAGS)

include Make.depend

modelgen_0.o: $(OBJS)
depend: Make.depend
Make.depend: $(SRCS) $(HDRS)
	$(CXX) -MM $(CXXFLAGS) $(SRCS) > Make.depend

.PHONY: clean depend
