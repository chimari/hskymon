bin_PROGRAMS    = hskymon
SUBDIRS = . libnova std_data debian sample icons
hskymon_SOURCES   =\
	 main.c gtkut.c calcpa.c fc.c fc_output.c progress.c adc.c lunar.c solar.c earth.c mercury.c venus.c mars.c jupiter.c saturn.c uranus.c neptune.c pluto.c rise_set.c sidereal_time.c vsop87.c julian_day.c utility.c transform.c precession.c nutation.c dynamical_time.c http-client.c main.h gtkut.h configfile.c skymon.c treeview.c telstat.c remoteObjects.c hsc.h votable.c json_parse.c io_gui.c lgs.c resources.c votable.h remoteObjects.h configfile.h esostd.h ssl.h gen2.h post.h post_sdss.h post_lamost.h post_kepler.h post_smoka.h post_hst.h post_eso.h get_gemini.h io_gui.h lgs.h resources.h observatory.h seimei.h

CLEANFILES      = *~ libnova/*~ std_data/*~ debian/*~ sample/*~ icons/*~
DISTCLEANFILES  = .deps/*.P
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"

if HAVE_WINDRES
HSKYMON_RES = hskymon.res
hskymon.res: hskymon_res.rc
	$(WINDRES) $< -O coff -o $@
else
HSKYMON_RES = 
endif



EXTRA_DIST = autogen.sh\
	     README.jp\
	     gresource.xml\
             subaru_logo.ico\
             hskymon.ico\
             hskymon_res.rc.in\
             hskymon.spec\
             hskymon_icon.png\
             hskymon.1\
             makewin32.sh\
             makewin64.sh\
             makeosx.sh

BUILT_SOURCES=resources.c resources.h

man_MANS = hskymon.1

DISTCLEANFILES  = 
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"

CLEANFILES += $(BUILT_SOURCES)

resource_files=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gresource.xml)

resources.h:
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-header $(srcdir)/gresource.xml

resources.c: gresource.xml $(resource_files) resources.h
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/gresource.xml

INCLUDES        = @GTK_CFLAGS@  $(GTK3_CFLAGS) $(GTKMAC_CFLAGS) $(XMLRPC_CFLAGS) $(XML_CPPFLAGS)
LDADD           = $(OSX_LDADD) @GTK_LIBS@  $(GTK3_LIBS) $(GTKMAC_LIBS) $(WINMM_LIBS) $(WINSOCK_LIBS) $(XMLRPC_LIBS) $(XML_LIBS) $(HSKYMON_RES)

GZIP_ENV =
BZIP2_ENV =
TAR=tar

#ACLOCAL_AMFLAGS = -I m4

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	BZIP2=$(BZIP2_ENV) $(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

