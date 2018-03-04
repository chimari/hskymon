//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      main.h  --- Main header file
//   
//                                           2012.10.22  A.Tajitsu

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

#undef ALLSKY_DEBUG
#undef SKYMON_DEBUG
#undef HTTP_DEBUG

#include<glib.h>
#include<gtk/gtk.h>

#ifdef USE_GTKMACINTEGRATION
#include<gtkmacintegration/gtkosxapplication.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#ifdef HAVE_PWD_H
#include<pwd.h>
#endif
#include<sys/types.h>
#include<errno.h>
#include<math.h>
#include<string.h>

#ifdef USE_WIN32
#include <windows.h>
#include <winnt.h>
#endif

#ifdef USE_XMLRPC
#include "remoteObjects.h"
#endif

#include "libnova/libnova.h"

#include "gen2.h"

#include "post.h"
#include "post_sdss.h"
#include "post_lamost.h"
#include "post_smoka.h"
#include "post_hst.h"
#include "post_eso.h"
#include "get_gemini.h"


#ifdef USE_WIN32
#define USER_CONFFILE "hskymon.ini"
#else
#define USER_CONFFILE ".hskymon"
#endif


#define AU_IN_KM 149597870.700

#ifdef SIGRTMIN
#define SIGHSKYMON1 SIGRTMIN
#define SIGHSKYMON2 SIGRTMIN+1
#else
#define SIGHSKYMON1 SIGUSR1
#define SIGHSKYMON2 SIGUSR2
#endif

#define WWW_BROWSER "firefox"

#define DEFAULT_URL "http://www.naoj.org/Observing/tools/hskymon"
#define VER_HOST "www.naoj.org"
#define VER_PATH "/Observing/tools/hskymon/ver"

#ifdef USE_WIN32
#define DSS_URL "http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "http://simbad.harvard.edu/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML"
#define DR8_URL "http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf"
#define DR14_URL "http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf"
#define NED_URL "http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define MAST_URL "http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER"
#define MASTP_URL "https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type"
#define SPITZER_URL "http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true"
#define CASSIS_URL "http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120"
#define RAPID_URL "http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptypes<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define MIRSTD_URL "http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define SSLOC_URL "http://simbad.harvard.edu/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define STD_SIMBAD_URL "http://simbad.harvard.edu/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML"
#define FCDB_NED_URL "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define FCDB_SDSS_URL "http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_URL "http://dr3.lamost.org/spectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "http://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d"
#define FCDB_SMOKA_SHOT_URL "http://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all"
#define FCDB_HST_URL "http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst&dataid=%s"
#define FCDB_ESO_URL "http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s&format=DecimDeg&tab_stat_plot=on&aladin_colour=aladin_instrument"
#define FCDB_GEMINI_URL "https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s"
#define TRDB_GEMINI_URL "https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT"
#define HASH_URL "http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d"

#elif defined(USE_OSX)
// in OSX    
//    - add  "open" at the beginning
//    - "&" --> "\\&"
#define DSS_URL "open http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick\\&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf\\&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "open http://simbad.harvard.edu/simbad/sim-coo?CooDefinedFrames=none\\&CooEquinox=2000\\&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf\\&submit=submit%%20query\\&Radius.unit=arcmin\\&CooEqui=2000\\&CooFrame=FK5\\&Radius=2\\&output.format=HTML"
#define DR8_URL "open http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf\\&dec=%s%d:%d:%.2lf"
#define DR14_URL "open http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf\\&dec=%s%lf"
#define NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search\\&in_csys=Equatorial\\&in_equinox=J2000.0\\&lon=%d%%3A%d%%3A%.2lf\\&lat=%s%d%%3A%d%%3A%.2lf\\&radius=2.0\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&z_constraint=Unconstrained\\&z_value1=\\&z_value2=\\&z_unit=z\\&ot_include=ANY\\&nmp_op=ANY\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=Distance+to+search+center\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define MAST_URL "open http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf\\&max_records=10\\&action=Search\\&resolver=SIMBAD\\&missions[]=EUVE\\&missions[]=WFC3-IMAGE\\&missions[]=WFPC1\\&missions[]=WFPC2\\&missions[]=FOC\\&missions[]=ACS-IMAGE\\&missions[]=UIT\\&missions[]=STIS-IMAGE\\&missions[]=COS-IMAGE\\&missions[]=GALEX\\&missions[]=XMM-OM\\&missions[]=NICMOS-IMAGE\\&missions[]=FUSE\\&missions[]=IMAPS\\&missions[]=BEFS\\&missions[]=TUES\\&missions[]=IUE\\&missions[]=COPERNICUS\\&missions[]=HUT\\&missions[]=WUPPE\\&missions[]=GHRS\\&missions[]=STIS-SPECTRUM\\&missions[]=COS-SPECTRUM\\&missions[]=WFC3-SPECTRUM\\&missions[]=ACS-SPECTRUM\\&missions[]=FOS\\&missions[]=HPOL\\&missions[]=NICMOS-SPECTRUM\\&missions[]=FGS\\&missions[]=HSP\\&missions[]=KEPLER"
#define MASTP_URL "open https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "open https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "open https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "open http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All\\&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf\\&mode=cone\\&radius=2\\&radunits=arcmin\\&range=6.25+Deg.\\&data=Data+Set+Type\\&radnum=2222\\&irsa=IRSA+Only\\&submit=Get+Inventory\\&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type\\&url=%%2Fworkspace%%2FTMP_3hX3SO_29666\\&dir=%%2Fwork%%2FTMP_3hX3SO_29666\\&snull=matches+only\\&datav=Data+Set+Type"
#define SPITZER_URL "open http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition\\&DoSearch=true\\&SearchByPosition.field.radius=0.033333333\\&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000\\&SimpleTargetPanel.field.resolvedBy=nedthensimbad\\&MoreOptions.field.prodtype=aor,pbcd\\&startIdx=0\\&pageSize=0\\&shortDesc=Position\\&isBookmarkAble=true\\&isDrillDownRoot=true\\&isSearchResult=true"
#define CASSIS_URL "open http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf\\&dec=%.6lf&radius=120"
#define RAPID_URL "open http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define MIRSTD_URL "open http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define SSLOC_URL "open http://simbad.harvard.edu/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define STD_SIMBAD_URL "open http://simbad.harvard.edu/simbad/sim-id?Ident=%s\\&NbIdent=1\\&Radius=2\\&Radius.unit=arcmin\\&submit=submit+id\\&output.format=HTML"
#define FCDB_NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s\\&extend=no\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=RA+or+Longitude\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define FCDB_SDSS_URL "open http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_URL "open http://dr3.lamost.org/spectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "open http://smoka.nao.ac.jp/info.jsp?frameid=%s\\&date_obs=%s\\&i=%d"
#define FCDB_SMOKA_SHOT_URL "open http://smoka.nao.ac.jp/fssearch?frameid=%s*\\&instruments=%s\\&obs_mod=all\\&data_typ=all\\&dispcol=default\\&diff=1000\\&action=Search\\&asciitable=table\\&obs_cat=all"
#define FCDB_HST_URL "open http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst\\&dataid=%s"
#define FCDB_ESO_URL "open http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s\\&format=DecimDeg\\&tab_stat_plot=on\\&aladin_colour=aladin_instrument"
#define FCDB_GEMINI_URL "open https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s"
#define TRDB_GEMINI_URL "open https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT"
#define HASH_URL "open http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d"

#else
// in UNIX    
//    - just add a pair of \" at the beginning and the end of each phrase.
#define DSS_URL "\"http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey\""
#define SIMBAD_URL "\"http://simbad.harvard.edu/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML\""
#define DR8_URL "\"http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf\""
#define DR14_URL "\"http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf\""
#define NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define MAST_URL "\"http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER\""
#define MASTP_URL "\"https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf\""
#define KECK_URL "\"https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single\""
#define GEMINI_URL "\"https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT\""
#define IRSA_URL "\"http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type\""
#define SPITZER_URL "\"http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true\""
#define CASSIS_URL "\"http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120\""
#define RAPID_URL "\"http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define MIRSTD_URL "\"http://simbad.harvard.edu/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define SSLOC_URL "\"http://simbad.harvard.edu/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define STD_SIMBAD_URL "\"http://simbad.harvard.edu/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML\""
#define FCDB_NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define FCDB_SDSS_URL "\"http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s\""
#define FCDB_LAMOST_URL "\"http://dr3.lamost.org/spectrum/view?obsid=%d\""
#define FCDB_SMOKA_URL "\"http://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d\""
#define FCDB_SMOKA_SHOT_URL "\"http://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all\""
#define FCDB_HST_URL "\"http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst&dataid=%s\""
#define FCDB_ESO_URL "\"http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s&format=DecimDeg&tab_stat_plot=on&aladin_colour=aladin_instrument\""
#define FCDB_GEMINI_URL "\"https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s\""
#define TRDB_GEMINI_URL "\"https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT\""
#define HASH_URL "\"http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d\""
#endif

#ifdef USE_WIN32
#define GMAP_URL "http://maps.google.com/maps?q=%lf,%lf%%28here!%%29&hl=en"
#elif defined(USE_OSX)
#define GMAP_URL "open http://maps.google.com/maps?q=%lf,%lf%%28here!%%29\\&hl=en"
#else
#define GMAP_URL "\"http://maps.google.com/maps?q=%lf,%lf%%28here!%%29&hl=en\""
#endif

#define DSS_ARCMIN_MIN 1
#define DSS_ARCMIN 3
#define DSS_ARCMIN_MAX 120
#define DSS_PIX 1500

#define FCDB_ARCMIN_MAX 100
#define FCDB_PS1_ARCMIN_MAX 60
#define FCDB_USNO_ARCMIN_MAX 24

#define STDDB_PATH_SSLOC "/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_RAPID "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_MIRSTD "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_FILE_XML "simbad.xml"

#define FCDB_HOST_SIMBAD_STRASBG "simbad.u-strasbg.fr"
#define FCDB_HOST_SIMBAD_HARVARD "simbad.harvard.edu"
#define FCDB_PATH "/simbad/sim-sam?Criteria=region%%28box%%2C%lf%s%lf%%2C%+lfm%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_FILE_XML "database_fc.xml"
#define FCDB_FILE_TXT "database_fc.txt"
#define FCDB_FILE_HTML "database_fc.html"
#define FCDB_FILE_JSON "database_fc.json"

#define FCDB_HOST_NED "ned.ipac.caltech.edu"
#define FCDB_NED_PATH "/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=%.2lf&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY%sout_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=0&img_stamp=YES&of=xml_main"

#define FCDB_HOST_GSC "gsss.stsci.edu"
#define FCDB_GSC_PATH "/webservices/vo/ConeSearch.aspx?RA=%lf&DEC=%+lf&SR=%lf%sMAX_OBJ=5000&FORMAT=VOTABLE"

