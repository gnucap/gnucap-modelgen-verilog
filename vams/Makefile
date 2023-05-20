
include Make1

MODELGEN = ../gnucap-mg-vams
GNUCAP_CONF = gnucap-conf

CXX = $(shell $(GNUCAP_CONF) --cxx)
GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION -DPIC
GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
GNUCAP_LDFLAGS = $(shell $(GNUCAP_CONF) --ldflags)
GNUCAP_LIBDIR = $(shell $(GNUCAP_CONF) --libdir)
GNUCAP_LIBS = $(shell $(GNUCAP_CONF) --libs)
GNUCAP_PKGLIBDIR = $(shell $(GNUCAP_CONF) --pkglibdir)
GNUCAP_EXEC_PREFIX = $(shell $(GNUCAP_CONF) --exec-prefix)


%.so: %.o
	${CXX} -shared ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@

%.o: %.cc
	${CXX} $(GNUCAP_CPPFLAGS) $(CPPFLAGS) -shared -fPIC ${GNUCAP_CXXFLAGS} ${CXXFLAGS} $< -o $@ -c

%.cc: %.vams
	${MODELGEN} -I .. --cc $< > $@

all: ${TARGET}

clean:
	rm -r -f ${TARGET} ${TARGET:.so=.o} ${TARGET:.so=.cc}

install:
	install -d $(DESTDIR)$(GNUCAP_PKGLIBDIR)/vams
	install $(TARGET) $(DESTDIR)$(GNUCAP_PKGLIBDIR)/vams

.PHONY: clean install
.SECONDARY: ${TARGET:.so=.cc}
