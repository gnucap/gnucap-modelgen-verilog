
all:
	@echo run configure first.

retest: check-git retest.subs

# does not actually belong here. put into a separate script?
retest.log:
	# ${MAKE} untest
	mkdir -p retest;
	( cd retest; ../configure )
	${MAKE} -C retest CPPFLAGS=-DTRACE_UNTESTED\ -DTRACE_UNTESTED_ONCE\ -DRETEST

	# TODO: parallel
	${MAKE} -j1 -C retest check 2> retest.log

retest.subs: retest.log
	grep -e ^@@@: -e ^@i@ retest.log | \
     sort -u | tr : " " | \
     gawk '{printf "sed -i %s -e '"'"'%s s/\\({\\|:\\)\\(\\ \\|\\t\\)\\?\\(un\\|i\\)tested();/\\1/'"'"' # %s +%s\n", $$2, $$3, $$2, $$3}' \
	  > $@

patch:
	(cd retest/tests; . ../../retest.subs)

check-git:
	[ -d .git ]

untest_exclude = \(extern\)\|\(switch\)\|\(enum\)\|\(union\)\|\(constexpr\)\|\(struct\)\|\(class\)\|\(namespace\)\|\(untested\)\|\(itested\)

# place test calls at test hooks
untest:
	sed -i '/${untest_exclude}/!s/{$$/{ untested();/' */*.cc
	sed -i '/${untest_exclude}/!s/{$$/{ untested();/' */*.h
	sed -i '/tested/!s/{\(.*\)}$$/{ untested();\1}/' */*.h

.PRECIOUS: retest.log
.PHONY: check-git retest.subs retest.log