#define FCDB_HOST_PS1 "gsss.stsci.edu"
#define FCDB_PS1_PATH  "/webservices/vo/CatalogSearch.aspx?CAT=PS1V3OBJECTS&RA=%lf&DEC=%+lf&SR=%lf&MINDET=%d%sMAXOBJ=5000"

#define FCDB_HOST_SDSS "skyserver.sdss.org"
#define FCDB_SDSS_PATH "/dr14/en/tools/search/x_results.aspx"

#define FCDB_HOST_USNO "www.nofs.navy.mil"
#define FCDB_USNO_PATH "/cgi-bin/vo_cone.cgi?CAT=USNO-B1&RA=%lf&DEC=%+lf&SR=%lf%sVERB=1"

#define FCDB_HOST_GAIA "vizier.u-strasbg.fr"
#define FCDB_GAIA_PATH "/viz-bin/votable?-source=I/337/gaia&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_2MASS "gsss.stsci.edu"
#define FCDB_2MASS_PATH "/webservices/vo/CatalogSearch.aspx?CAT=2MASS&RA=%lf&DEC=%+lf&SR=%lf%sMAXOBJ=5000"

#define FCDB_HOST_WISE "vizier.u-strasbg.fr"
#define FCDB_WISE_PATH "/viz-bin/votable?-source=II/311/wise&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_IRC "vizier.u-strasbg.fr"
#define FCDB_IRC_PATH "/viz-bin/votable?-source=II/297/irc&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_FIS "vizier.u-strasbg.fr"
#define FCDB_FIS_PATH "/viz-bin/votable?-source=II/298/fis&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=r&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_LAMOST "dr3.lamost.org"
#define FCDB_LAMOST_PATH "/q"

#define FCDB_HOST_SMOKA "smoka.nao.ac.jp"
#define FCDB_SMOKA_PATH "/fssearch"

#define FCDB_HOST_HST "archive.stsci.edu"
#define FCDB_HST_PATH "/hst/search.php"

#define FCDB_HOST_ESO "archive.eso.org"
#define FCDB_ESO_PATH "/wdb/wdb/eso/eso_archive_main/query"

#define FCDB_HOST_GEMINI "archive.gemini.edu"
#define FCDB_GEMINI_PATH "/jsonsummary/sr=%d/notengineering%sra=%.6lf/science/dec=%s%.6lf/NotFail/OBJECT/present/canonical"
#define TRDB_GEMINI_PATH "/jsonsummary/sr=%d/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT/present/canonical"



#define ADDOBJ_SIMBAD_PATH "/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=VOTABLE"
#define ADDOBJ_NED_PATH "/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES&of=xml_main"

#define FC_MAX_MAG 5

#define FC_HOST_STSCI "archive.stsci.edu"
#define FC_PATH_STSCI "/cgi-bin/dss_search?v=%s&r=%d+%d+%lf&d=%s%d+%d+%lf&e=J2000&h=%d.0&w=%d.0&f=gif&c=none&fov=NONE&v3="
#define FC_FILE_GIF "dss.gif"
#define FC_FILE_JPEG "dss.jpg"
#define FC_FILE_HTML "dss.html"

#define FC_HOST_ESO "archive.eso.org"
#define FC_PATH_ESO "/dss/dss?ra=%d%%20%d%%20%lf&dec=%s%d%%20%d%%20%lf&mime-type=image/gif&x=%d.0&y=%d.0&Sky-Survey=%s"


#define FC_HOST_SKYVIEW "skyview.gsfc.nasa.gov"
#define FC_PATH_SKYVIEW "/current/cgi/runquery.pl?survey=%s&coordinates=J%.1lf&projection=Tan&scaling=%s&sampler=LI&lut=colortables/blue-white.bin%ssize=%lf,%lf&pixels=%d&position=%lf,%lf"
#define FC_PATH_SKYVIEW_RGB "/current/cgi/runquery.pl?survey=%s&coordinates=J%.1lf&projection=Tan&scaling=%s&sampler=LI&lut=colortables/B-W%%20Linear&size=%lf,%lf&pixels=%d&position=%lf,%lf"

#define FC_SRC_STSCI_DSS1R "poss1_red"
#define FC_SRC_STSCI_DSS1B "poss1_blue"
#define FC_SRC_STSCI_DSS2R "poss2ukstu_red"
#define FC_SRC_STSCI_DSS2B "poss2ukstu_blue"
#define FC_SRC_STSCI_DSS2IR "poss2ukstu_ir"

#define FC_SRC_ESO_DSS1R "DSS1"
#define FC_SRC_ESO_DSS2R "DSS2-red"
#define FC_SRC_ESO_DSS2B "DSS2-blue"
#define FC_SRC_ESO_DSS2IR "DSS2-infrared"

#define FC_SRC_SKYVIEW_GALEXF "GALEX%20Far%20UV"
#define FC_SRC_SKYVIEW_GALEXN "GALEX%20Near%20UV"
#define FC_SRC_SKYVIEW_DSS1R "DSS1%20Red"
#define FC_SRC_SKYVIEW_DSS1B "DSS1%20Blue"
#define FC_SRC_SKYVIEW_DSS2R "DSS2%20Red"
#define FC_SRC_SKYVIEW_DSS2B "DSS2%20Blue"
#define FC_SRC_SKYVIEW_DSS2IR "DSS2%20IR"
#define FC_SRC_SKYVIEW_SDSSU "SDSSu"
#define FC_SRC_SKYVIEW_SDSSG "SDSSg"
#define FC_SRC_SKYVIEW_SDSSR "SDSSr"
#define FC_SRC_SKYVIEW_SDSSI "SDSSi"
#define FC_SRC_SKYVIEW_SDSSZ "SDSSz"
#define FC_SRC_SKYVIEW_2MASSJ "2MASS-J"
#define FC_SRC_SKYVIEW_2MASSH "2MASS-H"
#define FC_SRC_SKYVIEW_2MASSK "2MASS-K"
#define FC_SRC_SKYVIEW_WISE34 "WISE%203.4"
#define FC_SRC_SKYVIEW_WISE46 "WISE%204.6"
#define FC_SRC_SKYVIEW_WISE12 "WISE%2012"
#define FC_SRC_SKYVIEW_WISE22 "WISE%2022"
#define FC_SRC_SKYVIEW_AKARIN60 "AKARI%20N60"
#define FC_SRC_SKYVIEW_AKARIWS "AKARI%20WIDE-S"
#define FC_SRC_SKYVIEW_AKARIWL "AKARI%20WIDE-L"
#define FC_SRC_SKYVIEW_AKARIN160 "AKARI%20N160"
#define FC_SRC_SKYVIEW_NVSS "NVSS"

#define FC_HOST_SDSS "casjobs.sdss.org"
#define FC_PATH_SDSS "/ImgCutoutDR7/getjpeg.aspx?ra=%lf&dec=%+lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_SDSS8 "skyservice.pha.jhu.edu"
#define FC_PATH_SDSS8 "/DR8/ImgCutout/getjpeg.aspx?ra=%lf&dec=%lf&scale=%f&opt=&width=%d&height=%d&opt=%s%s&query=%s%s"
#define SDSS_SCALE 0.39612
#define FC_HOST_SDSS13 "skyserver.sdss.org"
#define FC_PATH_SDSS13 "/dr14/SkyServerWS/ImgCutout/getjpeg?TaskName=Skyserver.Chart.image&ra=%lf&dec=%lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_PANCOL "ps1images.stsci.edu"
#define FC_PATH_PANCOL "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=color&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANG "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=g&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANR "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=r&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANI "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=i&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANZ "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=z&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANY "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=y&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="


// Finding Chart
enum{FC_STSCI_DSS1R, 
     FC_STSCI_DSS1B, 
     FC_STSCI_DSS2R,
     FC_STSCI_DSS2B,
     FC_STSCI_DSS2IR,
     FC_SEP1,
     FC_ESO_DSS1R,
     FC_ESO_DSS2R,
     FC_ESO_DSS2B,
     FC_ESO_DSS2IR,
     FC_SEP2,
     FC_SKYVIEW_GALEXF,
     FC_SKYVIEW_GALEXN,
     FC_SKYVIEW_DSS1R,
     FC_SKYVIEW_DSS1B,
     FC_SKYVIEW_DSS2R,
     FC_SKYVIEW_DSS2B,
     FC_SKYVIEW_DSS2IR,
     FC_SKYVIEW_SDSSU,
     FC_SKYVIEW_SDSSG,
     FC_SKYVIEW_SDSSR,
     FC_SKYVIEW_SDSSI,
     FC_SKYVIEW_SDSSZ,
     FC_SKYVIEW_2MASSJ,
     FC_SKYVIEW_2MASSH,
     FC_SKYVIEW_2MASSK,
     FC_SKYVIEW_WISE34,
     FC_SKYVIEW_WISE46,
     FC_SKYVIEW_WISE12,
     FC_SKYVIEW_WISE22,
     FC_SKYVIEW_AKARIN60,
     FC_SKYVIEW_AKARIWS,
     FC_SKYVIEW_AKARIWL,
     FC_SKYVIEW_AKARIN160,
     FC_SKYVIEW_NVSS,
     FC_SKYVIEW_RGB,
     FC_SEP3,
     FC_SDSS,
     FC_SDSS13,
     FC_SEP4,
     FC_PANCOL,
     FC_PANG,
     FC_PANR,
     FC_PANI,
     FC_PANZ,
     FC_PANY} ModeFC;


#define PANSTARRS_MAX_ARCMIN 25

#define FC_WINSIZE 400
enum{ FC_OUTPUT_WINDOW, FC_OUTPUT_PDF, FC_OUTPUT_PRINT} FCOutput;
enum{ FC_INST_HDS, FC_INST_HDSAUTO, FC_INST_HDSZENITH, FC_INST_NONE, FC_INST_IRCS, FC_INST_COMICS, FC_INST_FOCAS, FC_INST_MOIRCS, FC_INST_FMOS, FC_INST_SPCAM, FC_INST_HSCDET,FC_INST_HSCA, FC_INST_NO_SELECT} FCInst;
enum{ FC_SCALE_LINEAR, FC_SCALE_LOG, FC_SCALE_SQRT, FC_SCALE_HISTEQ, FC_SCALE_LOGLOG} FCScale;

enum{ FCDB_SIMBAD_STRASBG, FCDB_SIMBAD_HARVARD } FCDBSimbad;

#define ADC_WINSIZE 400
#define ADC_SLIT_WIDTH 0.4
#define ADC_SIZE 5.0
#define ADC_SEEING 0.6
enum{ ADC_INST_IMR, ADC_INST_HDSAUTO, ADC_INST_HDSZENITH} ADC_Inst;

//  Instrument
#define HDS_SLIT_MASK_ARCSEC 9.2
// micron
#define HDS_SLIT_LENGTH 10000
#define HDS_SLIT_WIDTH 500
#define HDS_PA_OFFSET (-58.4)
#define HDS_SIZE 3

