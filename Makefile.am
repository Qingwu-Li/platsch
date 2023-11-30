EXTRA_DIST = README.rst LICENSE

sbin_PROGRAMS = platsch

platsch_SOURCES = platsch.c
platsch_CFLAGS = $(LIBDRM_CFLAGS)
platsch_LDADD = $(LIBDRM_LIBS)

if ENABLE_CAIRO
platsch_SOURCES += cairo.c
platsch_CFLAGS += $(CAIRO_CFLAGS)
platsch_LDADD += $(CAIRO_LIBS)
endif

CLEANFILES = \
        $(DIST_ARCHIVES)

DISTCLEAN = \
        config.log \
        config.status \
        Makefile

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	configure \
	depcomp \
	install-sh \
	Makefile.in \
	missing
