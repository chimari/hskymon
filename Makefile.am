bin_PROGRAMS    = hskymon
SUBDIRS = . libnova std_data
hskymon_SOURCES   =\
	 main.c calcpa.c fc.c adc.c lunar.c solar.c earth.c mercury.c venus.c mars.c jupiter.c saturn.c uranus.c neptune.c pluto.c rise_set.c sidereal_time.c vsop87.c julian_day.c utility.c transform.c precession.c nutation.c dynamical_time.c http-client.c main.h configfile.c skymon.c treeview.c telstat.c remoteObjects.c hsc.h votable.c votable.h remoteObjects.h configfile.h hskymon_icon.h google_icon.h esostd.h ssl.h

CLEANFILES      = *~ libnova/*~ std_data/*~
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
	     standard.list\
             brightest.lst\
             messier.lst\
	     sample.list\
	     samplePS.list\
             hskymon.ico\
             hskymon_res.rc.in\
             hskymon_icon.png\
             makewin32.sh\
             makewin64.sh\
             ar_u0.xpm\
             ar_u1.xpm\
             ar_u2.xpm\
             ar_u3.xpm\
             ar_d0.xpm\
             ar_d1.xpm\
             ar_d2.xpm\
             ar_d3.xpm

INCLUDES        = @GTK_CFLAGS@ $(XMLRPC_CFLAGS) $(XML_CPPFLAGS) $(DISABLE_DEPRECATED)
LDADD           = @GTK_LIBS@ $(WINMM_LIBS) $(WINSOCK_LIBS) $(XMLRPC_LIBS) $(XML_LIBS) $(HSKYMON_RES)


GZIP_ENV =
BZIP2_ENV =
TAR=tar

#ACLOCAL_AMFLAGS = -I m4

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	BZIP2=$(BZIP2_ENV) $(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