#define FMOS_SIZE 40
#define FMOS_R_ARCMIN 30

#define SPCAM_X_ARCMIN 34
#define SPCAM_Y_ARCMIN 27
#define SPCAM_GAP_ARCSEC 14.
#define SPCAM_SIZE 40

#define HSC_R_ARCMIN 90
enum{ HSC_DITH_NO, HSC_DITH_5, HSC_DITH_N} HSC_Dith;
#define HSC_DRA 120
#define HSC_DDEC 120
#define HSC_TDITH 15
#define HSC_RDITH 120

#define FOCAS_R_ARCMIN 6
#define FOCAS_GAP_ARCSEC 5.
#define FOCAS_SIZE 10

#define IRCS_X_ARCSEC 54.
#define IRCS_Y_ARCSEC 54.
#define IRCS_SIZE 3
#define IRCS_TTGS_ARCMIN 2

#define COMICS_X_ARCSEC 30.
#define COMICS_Y_ARCSEC 40.
#define COMICS_SIZE 3

#define MOIRCS_X_ARCMIN 4.0
#define MOIRCS_Y_ARCMIN 7.0
#define MOIRCS_GAP_ARCSEC 2.
#define MOIRCS_VIG1X_ARCSEC 29.
#define MOIRCS_VIG1Y_ARCSEC 29.
#define MOIRCS_VIG2X_ARCSEC 47.
#define MOIRCS_VIG2Y_ARCSEC 45.
#define MOIRCS_VIGR_ARCMIN 6.
#define MOIRCS_SIZE 10

#define HSC_SIZE 110

// Object Type
enum{OBJTYPE_OBJ,
     OBJTYPE_STD,
     OBJTYPE_TTGS
} ObjType; 

#define ADDTYPE_OBJ   -1
#define ADDTYPE_STD   -2
#define ADDTYPE_TTGS  -3

// OpenSSL
typedef enum {
	SSL_NONE,
	SSL_TUNNEL,
	SSL_STARTTLS
} SSLType;

typedef enum {
	SSL_CERT_NONE,
	SSL_CERT_ACCEPT,
	SSL_CERT_DENY
} SSLCertRes;


// Treeview
enum
{
  COLUMN_OBJ_DISP,
#ifdef USE_XMLRPC
  COLUMN_OBJ_LOCK,
#endif
  COLUMN_OBJ_NUMBER,
  COLUMN_OBJ_OPENUM,
  COLUMN_OBJ_DEF,
  COLUMN_OBJ_NAME,
  COLUMN_OBJ_AZ,
  COLUMN_OBJ_EL,
  COLUMN_OBJ_PIXBUF,
  COLUMN_OBJ_ELMAX,
  COLUMN_OBJ_SECZ,
  COLUMN_OBJ_HA,
#ifdef USE_XMLRPC
  COLUMN_OBJ_SLEW,
#endif
  COLUMN_OBJ_AD,
  COLUMN_OBJ_ADPA,
  COLUMN_OBJ_HPA,
  COLUMN_OBJ_MOON,
  COLUMN_OBJ_RA,
  COLUMN_OBJ_DEC,
  COLUMN_OBJ_EQUINOX,
  //COLUMN_OBJ_SETUP
  COLUMN_OBJ_NOTE,
  NUM_OBJ_COLUMNS
};

// StdTreeview
enum
{
  COLUMN_STD_NUMBER,
  COLUMN_STD_NAME,
  COLUMN_STD_RA,
  COLUMN_STD_DEC,
  COLUMN_STD_SP,
  COLUMN_STD_SEP,
  COLUMN_STD_ROT,
  COLUMN_STD_U,
  COLUMN_STD_B,
  COLUMN_STD_V,
  COLUMN_STD_R,
  COLUMN_STD_I,
  COLUMN_STD_J,
  COLUMN_STD_H,
  COLUMN_STD_K,
  /*  IRAS depricated in SIMBAD 2017-04
  COLUMN_STD_F12,
  COLUMN_STD_F25,
  COLUMN_STD_F60,
  COLUMN_STD_F100,
  */
  NUM_COLUMN_STD
};

// FCDBTreeview
enum
{
  COLUMN_FCDB_NUMBER,
  COLUMN_FCDB_NAME,
  COLUMN_FCDB_RA,
  COLUMN_FCDB_DEC,
  COLUMN_FCDB_SEP,
  COLUMN_FCDB_OTYPE,
  COLUMN_FCDB_SP,
  COLUMN_FCDB_U,
  COLUMN_FCDB_B,
  COLUMN_FCDB_V,
  COLUMN_FCDB_R,
  COLUMN_FCDB_I,
  COLUMN_FCDB_J,
  COLUMN_FCDB_H,
  COLUMN_FCDB_K,
  COLUMN_FCDB_NEDMAG,
  COLUMN_FCDB_NEDZ,
  COLUMN_FCDB_REF,
  COLUMN_FCDB_PLX,
  COLUMN_FCDB_FID,
  COLUMN_FCDB_DATE,
  COLUMN_FCDB_MODE,
  COLUMN_FCDB_TYPE,
  COLUMN_FCDB_FIL,
  COLUMN_FCDB_WV,
  COLUMN_FCDB_OBS,
  NUM_COLUMN_FCDB
};


// Tree DataBase
enum
{
  COLUMN_TRDB_NUMBER,
  COLUMN_TRDB_OPENUM,
  COLUMN_TRDB_NAME,
  COLUMN_TRDB_DATA,
  NUM_COLUMN_TRDB
};

// FCDB_TYPE
enum
{
  FCDB_TYPE_SIMBAD,
  FCDB_TYPE_NED,
  FCDB_TYPE_GSC,
  FCDB_TYPE_PS1,
  FCDB_TYPE_SDSS,
  FCDB_TYPE_LAMOST,
  FCDB_TYPE_USNO,
  FCDB_TYPE_GAIA,
  FCDB_TYPE_2MASS,
  FCDB_TYPE_WISE,
  FCDB_TYPE_IRC,
  FCDB_TYPE_FIS,
  FCDB_TYPE_SMOKA,
  FCDB_TYPE_HST,
  FCDB_TYPE_ESO,
  FCDB_TYPE_GEMINI,
  FCDB_TYPE_WWWDB_SMOKA,
  FCDB_TYPE_WWWDB_HST,
  FCDB_TYPE_WWWDB_ESO,
  TRDB_TYPE_SMOKA,
  TRDB_TYPE_HST,
  TRDB_TYPE_ESO,
  TRDB_TYPE_GEMINI,
  TRDB_TYPE_WWWDB_SMOKA,
  TRDB_TYPE_WWWDB_HST,
  TRDB_TYPE_WWWDB_ESO,
  TRDB_TYPE_FCDB_SMOKA,
  TRDB_TYPE_FCDB_HST,
  TRDB_TYPE_FCDB_ESO,
  TRDB_TYPE_FCDB_GEMINI
};

enum
{
  FCDB_BAND_NOP,
  FCDB_BAND_U,
  FCDB_BAND_B,
  FCDB_BAND_V,
  FCDB_BAND_R,
  FCDB_BAND_I,
  FCDB_BAND_J,
  FCDB_BAND_H,
  FCDB_BAND_K,
  NUM_FCDB_BAND
};

enum
{
  FCDB_OTYPE_ALL,
  FCDB_OTYPE_STAR,
  FCDB_OTYPE_ISM,
  FCDB_OTYPE_PN,
  FCDB_OTYPE_HII,
  FCDB_OTYPE_GALAXY,
  FCDB_OTYPE_QSO,
  FCDB_OTYPE_GAMMA,
  FCDB_OTYPE_X,
  FCDB_OTYPE_IR,
  FCDB_OTYPE_RADIO,
  NUM_FCDB_OTYPE
};

enum
{
  FCDB_NED_OTYPE_ALL,
  FCDB_NED_OTYPE_EXTRAG,
  FCDB_NED_OTYPE_QSO,
  FCDB_NED_OTYPE_STAR,
  FCDB_NED_OTYPE_SN,
  FCDB_NED_OTYPE_PN,
  FCDB_NED_OTYPE_HII,
  NUM_FCDB_NED_OTYPE
};

#define DEF_TREE_WIDTH 320
#define DEF_TREE_HEIGHT 360

enum
{
  COLUMN_NUMBER_TEXT,
  NUM_NUMBER_COLUMNS
};


enum{
  CHECK_TARGET_DEF_NOUSE,
  CHECK_TARGET_DEF_OBJECT,
  CHECK_TARGET_DEF_STANDARD
};


// Sky Monitor
#define SKYMON_WINSIZE 500

#ifdef USE_WIN32
#define SKYMON_FONT "arial 10"
#else
#define SKYMON_FONT "Suns 10"
#endif

#define DEF_SIZE_EDGE 4

#define PLOT_WINSIZE 400

#define PLOT_WIDTH_MM 160
#define PLOT_HEIGHT_MM 160

#define PLOT_HST0 17
#define PLOT_HST1 31

// popup message
#define GTK_MSG
// time out for error popup window [sec]
#define POPUP_TIMEOUT 2


//#define VERSION "0.8.0"
#define AZEL_INTERVAL 60*1000
#define TELSTAT_INTERVAL 3*1000
#define SKYCHECK_INTERVAL 500
#define CHECK_ALLSKY_INTERVAL 3*1000


#define SKYMON_INTERVAL 200
#define SKYMON_STEP 5

#define MAX_OBJECT 5000
#define MAX_ROPE 32
#define MAX_STD 100
#define MAX_FCDB 5000
#define MAX_TRDB_BAND 100

#ifdef USE_XMLRPC
enum{ ROPE_DIR, ROPE_ALL} ROPEMode;
#endif

#define BUFFSIZE 65535

#define VEL_AZ_SUBARU 0.5  // [deg/sec]
#define VEL_EL_SUBARU 0.5  // [deg/sec]

#define WAVE1_SUBARU 3500   //A
#define WAVE0_SUBARU 6500   //A
#define TEMP_SUBARU 0       //C
#define PRES_SUBARU 625     //hPa

//#define PA_A0_SUBARU 0.32
//#define PA_A1_SUBARU 0.03
// Corrected to ZERO after precession correction (ver2.5.0)
#define PA_A0_SUBARU 0.00
#define PA_A1_SUBARU 0.00


enum{ AZEL_NORMAL, AZEL_POSI, AZEL_NEGA} AZElMode;

enum{ WWWDB_SIMBAD, 
      WWWDB_NED, 
      WWWDB_DR8, 
      WWWDB_DR14, 
      WWWDB_MAST, 
      WWWDB_MASTP,
      WWWDB_KECK, 
      WWWDB_GEMINI, 
      WWWDB_IRSA, 
      WWWDB_SPITZER, 
      WWWDB_CASSIS, 
      WWWDB_HASH, 
      WWWDB_SEP1, 
      WWWDB_SSLOC, 
      WWWDB_RAPID, 
      WWWDB_MIRSTD, 
      WWWDB_SEP2, 
      WWWDB_SMOKA, 
      WWWDB_HST, 
      WWWDB_ESO} WWWDBMode;

enum{ STDDB_SSLOC, 
      STDDB_RAPID, 
      STDDB_MIRSTD, 
      STDDB_ESOSTD, 
      STDDB_IRAFSTD, 
      STDDB_CALSPEC, 
      STDDB_HDSSTD} STDDBMode;

#define STD_DRA 20
#define STD_DDEC 10
#define STD_VSINI 100
#define STD_VMAG 8
#define STD_SPTYPE "A0"
#define STD_IRAS12 5
#define STD_IRAS25 10
#define STD_CAT "FS"
#define STD_MAG1 5
#define STD_MAG2 15
#define STD_BAND "Jmag"
#define STD_SPTYPE_ALL "%20"
#define STD_SPTYPE_O   "%26(sptype>=O0%26sptype<=O9.9)"
#define STD_SPTYPE_B   "%26(sptype>=B0%26sptype<=B9.9)"
#define STD_SPTYPE_A   "%26(sptype>=A0%26sptype<=A9.9)"
#define STD_SPTYPE_F   "%26(sptype>=F0%26sptype<=F9.9)"
#define STD_SPTYPE_G   "%26(sptype>=G0%26sptype<=G9.9)"
#define STD_SPTYPE_K   "%26(sptype>=K0%26sptype<=K9.9)"
#define STD_SPTYPE_M   "%26(sptype>=M0%26sptype<=M11.9)"

#define OPE_EXTENSION "ope"
#define PRM_EXTENSION "prm"
#define LIST1_EXTENSION "list"
#define LIST2_EXTENSION "lst"
#define LIST3_EXTENSION "txt"
#define CSV_EXTENSION "csv"
#define NST1_EXTENSION "dat"
#define NST2_EXTENSION "tsc"
#define NST3_EXTENSION "eph"
#define PDF_EXTENSION "pdf"
#define HSKYMON_EXTENSION "hsk"


// SKYMON Mode
enum{ SKYMON_CUR, SKYMON_SET, SKYMON_LAST} SkymonMode;

#define SUNSET_OFFSET 25
#define SUNRISE_OFFSET 25

#define SKYMON_DEF_OBJSZ 10.0

// SIZE　OF GUI ENTRY
#define SMALL_ENTRY_SIZE 24
#define LARGE_ENTRY_SIZE 28


// SOSs
#define SOSS_HOSTNAME "ows1.sum.subaru.nao.ac.jp"
#define SOSS_PATH "Procedure"
#define COMMON_DIR "COMMON"


// OBSERVATORY
enum{
OBS_SUBARU, 
OBS_PALOMAR,  
OBS_LICK,  
OBS_KPNO, 
OBS_MMT,  
OBS_LBT,  
OBS_APACHE,  
OBS_HET,  
OBS_CTIO, 
OBS_GEMINIS, 
OBS_LASILLA,  
OBS_MAGELLAN,  
OBS_PARANAL,  
OBS_GTC,  
OBS_CAO,  
OBS_SALT,  
OBS_LAMOST,
OBS_KANATA,
OBS_OAO,
OBS_NHAO,
OBS_KISO,
OBS_GAO,
OBS_AAT
} ObsPos;

#define OBS_SUBARU_NAME "MaunaKea: Subaru Telescope, NAOJ"
#define OBS_SUBARU_LONGITUDE -155.4760278 //[deg] 155 28 33.7
#define OBS_SUBARU_LATITUDE 19.8255    //[deg] 19 49 31.8
#define OBS_SUBARU_ALTITUDE 4163    //[m]
#define OBS_SUBARU_TIMEZONE -600
#define OBS_SUBARU_TZNAME "HST"

#define OBS_PALOMAR_NAME "USA/CA: Palomar Observatory"
#define OBS_PALOMAR_LONGITUDE -116.864944
#define OBS_PALOMAR_LATITUDE 33.356278
#define OBS_PALOMAR_ALTITUDE 1706
#define OBS_PALOMAR_TIMEZONE -480
#define OBS_PALOMAR_TZNAME "PST"

#define OBS_LICK_NAME "USA/CA: Lick Observatory"
#define OBS_LICK_LONGITUDE -121.637256
#define OBS_LICK_LATITUDE 37.343022
#define OBS_LICK_ALTITUDE 1290
#define OBS_LICK_TIMEZONE -480
#define OBS_LICK_TZNAME "PST"

#define OBS_KPNO_NAME "USA/AZ: Kitt Peak National Observatory"
#define OBS_KPNO_LONGITUDE -111.599997 //[deg] 111 36.0
#define OBS_KPNO_LATITUDE 31.964133    //[deg] 31 57.8
#define OBS_KPNO_ALTITUDE 2120    //[m]
#define OBS_KPNO_TIMEZONE -420
#define OBS_KPNO_TZNAME "MST"

#define OBS_MMT_NAME "USA/AZ: Mt. Hopkins (MMT)"
#define OBS_MMT_LONGITUDE -110.885156
#define OBS_MMT_LATITUDE 31.688889
#define OBS_MMT_ALTITUDE 2606    //[m]
#define OBS_MMT_TIMEZONE -420
#define OBS_MMT_TZNAME "MST"

#define OBS_LBT_NAME "USA/AZ: Mt. Graham (LBT)"
#define OBS_LBT_LONGITUDE -109.88906
#define OBS_LBT_LATITUDE 32.70131
#define OBS_LBT_ALTITUDE 3221    //[m]
#define OBS_LBT_TIMEZONE -420
#define OBS_LBT_TZNAME "MST"

#define OBS_APACHE_NAME "USA/NM: Apache Point Observatory (SDSS)"
#define OBS_APACHE_LONGITUDE -105.82
#define OBS_APACHE_LATITUDE 32.78
#define OBS_APACHE_ALTITUDE 2798    //[m]
#define OBS_APACHE_TIMEZONE -420
#define OBS_APACHE_TZNAME "MST"

#define OBS_HET_NAME "USA/TX: McDonald Observatory (HET)"
#define OBS_HET_LONGITUDE -104.01472
#define OBS_HET_LATITUDE 30.68144
#define OBS_HET_ALTITUDE 2026
#define OBS_HET_TIMEZONE -360
#define OBS_HET_TZNAME "CST"

#define OBS_CTIO_NAME "Chile: Cerro Tololo Interamerican Observatory"
#define OBS_CTIO_LONGITUDE -70.806525
#define OBS_CTIO_LATITUDE -30.169661
#define OBS_CTIO_ALTITUDE 2241
#define OBS_CTIO_TIMEZONE -240
#define OBS_CTIO_TZNAME "PRT"

#define OBS_GEMINIS_NAME "Chile: Cerro Pachon (Gemini South)"
#define OBS_GEMINIS_LONGITUDE -70.736683
#define OBS_GEMINIS_LATITUDE -30.240742
#define OBS_GEMINIS_ALTITUDE 2750
#define OBS_GEMINIS_TIMEZONE -240
#define OBS_GEMINIS_TZNAME "PRT"

#define OBS_LASILLA_NAME "Chile: La Silla (NTT)"
#define OBS_LASILLA_LONGITUDE -70.7317
#define OBS_LASILLA_LATITUDE -29.261211
#define OBS_LASILLA_ALTITUDE 2375
#define OBS_LASILLA_TIMEZONE -240
#define OBS_LASILLA_TZNAME "PRT"

#define OBS_MAGELLAN_NAME "Chile: Las Campanus (Magellan)"
#define OBS_MAGELLAN_LONGITUDE -70.69239
#define OBS_MAGELLAN_LATITUDE -29.01418
#define OBS_MAGELLAN_ALTITUDE 2282
#define OBS_MAGELLAN_TIMEZONE -240
#define OBS_MAGELLAN_TZNAME "PRT"

#define OBS_PARANAL_NAME "Chile: Cerro Paranal (VLT)"
#define OBS_PARANAL_LONGITUDE -70.404267
#define OBS_PARANAL_LATITUDE -24.627328
#define OBS_PARANAL_ALTITUDE 2635
#define OBS_PARANAL_TIMEZONE -240
#define OBS_PARANAL_TZNAME "PRT"

#define OBS_GTC_NAME "Canary: La Palma (GTC)"
#define OBS_GTC_LONGITUDE -17.8917
#define OBS_GTC_LATITUDE 28.7564
#define OBS_GTC_ALTITUDE 2267
#define OBS_GTC_TIMEZONE 0
#define OBS_GTC_TZNAME "GMT"

#define OBS_CAO_NAME "Spain: Calar Alto Observatory"
#define OBS_CAO_LONGITUDE -2.54625
#define OBS_CAO_LATITUDE 37.2236
#define OBS_CAO_ALTITUDE 2168
#define OBS_CAO_TIMEZONE 60
#define OBS_CAO_TZNAME "ECT"

#define OBS_SALT_NAME "South Africa: SAAO (SALT)"
#define OBS_SALT_LONGITUDE 20.8107
#define OBS_SALT_LATITUDE -32.3760
#define OBS_SALT_ALTITUDE 1798
#define OBS_SALT_TIMEZONE 120
#define OBS_SALT_TZNAME "EET"

#define OBS_LAMOST_NAME "China: Xinglong (LAMOST)"
#define OBS_LAMOST_LONGITUDE 117.489433
#define OBS_LAMOST_LATITUDE 40.389094
#define OBS_LAMOST_ALTITUDE 656
#define OBS_LAMOST_TIMEZONE 480
#define OBS_LAMOST_TZNAME "CST"

#define OBS_KANATA_NAME "Japan: Higashi-Hiroshima (Kanata)"
#define OBS_KANATA_LONGITUDE 132.7767
#define OBS_KANATA_LATITUDE 34.3775
#define OBS_KANATA_ALTITUDE 511
#define OBS_KANATA_TIMEZONE 540
#define OBS_KANATA_TZNAME "JST"

#define OBS_OAO_NAME "Japan: Okayama Astrophysical Observatory"
#define OBS_OAO_LONGITUDE 133.5940
#define OBS_OAO_LATITUDE 34.5771
#define OBS_OAO_ALTITUDE 370
#define OBS_OAO_TIMEZONE 540
#define OBS_OAO_TZNAME "JST"

#define OBS_NHAO_NAME "Japan: Nishi-Harima (Nayuta)"
#define OBS_NHAO_LONGITUDE 134.33556
#define OBS_NHAO_LATITUDE 35.025272
#define OBS_NHAO_ALTITUDE 418
#define OBS_NHAO_TIMEZONE 540
#define OBS_NHAO_TZNAME "JST"

#define OBS_KISO_NAME "Japan: Kiso Observatory (Univ. of Tokyo)"
#define OBS_KISO_LONGITUDE 137.625352
#define OBS_KISO_LATITUDE 35.797290
#define OBS_KISO_ALTITUDE 1130
#define OBS_KISO_TIMEZONE 540
#define OBS_KISO_TZNAME "JST"

#define OBS_GAO_NAME "Japan: Gunma Astronomical Observatory"
#define OBS_GAO_LONGITUDE 138.972917
#define OBS_GAO_LATITUDE 36.596806
#define OBS_GAO_ALTITUDE 885
#define OBS_GAO_TIMEZONE 540
#define OBS_GAO_TZNAME "JST"

#define OBS_AAT_NAME "Australia: Anglo-Australian Observatory"
#define OBS_AAT_LONGITUDE 149.067222
#define OBS_AAT_LATITUDE -31.275558
#define OBS_AAT_ALTITUDE 1164
#define OBS_AAT_TIMEZONE 600
#define OBS_AAT_TZNAME "AEST"


// All-Sky Camera
enum{ 
ALLSKY_UH, 
ALLSKY_ASIVAV, 
ALLSKY_ASIVAR, 
ALLSKY_MKVIS,
ALLSKY_PALOMAR,
ALLSKY_LICK,
ALLSKY_KPNO,
ALLSKY_MMT,
ALLSKY_HET,
ALLSKY_CTIO,
ALLSKY_LASILLA,
ALLSKY_GTC,
ALLSKY_KANATA,
ALLSKY_OAO,
ALLSKY_NHAO,
ALLSKY_GAO,
ALLSKY_AAT
} AllSkyCamera;

#define ALLSKY_DEF_SHORT "All-Sky Camera"

#define ALLSKY_UH_NAME "MaunaKea: UH 2.2m All-Sky Camera"
#define ALLSKY_UH_SHORT "UH88 All-Sky Camera"
#define ALLSKY_UH_HOST "kree.ifa.hawaii.edu"
#define ALLSKY_UH_PATH "/allsky/allsky_last_eq.png"
#define ALLSKY_UH_FILE "allsky.png"
#define ALLSKY_UH_LAST_FILE "allsky-%ld.png"
//#define ALLSKY_ANGLE 34.5
#define ALLSKY_UH_ANGLE 30.5
#define ALLSKY_UH_DIAMETER 580
#define ALLSKY_UH_CENTERX 326
#define ALLSKY_UH_CENTERY 235

#define ALLSKY_ASIVAV_NAME "MaunaKea: CFHT ASIVA (Visible)"
#define ALLSKY_ASIVAV_SHORT "ASIVA [Visible]"
#define ALLSKY_ASIVAV_HOST "www.cfht.hawaii.edu"
#define ALLSKY_ASIVAV_PATH "/~asiva/images/mask_rot/current_vis.png"
#define ALLSKY_ASIVAV_FILE "allsky.png"
#define ALLSKY_ASIVAV_LAST_FILE "allsky-%ld.png"
#define ALLSKY_ASIVAV_ANGLE (-0.5)
#define ALLSKY_ASIVAV_DIAMETER 570
#define ALLSKY_ASIVAV_CENTERX 394
#define ALLSKY_ASIVAV_CENTERY 282


#define ALLSKY_ASIVAR_NAME "MaunaKea: CFHT ASIVA (Mid-IR)"
#define ALLSKY_ASIVAR_SHORT "ASIVA [Mid-IR]"
#define ALLSKY_ASIVAR_HOST "www.cfht.hawaii.edu"
#define ALLSKY_ASIVAR_PATH "/~asiva/images/mask_rot/raw_a.jpg"
#define ALLSKY_ASIVAR_FILE "allsky.jpg"
#define ALLSKY_ASIVAR_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_ASIVAR_ANGLE 14.0
#define ALLSKY_ASIVAR_DIAMETER 550
#define ALLSKY_ASIVAR_CENTERX 333
#define ALLSKY_ASIVAR_CENTERY 253

#define ALLSKY_MKVIS_NAME "MaunaKea: Hale Pohaku (Visible)"
#define ALLSKY_MKVIS_SHORT "Hale Pohaku [Visible]"
#define ALLSKY_MKVIS_HOST "www.ifa.hawaii.edu"
#define ALLSKY_MKVIS_PATH "/info/vis/uploads/webcams/allsky/AllSkyCurrentImage.JPG"
#define ALLSKY_MKVIS_FILE "allsky.jpg"
#define ALLSKY_MKVIS_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_MKVIS_ANGLE (-63.0)
#define ALLSKY_MKVIS_DIAMETER 610
#define ALLSKY_MKVIS_CENTERX 313
#define ALLSKY_MKVIS_CENTERY 243

// Palomar 640x480
#define ALLSKY_PALOMAR_NAME "USA/CA: Palomar Observatory (Visual)"
#define ALLSKY_PALOMAR_SHORT "Palomar [Visual]"
#define ALLSKY_PALOMAR_HOST "bianca.palomar.caltech.edu"
#define ALLSKY_PALOMAR_PATH "/images/allsky/AllSkyCurrentImage.JPG"
#define ALLSKY_PALOMAR_FILE "allsky.jpg"
#define ALLSKY_PALOMAR_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_PALOMAR_ANGLE (-10.0)
#define ALLSKY_PALOMAR_DIAMETER 610
#define ALLSKY_PALOMAR_CENTERX 352
#define ALLSKY_PALOMAR_CENTERY 248

// Lick 765x521
#define ALLSKY_LICK_NAME "USA/CA: Lick Observatory / LICK (Visual)"
#define ALLSKY_LICK_SHORT "Lick [Visual]"
#define ALLSKY_LICK_HOST "mthamilton.ucolick.org"
#define ALLSKY_LICK_PATH "/hamcam/skycam/current.jpg"
#define ALLSKY_LICK_FILE "allsky.jpg"
#define ALLSKY_LICK_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_LICK_ANGLE (-17.0)
#define ALLSKY_LICK_DIAMETER 850
#define ALLSKY_LICK_CENTERX 400
#define ALLSKY_LICK_CENTERY 275

#define ALLSKY_KPNO_NAME "USA/AZ: Kitt Peak National Observatory (Visual/Red)"
#define ALLSKY_KPNO_SHORT "KPNO [Visual/Red]"
#define ALLSKY_KPNO_HOST "kpasca-db.tuc.noao.edu"
#define ALLSKY_KPNO_PATH "/latestred.png"
#define ALLSKY_KPNO_FILE "allsky.png"
#define ALLSKY_KPNO_LAST_FILE "allsky-%ld.png"
#define ALLSKY_KPNO_ANGLE (-0.9)
#define ALLSKY_KPNO_DIAMETER 490
#define ALLSKY_KPNO_CENTERX 252
#define ALLSKY_KPNO_CENTERY 247

#define ALLSKY_MMT_NAME "USA/AZ: Mt. Hopkins / MMT (Visual)"
#define ALLSKY_MMT_SHORT "MMT [Visual]"
#define ALLSKY_MMT_HOST "skycam.mmto.arizona.edu"
#define ALLSKY_MMT_PATH "/skycam/latest_image.png"
#define ALLSKY_MMT_FILE "allsky.png"
#define ALLSKY_MMT_LAST_FILE "allsky-%ld.png"
#define ALLSKY_MMT_ANGLE 8.0
#define ALLSKY_MMT_DIAMETER 470
#define ALLSKY_MMT_CENTERX 296
#define ALLSKY_MMT_CENTERY 240

#define ALLSKY_HET_NAME "USA/TX: McDonald Observatory / HET (Visual)"
#define ALLSKY_HET_SHORT "HET [Visual]"
#define ALLSKY_HET_HOST "www.as.utexas.edu"
#define ALLSKY_HET_PATH "/mcdonald/webcams/monet-n-sky.jpg"
#define ALLSKY_HET_FILE "allsky.jpg"
#define ALLSKY_HET_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_HET_ANGLE (-5.0)
#define ALLSKY_HET_DIAMETER 740
#define ALLSKY_HET_CENTERX 332
#define ALLSKY_HET_CENTERY 247

#define ALLSKY_CTIO_NAME "Chile: Cerro Tololo (Visual/Z)"
#define ALLSKY_CTIO_SHORT "CTIO [Z]"
#define ALLSKY_CTIO_HOST "www.ctio.noao.edu"
#define ALLSKY_CTIO_PATH "/noao/sites/default/files/lastpic.png"
#define ALLSKY_CTIO_FILE "allsky.png"
#define ALLSKY_CTIO_LAST_FILE "allsky-%ld.png"
#define ALLSKY_CTIO_ANGLE 10.0
#define ALLSKY_CTIO_DIAMETER 500
#define ALLSKY_CTIO_CENTERX 265
#define ALLSKY_CTIO_CENTERY 265

#define ALLSKY_LASILLA_NAME "Chile: La Silla (Visual)"
#define ALLSKY_LASILLA_SHORT "La Silla [Visual]"
#define ALLSKY_LASILLA_HOST "www.ls.eso.org"
#define ALLSKY_LASILLA_PATH "/lasilla/dimm/lasc/gifs/last.gif"
#define ALLSKY_LASILLA_FILE "allsky.gif"
#define ALLSKY_LASILLA_LAST_FILE "allsky-%ld.gif"
#define ALLSKY_LASILLA_ANGLE 3.0
#define ALLSKY_LASILLA_DIAMETER 510
#define ALLSKY_LASILLA_CENTERX 205
#define ALLSKY_LASILLA_CENTERY 220

#define ALLSKY_GTC_NAME "Canary: La Palma / GTC (Visual)"
#define ALLSKY_GTC_SHORT "La Palma [Visual]"
#define ALLSKY_GTC_HOST "www.gtc.iac.es"
#define ALLSKY_GTC_PATH "/multimedia/netcam/camaraAllSky.jpg"
#define ALLSKY_GTC_FILE "allsky.jpg"
#define ALLSKY_GTC_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_GTC_ANGLE 35.6
#define ALLSKY_GTC_DIAMETER 600
#define ALLSKY_GTC_CENTERX 327
#define ALLSKY_GTC_CENTERY 252

#define ALLSKY_KANATA_NAME "Japan: Higashi-Hiroshima / Kanata (Visual)"
#define ALLSKY_KANATA_SHORT "Kanata [Visual]"
#define ALLSKY_KANATA_HOST "hasc.hiroshima-u.ac.jp"
#define ALLSKY_KANATA_PATH "/environ/current_srk.jpg"
#define ALLSKY_KANATA_FILE "allsky.jpg"
#define ALLSKY_KANATA_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_KANATA_ANGLE 0.0
#define ALLSKY_KANATA_DIAMETER 602
#define ALLSKY_KANATA_CENTERX 304
#define ALLSKY_KANATA_CENTERY 289

#define ALLSKY_OAO_NAME "Japan: Okayama Astrophysical Observatory (Visual)"
#define ALLSKY_OAO_SHORT "OAO [Visual]"
#define ALLSKY_OAO_HOST "www.oao.nao.ac.jp"
#define ALLSKY_OAO_PATH "/weather/skymonitor/optsky.jpg"
#define ALLSKY_OAO_FILE "allsky.jpg"
#define ALLSKY_OAO_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_OAO_ANGLE (-3.5)
#define ALLSKY_OAO_DIAMETER 800
#define ALLSKY_OAO_CENTERX 370
#define ALLSKY_OAO_CENTERY 408

#define ALLSKY_NHAO_NAME "Japan: Nishi-Harima / Nayuta (Visual)"
#define ALLSKY_NHAO_SHORT "Nayuta [Visual]"
#define ALLSKY_NHAO_HOST "www.nhao.jp"
#define ALLSKY_NHAO_PATH "/nhao/live/images/skyview.jpg"
#define ALLSKY_NHAO_FILE "allsky.jpg"
#define ALLSKY_NHAO_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_NHAO_ANGLE (-90.0)
#define ALLSKY_NHAO_DIAMETER 560
#define ALLSKY_NHAO_CENTERX 300
#define ALLSKY_NHAO_CENTERY 240

#define ALLSKY_GAO_NAME "Japan: Gunma Astronomical Observatory (Visual)"
#define ALLSKY_GAO_SHORT "GAO [Visual]"
#define ALLSKY_GAO_HOST "www.astron.pref.gunma.jp"
#define ALLSKY_GAO_PATH "/webcam/allsky.jpg"
#define ALLSKY_GAO_FILE "allsky.jpg"
#define ALLSKY_GAO_LAST_FILE "allsky-%ld.jpg"
#define ALLSKY_GAO_ANGLE 0.0
#define ALLSKY_GAO_DIAMETER 460
#define ALLSKY_GAO_CENTERX 332
#define ALLSKY_GAO_CENTERY 238

#define ALLSKY_AAT_NAME "Australia: Siding Spring Observatory (Visual)"
#define ALLSKY_AAT_SHORT "AAT [Visual]"
#define ALLSKY_AAT_HOST "150.203.153.131"
#define ALLSKY_AAT_PATH "/~dbayliss/allskycam/latest_image.png"
#define ALLSKY_AAT_FILE "allsky.png"
#define ALLSKY_AAT_LAST_FILE "allsky-%ld.png"
#define ALLSKY_AAT_ANGLE (-54.0)
#define ALLSKY_AAT_DIAMETER 750
#define ALLSKY_AAT_CENTERX 415
#define ALLSKY_AAT_CENTERY 330


#define ALLSKY_INTERVAL 120

#ifdef ALLSKY_DEBUG
#define ALLSKY_LAST_MAX 20
#else
#define ALLSKY_LAST_MAX 20
#endif

#ifndef USE_WIN32
#define ALLSKY_REPEAT_MAX 30
#endif

#define ALLSKY_DIFF_BASE 64
#define ALLSKY_DIFF_MAG 12

#define ALLSKY_ALPHA (-20)

#define ALLSKY_LIMIT 800

#define ALLSKY_CLOUD_THRESH 3.0
#define ALLSKY_SE_MIN 20.0
#define ALLSKY_SE_MAX 200.0

#define HSKYMON_HTTP_ERROR_GETHOST  -1
#define HSKYMON_HTTP_ERROR_SOCKET   -2
#define HSKYMON_HTTP_ERROR_CONNECT  -3
#define HSKYMON_HTTP_ERROR_TEMPFILE -4
#ifdef USE_SSL
#define HSKYMON_HTTP_ERROR_SSL -5
#endif

// Plot Mode
enum{ PLOT_EL, PLOT_AZ, PLOT_AD, PLOT_ADPAEL, PLOT_MOONSEP,  PLOT_HDSPA} PlotMode;
enum{ PLOT_ALL_SINGLE, PLOT_ALL_SELECTED,PLOT_ALL_ALL} PlotAll;
enum{ PLOT_OUTPUT_WINDOW, PLOT_OUTPUT_PDF, PLOT_OUTPUT_PRINT} PlotOutput;
enum{ PLOT_CENTER_MIDNIGHT, PLOT_CENTER_CURRENT,PLOT_CENTER_MERIDIAN} PlotCenter;
#define PLOT_INTERVAL 60*1000

typedef struct _EPHpara EPHpara;
struct _EPHpara{
  gdouble jd;
  gdouble ra;
  gdouble dec;
  gdouble equinox;
  gdouble geo_d;
};

typedef struct _NSTpara NSTpara;
struct _NSTpara{
  gchar*  filename;
  gint    i_max;
  EPHpara* eph;
  gint    c_fl;
  gint    s_fl;
};

typedef struct _SSLpara SSLpara;
struct _SSLpara{
  guint mode;
  gint cert_res;
  gboolean cert_skip;
  gboolean nonblock;
  gchar *sub;
  gchar *iss;
};


typedef struct _OBJpara OBJpara;
struct _OBJpara{
  gchar *name;
  gchar *def;
  gdouble ra;
  gdouble dec;
  gdouble equinox;

  gint hash;
  gint i_nst;

  /*
  GtkWidget *w_az;
  GtkWidget *w_ha;
  GtkWidget *w_pa;
  */
  gdouble c_az;
  gdouble c_el;
  gdouble c_elmax;
  gdouble c_ha;
  gdouble c_pa;
  gdouble c_ad;
  gdouble c_rt;
  gdouble c_vaz;
  gdouble c_vpa;
  gdouble c_sep;
  gdouble c_hpa;
  gdouble c_vhpa;

  gdouble s_az;
  gdouble s_el;
  gdouble s_elmax;
  gdouble s_ha;
  gdouble s_pa;
  gdouble s_ad;
  gdouble s_vaz;
  gdouble s_vpa;
  gdouble s_sep;
  gdouble s_hpa;
  gdouble s_vhpa;

  gboolean check_disp;
  gboolean check_sm;
  gboolean check_lock;
  gboolean check_used;
  gboolean check_std;


  gchar *note;

  gint ope;
  gint ope_i;

  gdouble x;
  gdouble y;

  gint type;

  gchar *trdb_str;
  gchar *trdb_mode[MAX_TRDB_BAND];
  gchar *trdb_band[MAX_TRDB_BAND];
  gdouble trdb_exp[MAX_TRDB_BAND];
  gint trdb_shot[MAX_TRDB_BAND];
  gint trdb_band_max;
};

typedef struct _STDpara STDpara;
struct _STDpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble d_ra;
  gdouble d_dec;
  gdouble pmra;
  gdouble pmdec;
  gboolean pm;
  gdouble equinox;
  gchar *sp;
  gdouble sep;
  gdouble rot;
  gdouble u;
  gdouble b;
  gdouble v;
  gdouble r;
  gdouble i;
  gdouble j;
  gdouble h;
  gdouble k;
  /*  IRAS depricated in SIMBAD 2017-04
  gchar *f12;
  gchar *f25;
  gchar *f60;
  gchar *f100;
  gchar *q12;
  gchar *q25;
  gchar *q60;
  gchar *q100;*/
  gdouble c_az;
  gdouble c_el;
  gdouble c_elmax;
  gdouble s_az;
  gdouble s_el;
  gdouble s_elmax;

  gdouble x;
  gdouble y;
};

typedef struct _FCDBpara FCDBpara;
struct _FCDBpara{
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble d_ra;
  gdouble d_dec;
  gdouble pmra;
  gdouble pmdec;
  gboolean pm;
  gdouble equinox;
  gchar *otype;
  gchar *sp;
  gdouble sep;
  gchar *nedmag;
  gdouble nedvel;
  gdouble nedz;
  gdouble plx;
  gdouble u;
  gdouble b;
  gdouble v;
  gdouble r;
  gdouble i;
  gdouble j;
  gdouble h;
  gdouble k;
  gchar *fid;
  gchar *date;
  gchar *mode;
  gchar *type;
  gchar *fil;
  gchar *wv;
  gchar *obs;
  gdouble x;
  gdouble y;
  gint ref;
};


typedef struct _HMSpara my_hms;
struct _HMSpara{
  gint hours;
  gint minutes;
  gdouble seconds;
};

typedef struct _Moonpara typMoon;
struct _Moonpara{
  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  gdouble c_disk;
  gdouble c_phase;
  gdouble c_limb;
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  gdouble s_disk;
  gdouble s_phase;
  gdouble s_limb;
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;
};

typedef struct _Sunpara typSun;
struct _Sunpara{
  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  my_hms c_rise;
  my_hms c_set;
  gboolean c_circum;

  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  my_hms s_rise;
  my_hms s_set;
  gboolean s_circum;
};


typedef struct _Planetpara typPlanet;
struct _Planetpara{
  gchar *name;

  struct ln_hms c_ra;
  struct ln_dms c_dec;
  gdouble c_az;
  gdouble c_el;
  gdouble c_mag;

  struct ln_hms s_ra;
  struct ln_dms s_dec;
  gdouble s_az;
  gdouble s_el;
  gdouble s_mag;
};


typedef struct _typHOE typHOE;
struct _typHOE{
  gchar *temp_dir;
  gchar *home_dir;

#ifdef USE_WIN32
  HANDLE hThread_allsky;
  HANDLE hThread_dss;
  HANDLE hThread_stddb;
  HANDLE hThread_fcdb;
  unsigned int dwThreadID_allsky;
  unsigned int dwThreadID_dss;
  unsigned int dwThreadID_stddb;
  unsigned int dwThreadID_fcdb;
#endif

  gint sz_skymon;
  gint sz_plot;
  gint sz_fc;
  gint sz_adc;

  GtkWidget *w_top;
  GtkWidget *w_box;
  GtkWidget *all_note;
  GtkWidget *scrwin;

  GtkWidget *pbar;
  GtkWidget *pbar2;
  GtkWidget *plabel;

  GtkPrintContext *context;

  gchar *fontname;  
  gchar *fontfamily;  
  gchar *fontname_all;  
  gchar *fontfamily_all;  
  gint  skymon_allsz;

  gint timer;
  gint skymon_timer;
  gint plot_timer;
#ifdef USE_XMLRPC
  gint telstat_timer;
#endif

  gchar *filename_list;
  gchar *filename_ope;
  gchar *filename_prm;
  gchar *filename_pdf;
  gchar *filename_txt;
  gchar *filename_fcdb;
  gchar *filename_trdb;
  gchar *filename_trdb_save;
  gchar *filename_nst;
  gchar *filename_jpl;
  gchar *filename_tscconv;
  gchar *filehead;

#ifdef USE_XMLRPC
  gchar *filename_rope[MAX_ROPE];
  gchar *dirname_rope;
  gint max_rope;
#endif

  gchar *window_title;

  gint i_max;
  
  OBJpara obj[MAX_OBJECT];
  NSTpara nst[MAX_ROPE];
  STDpara std[MAX_STD];
  FCDBpara fcdb[MAX_FCDB];

  gchar *prop_id;
  gchar *prop_pass;

  gint obs_timezone;
  gboolean obs_preset_flag;
  gboolean obs_preset_flag_tmp;
  gint obs_preset;
  gint obs_preset_tmp;
  gdouble obs_longitude;
  gdouble obs_latitude;
  gdouble obs_longitude_tmp;
  gdouble obs_latitude_tmp;
  gdouble obs_long_ss;
  gdouble obs_altitude;
  gchar *obs_tzname;
  GtkWidget *obs_combo_preset;
  GtkWidget *obs_combo_ew, *obs_combo_ns, *obs_entry_tz, *obs_frame_pos; 
  GtkAdjustment *obs_adj_lodd,*obs_adj_lomm,*obs_adj_loss;
  GtkAdjustment *obs_adj_ladd,*obs_adj_lamm,*obs_adj_lass;
  GtkAdjustment *obs_adj_alt, *obs_adj_tz;
  gdouble vel_az;
  gdouble vel_el;
  gdouble pa_a0;
  gdouble pa_a1;
  guint wave1;
  guint wave0;
  guint pres;
  gint  temp;

  gboolean show_def;
  gboolean show_elmax;
  gboolean show_secz;
  gboolean show_ha;
#ifdef USE_XMLRPC
  gboolean show_rt;
#endif
  gboolean show_ad;
  gboolean show_ang;
  gboolean show_hpa;
  gboolean show_moon;
  gboolean show_ra;
  gboolean show_dec;
  gboolean show_equinox;
  gboolean show_note;



  gint lst_hour;
  gint lst_min;
  gdouble lst_sec;

  gint skymon_lst_hour;
  gint skymon_lst_min;
  gdouble skymon_lst_sec;

  gint azel_mode;

  gint wwwdb_mode;
  gint stddb_mode;

  GtkWidget *skymon_main;
  GtkWidget *skymon_dw;
  GtkWidget *skymon_frame_mode;
  GtkWidget *skymon_frame_date;
  GtkWidget *skymon_frame_time;
  GtkWidget *skymon_frame_sz;
  GtkWidget *skymon_button_set;
  GtkWidget *skymon_button_fwd;
  GtkWidget *skymon_button_rev;
  GtkWidget *skymon_button_morn;
  GtkWidget *skymon_button_even;
  GtkAdjustment *skymon_adj_year;
  GtkAdjustment *skymon_adj_month;
  GtkAdjustment *skymon_adj_day;
  GtkAdjustment *skymon_adj_hour;
  GtkAdjustment *skymon_adj_min;
  GtkAdjustment *skymon_adj_objsz;
  gint skymon_mode;
  gint skymon_year,skymon_month,skymon_day,skymon_min,skymon_hour;
  gint skymon_objsz;

  gint plot_mode;
  gint plot_all;
  GtkWidget *plot_main;
  GtkWidget *plot_dw;
  gint plot_i;
  gboolean plot_moon;
  gint plot_center;
  gint plot_center_transit;
  gfloat plot_ihst0;
  gfloat plot_ihst1;
  gint plot_output;


  typMoon moon;
  typSun sun;
  typSun atw06;
  typSun atw12;
  typSun atw18;

  typPlanet mercury;
  typPlanet venus;
  typPlanet mars;
  typPlanet jupiter;
  typPlanet saturn;
  typPlanet uranus;
  typPlanet neptune;
  typPlanet pluto;

  GtkWidget *obj_note;
  GtkWidget *query_note;
  GtkWidget *tree;
  GtkWidget *tree_label;
  gchar *tree_label_text;
  guint tree_focus;
  gboolean tree_editing;
  guint tree_width;
  guint tree_height;
  gint tree_x;
  gint tree_y;
  GtkWidget *tree_search_label;
  gchar *tree_search_text;
  guint tree_search_i;
  guint tree_search_iobj[MAX_OBJECT];
  guint tree_search_imax;

  gchar *www_com;

  gint fc_mode;
  gint fc_mode_RGB[3];
  gint i_RGB;
  gint fc_mode_get;
  gint fc_mode_def;
  gint fc_inst;
  gint fc_output;
  GtkWidget *fc_frame_col;
  GtkWidget *fc_button_flip;
  gint dss_arcmin;
  gint dss_arcmin_ip;
  gint dss_pix;
  gint dss_scale;
  gint dss_scale_RGB[3];
  gboolean dss_invert;

  gint dss_i;
  gchar *dss_host;
  gchar *dss_path;
  gchar *dss_src;
  gchar *dss_tmp;
  gchar *dss_file;
  gint dss_pa;
  GtkAdjustment *fc_adj_dss_pa;
  GtkAdjustment *fc_adj_dss_arcmin;
  gboolean dss_flip;
  gboolean dss_draw_slit;
  gboolean sdss_photo;
  gboolean sdss_spec;
  GtkWidget *fc_main;
  GtkWidget *fc_dw;
  gint fc_mag;
  gint fc_magx;
  gint fc_magy;
  gint fc_magmode;
  gint fc_ptn;
  gint fc_ptx1;
  gint fc_pty1;
  gint fc_ptx2;
  gint fc_pty2;

  gint hsc_dithi;
  gint hsc_dithp;
  gint hsc_dra;
  gint hsc_ddec;
  gint hsc_ndith;
  gint hsc_tdith;
  gint hsc_rdith;
  gint hsc_offra;
  gint hsc_offdec;
  GtkWidget *hsc_label_dith;

  gchar *std_file;
  gchar *std_host;
  gchar *std_path;
  gint  std_i;
  gint  std_i_max;
  GtkWidget *stddb_tree;
  GtkWidget *std_tgt;
  gint stddb_tree_focus;
  GtkWidget *stddb_label;
  GtkWidget *stddb_button;
  gchar *stddb_label_text;
  gboolean stddb_flag;

  gint fcdb_type;
  gint fcdb_type_tmp;
  gboolean fcdb_post;
  gchar *fcdb_file;
  gint fcdb_simbad;
  gchar *fcdb_host;
  gchar *fcdb_path;
  gint fcdb_i;
  gint fcdb_tree_focus;
  gdouble fcdb_d_ra0;
  gdouble fcdb_d_dec0;
  gint  fcdb_i_max;
  gint  fcdb_i_all;
  GtkWidget *fcdb_tree;
  GtkWidget *fcdb_sw;
  GtkWidget *fcdb_label;
  GtkWidget *fcdb_frame;
  GtkWidget *fcdb_button;
  GtkWidget *fcdb_tgt;
  gchar *fcdb_label_text;
  gboolean fcdb_flag;
  gint fcdb_band;
  gint fcdb_mag;
  gint fcdb_otype;
  gint fcdb_ned_diam;
  gint fcdb_ned_otype;
  gboolean fcdb_auto;
  gboolean fcdb_ned_ref;
  gboolean fcdb_gsc_fil;
  gint fcdb_gsc_mag;
  gint fcdb_gsc_diam;
  gboolean fcdb_ps1_fil;
  gint fcdb_ps1_mag;
  gint fcdb_ps1_diam;
  gint fcdb_ps1_mindet;
  gint fcdb_sdss_search;
  gint fcdb_sdss_magmin[NUM_SDSS_BAND];
  gint fcdb_sdss_magmax[NUM_SDSS_BAND];
  gboolean fcdb_sdss_fil[NUM_SDSS_BAND];
  gint fcdb_sdss_diam;
  gint fcdb_usno_mag;
  gint fcdb_usno_diam;
  gboolean fcdb_usno_fil;
  gint fcdb_gaia_mag;
  gint fcdb_gaia_diam;
  gboolean fcdb_gaia_fil;
  gint fcdb_2mass_mag;
  gint fcdb_2mass_diam;
  gboolean fcdb_2mass_fil;
  gint fcdb_wise_mag;
  gint fcdb_wise_diam;
  gboolean fcdb_wise_fil;
  gboolean fcdb_smoka_shot;
  gboolean fcdb_smoka_subaru[NUM_SMOKA_SUBARU];
  gboolean fcdb_smoka_kiso[NUM_SMOKA_KISO];
  gboolean fcdb_smoka_oao[NUM_SMOKA_OAO];
  gboolean fcdb_smoka_mtm[NUM_SMOKA_MTM];
  gboolean fcdb_smoka_kanata[NUM_SMOKA_KANATA];
  gboolean fcdb_hst_image[NUM_HST_IMAGE];
  gboolean fcdb_hst_spec[NUM_HST_SPEC];
  gboolean fcdb_hst_other[NUM_HST_OTHER];
  gboolean fcdb_eso_image[NUM_ESO_IMAGE];
  gboolean fcdb_eso_spec[NUM_ESO_SPEC];
  gboolean fcdb_eso_vlti[NUM_ESO_VLTI];
  gboolean fcdb_eso_pola[NUM_ESO_POLA];
  gboolean fcdb_eso_coro[NUM_ESO_CORO];
  gboolean fcdb_eso_other[NUM_ESO_OTHER];
  gboolean fcdb_eso_sam[NUM_ESO_SAM];
  gint fcdb_gemini_inst;

  GtkWidget *trdb_tree;
  GtkWidget *trdb_label;
  gint trdb_i_max;
  gint trdb_tree_focus;
  gboolean trdb_disp_flag;
  gchar *trdb_label_text;
  GtkWidget *trdb_search_label;
  gchar *trdb_search_text;
  guint trdb_search_i;
  guint trdb_search_iobj[MAX_OBJECT];
  guint trdb_search_imax;
  gint trdb_used;
  gint trdb_arcmin;
  gint trdb_arcmin_used;

  gint trdb_smoka_inst;
  gint trdb_smoka_inst_used;
  gchar *trdb_smoka_date;
  gchar *trdb_smoka_date_used;
  gboolean trdb_smoka_shot;
  gboolean trdb_smoka_shot_used;
  gboolean trdb_smoka_imag;
  gboolean trdb_smoka_imag_used;
  gboolean trdb_smoka_spec;
  gboolean trdb_smoka_spec_used;
  gboolean trdb_smoka_ipol;
  gboolean trdb_smoka_ipol_used;

  gint trdb_hst_mode;
  gint trdb_hst_mode_used;
  gchar *trdb_hst_date;
  gchar *trdb_hst_date_used;
  gint trdb_hst_image;
  gint trdb_hst_image_used;
  gint trdb_hst_spec;
  gint trdb_hst_spec_used;
  gint trdb_hst_other;
  gint trdb_hst_other_used;

  gint trdb_eso_mode;
  gint trdb_eso_mode_used;
  gchar *trdb_eso_stdate;
  gchar *trdb_eso_stdate_used;
  gchar *trdb_eso_eddate;
  gchar *trdb_eso_eddate_used;
  gint trdb_eso_image;
  gint trdb_eso_image_used;
  gint trdb_eso_spec;
  gint trdb_eso_spec_used;
  gint trdb_eso_vlti;
  gint trdb_eso_vlti_used;
  gint trdb_eso_pola;
  gint trdb_eso_pola_used;
  gint trdb_eso_coro;
  gint trdb_eso_coro_used;
  gint trdb_eso_other;
  gint trdb_eso_other_used;
  gint trdb_eso_sam;
  gint trdb_eso_sam_used;

  gint trdb_gemini_inst;
  gint trdb_gemini_inst_used;
  gint trdb_gemini_mode;
  gint trdb_gemini_mode_used;
  gchar *trdb_gemini_date;
  gchar *trdb_gemini_date_used;

  gint addobj_type;
  gchar *addobj_name;
  gchar *addobj_voname;
  gchar *addobj_votype;
  gdouble addobj_ra;
  gdouble addobj_dec;
  GtkWidget *addobj_label;
  GtkWidget *addobj_entry_ra;
  GtkWidget *addobj_entry_dec;

  GtkWidget *adc_main;
  GtkWidget *adc_dw;
  GtkWidget *adc_button_flip; 
  GtkAdjustment *adc_adj_pa;
  gint adc_inst;
  gint adc_pa;
  gboolean adc_flip;
  gdouble adc_slit_width;
  gdouble adc_seeing;
  gdouble adc_size;
  gint adc_timer;

  GdkPixbuf *pixbuf;
  GdkPixbuf *pixbuf2;
  GdkPixmap *pixmap_skymon;
#ifdef USE_XMLRPC
  GdkPixmap *pixmap_skymonbg;
#endif
  gboolean allsky_flag;
  gboolean allsky_diff_flag;
  gchar *allsky_name;
  gchar *allsky_host;
  gchar *allsky_file;
  gchar *allsky_path;
  //  gchar *allsky_date_path;
  gchar *allsky_date;
  gchar *allsky_date_old;
  gint allsky_timer;
  gchar *allsky_last_file0;
  gchar *allsky_last_file00;
  gchar *allsky_last_file[ALLSKY_LAST_MAX+1];
  gchar *allsky_last_date[ALLSKY_LAST_MAX+1];
  gint  allsky_last_i;
  guint allsky_interval;
  guint allsky_last_interval;
  gint  allsky_last_frame;
  gint  allsky_last_repeat;
  gint allsky_last_timer;
  gint  allsky_last_time;
  time_t allsky_last_t[ALLSKY_LAST_MAX+1];
  gint  allsky_check_timer;
  gdouble allsky_angle;
  gdouble allsky_sat;
  gint allsky_alpha;
  gint allsky_diameter;
  gint allsky_centerx;
  gint allsky_centery;
  gint allsky_preset;
  gint allsky_preset_tmp;
  gboolean allsky_preset_flag;
  gboolean allsky_preset_flag_tmp;
  GtkWidget *allsky_combo_preset;
  GtkWidget *allsky_frame_server;
  GtkWidget *allsky_frame_image;
  GtkWidget *allsky_entry_host;
  GtkWidget *allsky_entry_file;
  GtkWidget *allsky_entry_path;
  //GtkWidget *allsky_entry_date_path;
  GtkWidget *allsky_entry_last_file;
  GtkAdjustment *allsky_adj_angle;
  GtkAdjustment *allsky_adj_diameter;
  GtkAdjustment *allsky_adj_centerx;
  GtkAdjustment *allsky_adj_centery;
  GtkWidget *allsky_check_limit;
  GtkWidget *allsky_check_flip;
  gint allsky_diff_base;
  gint allsky_diff_mag;
  guint allsky_diff_dpix;
  gboolean allsky_limit;
  gboolean allsky_flip;

  gboolean allsky_pixbuf_flag;
  gboolean allsky_pixbuf_flag0;
  GdkPixbuf *allsky_last_pixbuf[ALLSKY_LAST_MAX+1];
  GdkPixbuf *allsky_diff_pixbuf[ALLSKY_LAST_MAX+1];
  gdouble allsky_cloud_abs[ALLSKY_LAST_MAX+1];
  gdouble allsky_cloud_se[ALLSKY_LAST_MAX+1];
  gdouble allsky_cloud_se_max;
  gdouble allsky_cloud_area[ALLSKY_LAST_MAX+1];  
  gboolean allsky_cloud_show;
  gboolean allsky_cloud_emp;
  gdouble allsky_cloud_thresh;
  gboolean allsky_diff_zero;

  gboolean noobj_flag;
  gboolean hide_flag;

  gboolean orbit_flag;

#ifdef USE_XMLRPC
  gboolean telstat_flag;

  gchar *ro_ns_host;
  gint  ro_ns_port;
  gboolean  ro_use_default_auth;

  gdouble stat_az;
  gdouble stat_az_cmd;
  gdouble stat_az_check;
  gdouble stat_el;
  gdouble stat_el_cmd;
  gdouble stat_reachtime;
  gboolean stat_fixflag;
  gchar *stat_obcp;

  xmlrpc_env env;

  remoteObjectProxy *ro_proxyP;
  gboolean stat_initflag;

  gboolean auto_check_lock;

  GtkWidget *skymon_button_telstat;
  gboolean telstat_error;
#endif

  FILE *fp_log;
  gchar *filename_log;

  gint ope_max;
  gint add_max;
  gint nst_max;
  GdkColor *col[MAX_ROPE];
  GdkColor *col_edge;
  gint alpha_edge;
  gint size_edge;

  gdouble win_cx;
  gdouble win_cy;
  gdouble win_r;

  gint std_dra;
  gint std_ddec;
  gint std_vsini;
  gint std_vmag;
  gchar *std_sptype;
  gint std_iras12;
  gint std_iras25;
  gchar *std_cat;
  gint std_mag1;
  gint std_mag2;
  gchar *std_band;
  gchar *std_sptype2;
};


// Struct for Callback
typedef struct{
  GdkColor *col;
  gint alpha;
}confCol;

// Struct for Callback
typedef struct{
  struct ln_dms *longitude;
  struct ln_dms *latitude;
  gchar *www_com;
}confPos;

// Struct for Callback
typedef struct{
  typHOE *hg;
  gint i_obj;
}confPA;

// Struct for Callback
typedef struct{
  GtkWidget *dialog;
  gint mode;
}confProp;

// Struct for Callback
typedef struct{
  GtkWidget *dialog;
  gint mode;
  GSList *fcdb_group;
  gint fcdb_type;
}confPropFCDB;


#define is_num_char(c) ((c>='0' && c<='9')||(c==' ')||(c=='\t')||(c=='.')||(c=='+')||(c=='-')||(c=='\n'))




//// Global args.
gboolean  flagProp;
gboolean  flagChildDialog;
gboolean  flagTree;
gboolean  flagPlot;
gboolean  flagFC;
gboolean  flagADC;
gboolean  flag_getting_allsky;
#ifndef USE_WIN32
pid_t allsky_pid;
#endif
pid_t fc_pid;
pid_t fcdb_pid;
pid_t stddb_pid;


//// Functions' proto-type
// main.c
gchar* fgets_new();
void printf_log(typHOE *hg, const gchar *format, ...);
void my_signal_connect();
void cc_get_toggle();
void cc_get_adj();
void cc_get_adj_double();
void cc_get_combo_box();
void cc_get_entry();
void cc_get_entry_double();
void my_entry_set_width_chars();
GtkWidget* gtkut_button_new_from_stock();
GtkWidget* gtkut_toggle_button_new_from_stock();
GtkWidget* gtkut_button_new_from_pixbuf();
GtkWidget* gtkut_toggle_button_new_from_pixbuf();
void get_current_obs_time();
void add_day();
void do_save_FCDB_List();
void do_save_TRDB_CSV();
void do_save_pdf();
void do_save_fc_pdf();
void do_quit();
void do_plot();
void do_update_azel();
void popup_message(gchar*, gint , ...);
void create_fcdb_para_dialog();
gboolean is_separator();
GtkWidget *make_menu();
gint update_allsky();
#ifdef USE_XMLRPC
gint update_telstat();
#endif
void clear_trdb();


//adc.c
void do_adc();
void adc_item2 ();
gboolean draw_adc_cairo();

//calcpa.c
void calcpa2_main();
void calcpa2_skymon();
void pdf_plot();
void create_plot_dialog();
void geocen_to_topocen();
gdouble ra_to_deg();
gdouble dec_to_deg();
gdouble deg_to_ra();
gdouble deg_to_dec();
gdouble date_to_jd();
void calc_moon();
double get_julian_day_of_equinox();
gboolean draw_plot_cairo();
gfloat get_meridian_hour();
gdouble deg_sep();

//fc.c
void pdf_fc ();
void set_fc_mode();
void fc_item2 ();
void fcdb_item2();
void trdb_run();
gboolean draw_fc_cairo();
void fcdb_tree_update_azel_item();
void trdb_tree_update_azel_item();
void ver_dl();
void addobj_dl();
gdouble current_yrs();
gboolean progress_timeout();
void make_trdb_label();

//fc_output.c
void Export_FCDB_List();
void Export_TRDB_List();

//http-client.c
int get_allsky();
GdkPixbuf* diff_pixbuf();
int get_dss();
int get_stddb();
int get_fcdb();
void allsky_debug_print (const gchar *format, ...) G_GNUC_PRINTF(1, 2);
int month_from_string_short();

//julian_day.c
int get_gmtoff_from_sys ();

//skymon.c
void skymon_set_time_current();
void skymon_debug_print(const gchar *format, ...);
gboolean draw_skymon();
void create_skymon_dialog();
gboolean draw_skymon_cairo();
#ifdef USE_XMLRPC
gboolean draw_skymon_with_telstat_cairo();
#endif


//telstat.c
#ifdef USE_XMLRPC
int close_telstat();
int get_telstat();
int get_rope();
#endif

//treeview.c
void make_tree();
void remake_tree();
void rebuild_fcdb_tree();
gint tree_update_azel();
gchar* make_tgt();
gchar *make_simbad_id();
void addobj_dialog();
void raise_tree();
void str_replace();
void fcdb_append_tree();


//utility.c
void ln_deg_to_dms();
double ln_dms_to_deg();
void ln_equ_to_hequ();

//votable.c
void make_band_str();
void fcdb_vo_parse();
void fcdb_ned_vo_parse();
void fcdb_gsc_vo_parse();
void fcdb_ps1_vo_parse();
void fcdb_sdss_vo_parse();
void fcdb_usno_vo_parse();
void fcdb_gaia_vo_parse();
void fcdb_2mass_vo_parse();
void fcdb_wise_vo_parse();
void fcdb_irc_vo_parse();
void fcdb_fis_vo_parse();
void fcdb_lamost_vo_parse();
void fcdb_smoka_txt_parse();
void trdb_smoka_txt_parse();
void fcdb_hst_vo_parse();
void trdb_hst_vo_parse();
void fcdb_eso_vo_parse();
void trdb_eso_vo_parse();
void addobj_vo_parse();
void stddb_vo_parse();

//jason_parse.c
void fcdb_gemini_jason_parse();
void trdb_gemini_jason_parse();
