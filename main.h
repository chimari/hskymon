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
//#undef HTTP_DEBUG

#include<glib.h>
#include<gtk/gtk.h>

#ifdef USE_GTKMACINTEGRATION
#include<gtkmacintegration/gtkosxapplication.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<ctype.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#ifdef HAVE_PWD_H
#include<pwd.h>
#endif
#include<sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include<sys/wait.h>
#endif
#include<errno.h>
#include<math.h>
#include<string.h>

#ifdef USE_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winnt.h>
#endif

#include "gtkut.h"

#ifdef USE_XMLRPC
#include "remoteObjects.h"
#endif

#include "libnova/libnova.h"

#include "gen2.h"

#include "observatory.h"

#include "post.h"
#include "post_sdss.h"
#include "post_lamost.h"
#include "post_kepler.h"
#include "post_smoka.h"
#include "post_hst.h"
#include "post_eso.h"
#include "get_gemini.h"

#include "io_gui.h"

#include "lgs.h"

#include "seimei.h"

#ifdef USE_WIN32
#define USER_CONFFILE "hskymon.ini"
#else
#define USER_CONFFILE ".hskymon"
#endif


#define AU_IN_KM 149597870.700

#define WWW_BROWSER "firefox"

#define DEFAULT_URL "https://www.naoj.org/Observing/tools/hskymon/"
#define VER_HOST "www.naoj.org"
#define VER_PATH "/Observing/tools/hskymon/ver"

#define DEFAULT_PROXY_HOST "proxy.host.address"

#ifdef USE_WIN32
#define DSS_URL "http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey"
#define SIMBAD_URL "http://%s/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML"
#define DR8_URL "http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf"
#define SDSS_DRNOW_URL "http://skyserver.sdss.org/dr16/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf"
#define NED_URL "http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define TRANSIENT_URL "https://www.wis-tns.org/search?&discovered_period_value=&discovered_period_units=years&unclassified_at=0&classified_sne=0&name=&name_like=0&isTNS_AT=all&public=all&ra=%lf&decl=%+lf&radius=120&coords_unit=arcsec&reporting_groupid%%5B%%5D=null&groupid%%5B%%5D=null&classifier_groupid%%5B%%5D=null&objtype%%5B%%5D=null&at_type%%5B%%5D=null&date_start%%5Bdate%%5D=&date_end%%5Bdate%%5D=&discovery_mag_min=&discovery_mag_max=&internal_name=&discoverer=&classifier=&spectra_count=&redshift_min=&redshift_max=&hostname=&ext_catid=&ra_range_min=&ra_range_max=&decl_range_min=&decl_range_max=&discovery_instrument%%5B%%5D=null&classification_instrument%%5B%%5D=null&associated_groups%%5B%%5D=null&at_rep_remarks=&class_rep_remarks=&num_page=50"
#define MAST_URL "http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER"
#define MASTP_URL "https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type"
#define SPITZER_URL "http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true"
#define CASSIS_URL "http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120"
#define RAPID_URL "http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptypes<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define MIRSTD_URL "http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define SSLOC_URL "http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST"
#define STD_SIMBAD_URL "http://%s/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML"
#define FCDB_NED_URL "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
#define FCDB_SDSS_URL "http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_DR5_URL "http://dr5.lamost.org/spectrum/view?obsid=%d"
#define FCDB_LAMOST_DR6_URL "http://dr6.lamost.org/spectrum/view?obsid=%d"
#define FCDB_LAMOST_DR6M_URL "http://dr6.lamost.org/medspectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "https://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d"
#define FCDB_SMOKA_SHOT_URL "https://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all"
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
#define SIMBAD_URL "open http://%s/simbad/sim-coo?CooDefinedFrames=none\\&CooEquinox=2000\\&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf\\&submit=submit%%20query\\&Radius.unit=arcmin\\&CooEqui=2000\\&CooFrame=FK5\\&Radius=2\\&output.format=HTML"
#define DR8_URL "open http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf\\&dec=%s%d:%d:%.2lf"
#define SDSS_DRNOW_URL "open http://skyserver.sdss.org/dr14/en/tools/quicklook/summary.aspx?ra=%lf\\&dec=%s%lf"
#define NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search\\&in_csys=Equatorial\\&in_equinox=J2000.0\\&lon=%d%%3A%d%%3A%.2lf\\&lat=%s%d%%3A%d%%3A%.2lf\\&radius=2.0\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&z_constraint=Unconstrained\\&z_value1=\\&z_value2=\\&z_unit=z\\&ot_include=ANY\\&nmp_op=ANY\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=Distance+to+search+center\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define TRANSIENT_URL "open https://www.wis-tns.org/search?\\&discovered_period_value=\\&discovered_period_units=years\\&unclassified_at=0\\&classified_sne=0\\&name=\\&name_like=0\\&isTNS_AT=all\\&public=all\\&ra=%lf\\&decl=%+lf\\&radius=120\\&coords_unit=arcsec\\&reporting_groupid%%5B%%5D=null\\&groupid%%5B%%5D=null\\&classifier_groupid%%5B%%5D=null\\&objtype%%5B%%5D=null\\&at_type%%5B%%5D=null\\&date_start%%5Bdate%%5D=\\&date_end%%5Bdate%%5D=\\&discovery_mag_min=\\&discovery_mag_max=\\&internal_name=\\&discoverer=\\&classifier=\\&spectra_count=\\&redshift_min=\\&redshift_max=\\&hostname=\\&ext_catid=\\&ra_range_min=\\&ra_range_max=\\&decl_range_min=\\&decl_range_max=\\&discovery_instrument%%5B%%5D=null\\&classification_instrument%%5B%%5D=null\\&associated_groups%%5B%%5D=null\\&at_rep_remarks=\\&class_rep_remarks=\\&num_page=50"
#define MAST_URL "open http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf\\&max_records=10\\&action=Search\\&resolver=SIMBAD\\&missions[]=EUVE\\&missions[]=WFC3-IMAGE\\&missions[]=WFPC1\\&missions[]=WFPC2\\&missions[]=FOC\\&missions[]=ACS-IMAGE\\&missions[]=UIT\\&missions[]=STIS-IMAGE\\&missions[]=COS-IMAGE\\&missions[]=GALEX\\&missions[]=XMM-OM\\&missions[]=NICMOS-IMAGE\\&missions[]=FUSE\\&missions[]=IMAPS\\&missions[]=BEFS\\&missions[]=TUES\\&missions[]=IUE\\&missions[]=COPERNICUS\\&missions[]=HUT\\&missions[]=WUPPE\\&missions[]=GHRS\\&missions[]=STIS-SPECTRUM\\&missions[]=COS-SPECTRUM\\&missions[]=WFC3-SPECTRUM\\&missions[]=ACS-SPECTRUM\\&missions[]=FOS\\&missions[]=HPOL\\&missions[]=NICMOS-SPECTRUM\\&missions[]=FGS\\&missions[]=HSP\\&missions[]=KEPLER"
#define MASTP_URL "open https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf"
#define KECK_URL "open https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single"
#define GEMINI_URL "open https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT"
#define IRSA_URL "open http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All\\&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf\\&mode=cone\\&radius=2\\&radunits=arcmin\\&range=6.25+Deg.\\&data=Data+Set+Type\\&radnum=2222\\&irsa=IRSA+Only\\&submit=Get+Inventory\\&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type\\&url=%%2Fworkspace%%2FTMP_3hX3SO_29666\\&dir=%%2Fwork%%2FTMP_3hX3SO_29666\\&snull=matches+only\\&datav=Data+Set+Type"
#define SPITZER_URL "open http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition\\&DoSearch=true\\&SearchByPosition.field.radius=0.033333333\\&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000\\&SimpleTargetPanel.field.resolvedBy=nedthensimbad\\&MoreOptions.field.prodtype=aor,pbcd\\&startIdx=0\\&pageSize=0\\&shortDesc=Position\\&isBookmarkAble=true\\&isDrillDownRoot=true\\&isSearchResult=true"
#define CASSIS_URL "open http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf\\&dec=%.6lf&radius=120"
#define RAPID_URL "open http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define MIRSTD_URL "open http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define SSLOC_URL "open http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s\\&submit=submit%%20query\\&output.max=%d\\&OutputMode=LIST"
#define STD_SIMBAD_URL "open http://%s/simbad/sim-id?Ident=%s\\&NbIdent=1\\&Radius=2\\&Radius.unit=arcmin\\&submit=submit+id\\&output.format=HTML"
#define FCDB_NED_URL "open http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s\\&extend=no\\&hconst=73\\&omegam=0.27\\&omegav=0.73\\&corr_z=1\\&out_csys=Equatorial\\&out_equinox=J2000.0\\&obj_sort=RA+or+Longitude\\&of=pre_text\\&zv_breaker=30000.0\\&list_limit=5\\&img_stamp=YES"
#define FCDB_SDSS_URL "open http://skyserver.sdss.org/dr16/en/tools/quicklook/summary.aspx?id=%s"
#define FCDB_LAMOST_DR5_URL "open http://dr5.lamost.org/spectrum/view?obsid=%d"
#define FCDB_LAMOST_DR6_URL "open http://dr6.lamost.org/spectrum/view?obsid=%d"
#define FCDB_LAMOST_DR6M_URL "open http://dr6.lamost.org/medspectrum/view?obsid=%d"
#define FCDB_SMOKA_URL "open https://smoka.nao.ac.jp/info.jsp?frameid=%s\\&date_obs=%s\\&i=%d"
#define FCDB_SMOKA_SHOT_URL "open https://smoka.nao.ac.jp/fssearch?frameid=%s*\\&instruments=%s\\&obs_mod=all\\&data_typ=all\\&dispcol=default\\&diff=1000\\&action=Search\\&asciitable=table\\&obs_cat=all"
#define FCDB_HST_URL "open http://archive.stsci.edu/cgi-bin/mastpreview?mission=hst\\&dataid=%s"
#define FCDB_ESO_URL "open http://archive.eso.org/wdb/wdb/eso/eso_archive_main/query?dp_id=%s\\&format=DecimDeg\\&tab_stat_plot=on\\&aladin_colour=aladin_instrument"
#define FCDB_GEMINI_URL "open https://archive.gemini.edu/searchform/cols=CTOWEQ/notengineering/NotFail/OBJECT/%s"
#define TRDB_GEMINI_URL "open https://archive.gemini.edu/searchform/sr=%d/cols=CTOWEQ/notengineering%sra=%.6lf/%s/science%sdec=%s%.6lf/NotFail/OBJECT"
#define HASH_URL "open http://202.189.117.101:8999/gpne/objectInfoPage.php?id=%d"

#else
// in UNIX    
//    - just add a pair of \" at the beginning and the end of each phrase.
#define DSS_URL "\"http://skyview.gsfc.nasa.gov/current/cgi/runquery.pl?Interface=quick&Position=%d+%d+%.2lf%%2C+%s%d+%d+%.2lf&SURVEY=Digitized+Sky+Survey\""
#define SIMBAD_URL "\"http://%s/simbad/sim-coo?CooDefinedFrames=none&CooEquinox=2000&Coord=%d%%20%d%%20%.2lf%%20%s%d%%20%d%%20%.2lf&submit=submit%%20query&Radius.unit=arcmin&CooEqui=2000&CooFrame=FK5&Radius=2&output.format=HTML\""
#define DR8_URL "\"http://skyserver.sdss3.org/dr8/en/tools/quicklook/quickobj.asp?ra=%d:%d:%.2lf&dec=%s%d:%d:%.2lf\""
#define SDSS_DRNOW_URL "\"http://skyserver.sdss.org/dr16/en/tools/quicklook/summary.aspx?ra=%lf&dec=%s%lf\""
#define NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=2.0&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define TRANSIENT_URL "\"https://www.wis-tns.org/search?&discovered_period_value=&discovered_period_units=years&unclassified_at=0&classified_sne=0&name=&name_like=0&isTNS_AT=all&public=all&ra=%lf&decl=%+lf&radius=120&coords_unit=arcsec&reporting_groupid%%5B%%5D=null&groupid%%5B%%5D=null&classifier_groupid%%5B%%5D=null&objtype%%5B%%5D=null&at_type%%5B%%5D=null&date_start%%5Bdate%%5D=&date_end%%5Bdate%%5D=&discovery_mag_min=&discovery_mag_max=&internal_name=&discoverer=&classifier=&spectra_count=&redshift_min=&redshift_max=&hostname=&ext_catid=&ra_range_min=&ra_range_max=&decl_range_min=&decl_range_max=&discovery_instrument%%5B%%5D=null&classification_instrument%%5B%%5D=null&associated_groups%%5B%%5D=null&at_rep_remarks=&class_rep_remarks=&num_page=50\""
#define MAST_URL "\"http://archive.stsci.edu/xcorr.php?target=%.5lf%s%.10lf&max_records=10&action=Search&resolver=SIMBAD&missions[]=EUVE&missions[]=WFC3-IMAGE&missions[]=WFPC1&missions[]=WFPC2&missions[]=FOC&missions[]=ACS-IMAGE&missions[]=UIT&missions[]=STIS-IMAGE&missions[]=COS-IMAGE&missions[]=GALEX&missions[]=XMM-OM&missions[]=NICMOS-IMAGE&missions[]=FUSE&missions[]=IMAPS&missions[]=BEFS&missions[]=TUES&missions[]=IUE&missions[]=COPERNICUS&missions[]=HUT&missions[]=WUPPE&missions[]=GHRS&missions[]=STIS-SPECTRUM&missions[]=COS-SPECTRUM&missions[]=WFC3-SPECTRUM&missions[]=ACS-SPECTRUM&missions[]=FOS&missions[]=HPOL&missions[]=NICMOS-SPECTRUM&missions[]=FGS&missions[]=HSP&missions[]=KEPLER\""
#define MASTP_URL "\"https://mast.stsci.edu/portal/Mashup/Clients/Mast/Portal.html?searchQuery=%lf%%20%s%lf\""
#define KECK_URL "\"https://koa.ipac.caltech.edu/cgi-bin/bgServices/nph-bgExec?bgApp=/KOA/nph-KOA&instrument_de=deimos&instrument_es=esi&instrument_hi=hires&instrument_lr=lris&instrument_lw=lws&instrument_mf=mosfire&instrument_n1=nirc&instrument_n2=nirc2&instrument_ns=nirspec&instrument_os=osiris&filetype=science&calibassoc=assoc&locstr=%.6lf+%+.6lf&regSize=120&resolver=ned&radunits=arcsec&spt_obj=spatial&single_multiple=single\""
#define GEMINI_URL "\"https://archive.gemini.edu/searchform/sr=120/cols=CTOWEQ/notengineering/ra=%.6lf/dec=%+.6lf/NotFail/OBJECT\""
#define IRSA_URL "\"http://irsa.ipac.caltech.edu/cgi-bin/Radar/nph-estimation?mission=All&objstr=%d%%3A%d%%3A%.2lf+%s%d%%3A%d%%3A%.2lf&mode=cone&radius=2&radunits=arcmin&range=6.25+Deg.&data=Data+Set+Type&radnum=2222&irsa=IRSA+Only&submit=Get+Inventory&output=%%2Firsa%%2Fcm%%2Fops_2.0%%2Firsa%%2Fshare%%2Fwsvc%%2FRadar%%2Fcatlist.tbl_type&url=%%2Fworkspace%%2FTMP_3hX3SO_29666&dir=%%2Fwork%%2FTMP_3hX3SO_29666&snull=matches+only&datav=Data+Set+Type\""
#define SPITZER_URL "\"http://sha.ipac.caltech.edu/applications/Spitzer/SHA/#id=SearchByPosition&DoSearch=true&SearchByPosition.field.radius=0.033333333&UserTargetWorldPt=%.5lf;%.10lf;EQ_J2000&SimpleTargetPanel.field.resolvedBy=nedthensimbad&MoreOptions.field.prodtype=aor,pbcd&startIdx=0&pageSize=0&shortDesc=Position&isBookmarkAble=true&isDrillDownRoot=true&isSearchResult=true\""
#define CASSIS_URL "\"http://cassis.sirtf.com/atlas/cgi/radec.py?ra=%.5lf&dec=%.6lf&radius=120\""
#define RAPID_URL "\"http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define MIRSTD_URL "\"http://%s/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define SSLOC_URL "\"http://%s/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST\""
#define STD_SIMBAD_URL "\"http://%s/simbad/sim-id?Ident=%s&NbIdent=1&Radius=2&Radius.unit=arcmin&submit=submit+id&output.format=HTML\""
#define FCDB_NED_URL "\"http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES\""
#define FCDB_SDSS_URL "\"http://skyserver.sdss.org/dr16/en/tools/quicklook/summary.aspx?id=%s\""
#define FCDB_LAMOST_DR5_URL "\"http://dr5.lamost.org/spectrum/view?obsid=%d\""
#define FCDB_LAMOST_DR6_URL "\"http://dr6.lamost.org/spectrum/view?obsid=%d\""
#define FCDB_LAMOST_DR6M_URL "\"http://dr6.lamost.org/medspectrum/view?obsid=%d\""
#define FCDB_SMOKA_URL "\"https://smoka.nao.ac.jp/info.jsp?frameid=%s&date_obs=%s&i=%d\""
#define FCDB_SMOKA_SHOT_URL "\"https://smoka.nao.ac.jp/fssearch?frameid=%s*&instruments=%s&obs_mod=all&data_typ=all&dispcol=default&diff=1000&action=Search&asciitable=table&obs_cat=all\""
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
#define FCDB_PS1_MIN_NDET 10
#define FCDB_PS1_MAX_DIAM 30
enum
{
  FCDB_PS1_OLD,
  FCDB_PS1_DR_1,
  FCDB_PS1_DR_2,
  FCDB_PS1_DR_NUM
};

enum
{
  FCDB_PS1_MODE_MEAN,
  FCDB_PS1_MODE_STACK,
  FCDB_PS1_MODE_NUM
};

#define STDDB_PATH_SSLOC "/simbad/sim-sam?Criteria=cat=%s%%26%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26%%28%s>%d%%26%s<%d%%29%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_RAPID "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26rot.vsini>%d%%26Vmag<%d%%26sptype<%s&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_PATH_MIRSTD "/simbad/sim-sam?Criteria=%%28ra>%.2lf%sra<%.2lf%%29%%26dec>%.2lf%%26dec<%.2lf%%26iras.f12>%d%%26iras.f25>%d&submit=submit%%20query&output.max=%d&OutputMode=LIST&output.format=VOTABLE"
#define STDDB_FILE_XML "simbad.xml"

#define FCDB_HOST_SIMBAD_STRASBG "simbad.u-strasbg.fr"
#define FCDB_HOST_SIMBAD_HARVARD "simbad.harvard.edu"
#define FCDB_SIMBAD_PATH_B "/simbad/sim-sam?Criteria=region%%28box%%2C%lf%s%lf%%2C%+lfm%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_SIMBAD_PATH_R "/simbad/sim-sam?Criteria=region%%28circle%%2C%lf%s%lf%%2C%+lfm%%29%s%s&submit=submit+query&OutputMode=LIST&maxObject=%d&CriteriaFile=&output.format=VOTABLE"
#define FCDB_FILE_XML "database_fc.xml"
#define FCDB_FILE_TXT "database_fc.txt"
#define FCDB_FILE_HTML "database_fc.html"
#define FCDB_FILE_JSON "database_fc.json"

#define FCDB_HOST_NED "ned.ipac.caltech.edu"
#define FCDB_NED_PATH "/cgi-bin/nph-objsearch?search_type=Near+Position+Search&in_csys=Equatorial&in_equinox=J2000.0&lon=%d%%3A%d%%3A%.2lf&lat=%s%d%%3A%d%%3A%.2lf&radius=%.2lf&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&z_constraint=Unconstrained&z_value1=&z_value2=&z_unit=z&ot_include=ANY&nmp_op=ANY%sout_csys=Equatorial&out_equinox=J2000.0&obj_sort=Distance+to+search+center&of=pre_text&zv_breaker=30000.0&list_limit=0&img_stamp=YES&of=xml_main"

#define FCDB_HOST_GSC "gsss.stsci.edu"
#define FCDB_GSC_PATH "/webservices/vo/ConeSearch.aspx?RA=%lf&DEC=%+lf&SR=%lf%sMAX_OBJ=5000&FORMAT=VOTABLE&CAT=GSC241"

#define FCDB_HOST_PS1OLD "gsss.stsci.edu"
#define FCDB_PS1OLD_PATH  "/webservices/vo/CatalogSearch.aspx?CAT=PS1V3OBJECTS&RA=%lf&DEC=%+lf&SR=%lf&MINDET=%d%sMAXOBJ=5000"
#define FCDB_HOST_PS1 "catalogs.mast.stsci.edu"
#define FCDB_PS1_PATH  "/api/v0.1/panstarrs/%s/%s?ra=%lf&dec=%+lf&radius=%lf&nDetections.gte=%d%spagesize=5000&format=votable"

#define FCDB_HOST_SDSS "skyserver.sdss.org"
#define FCDB_SDSS_PATH "/dr16/en/tools/search/x_results.aspx"

#define FCDB_HOST_VIZIER_STRASBG "vizier.u-strasbg.fr"
#define FCDB_HOST_VIZIER_NAOJ "vizier.nao.ac.jp"
#define FCDB_HOST_VIZIER_HARVARD "vizier.cfa.harvard.edu"

#define FCDB_HOST_USNO "vizier.u-strasbg.fr"
#define FCDB_USNO_PATH_B "/viz-bin/votable?-source=USNO-B1&-c=%lf%%20%+lf&-c.u=arcsec&-c.bs=%dx%d&-c.geom=b&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_UCAC "vizier.u-strasbg.fr"
#define FCDB_UCAC_PATH_B "/viz-bin/votable?-source=UCAC4&-c=%lf%%20%+lf&-c.u=arcsec&-c.bs=%dx%d&-c.geom=b&-out.max=5000%s-out.form=VOTable&-out=UCAC4&-out=RAJ2000&-out=DEJ2000&-out=ePos&-out=f.mag&-out=of&-out=db&-out=pmRA&-out=pmDE&-out=Jmag&-out=Hmag&-out=Kmag&-out=Bmag&-out=Vmag&-out=gmag&-out=rmag&-out=imag"
#define FCDB_UCAC_PATH_R "/viz-bin/votable?-source=UCAC4&-c=%lf%%20%+lf&-c.u=arcsec&-c.r=%d&-c.geom=r&-out.max=5000%s-out.form=VOTable&-out=UCAC4&-out=RAJ2000&-out=DEJ2000&-out=ePos&-out=f.mag&-out=of&-out=db&-out=pmRA&-out=pmDE&-out=Jmag&-out=Hmag&-out=Kmag&-out=Bmag&-out=Vmag&-out=gmag&-out=rmag&-out=imag"

#define FCDB_HOST_GAIA "vizier.u-strasbg.fr"
#define FCDB_GAIA_PATH_R "/viz-bin/votable?-source=I/355/gaiadr3&-c=%lf%%20%+lf&-c.u=arcsec&-c.r=%d&-c.geom=r&-out.max=5000%s-out.form=VOTable"
#define FCDB_GAIA_PATH_B "/viz-bin/votable?-source=I/355/gaiadr3&-c=%lf%%20%+lf&-c.u=arcsec&-c.bs=%dx%d&-c.geom=b&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_2MASS "gsss.stsci.edu"
#define FCDB_2MASS_PATH "/webservices/vo/CatalogSearch.aspx?CAT=2MASS&RA=%lf&DEC=%+lf&SR=%lf%sMAXOBJ=5000"

#define FCDB_HOST_WISE "vizier.u-strasbg.fr"
#define FCDB_WISE_PATH_B "/viz-bin/votable?-source=II/311/wise&-c=%lf%%20%+lf&-c.u=arcsec&-c.bs=%dx%d&-c.geom=b&-out.max=5000%s-out.form=VOTable"

#define FCDB_HOST_IRC "vizier.u-strasbg.fr"
#define FCDB_IRC_PATH_B "/viz-bin/votable?-source=II/297/irc&-c=%lf%%20%+lf&-c.u=arcsec&-c.bs=%dx%d&-c.geom=b&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_FIS "vizier.u-strasbg.fr"
#define FCDB_FIS_PATH_B "/viz-bin/votable?-source=II/298/fis&-c=%lf%%20%+lf&-c.u=deg&-c.bs=%dx%d&-c.geom=b&-out.max=5000&-out.form=VOTable"

#define FCDB_HOST_LAMOST_DR5 "dr5.lamost.org"
#define FCDB_HOST_LAMOST_DR6 "dr6.lamost.org"
#define FCDB_LAMOST_PATH "/q"
#define FCDB_LAMOST_MED_PATH "/medcas/q"

#define FCDB_HOST_KEPLER "archive.stsci.edu"
#define FCDB_KEPLER_PATH "/kepler/kic10/search.php"

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

#define ADDOBJ_TRANSIENT_HOST "www.wis-tns.org"
#define ADDOBJ_TRANSIENT_PATH "/search?name=%s&discovered_period_value=100&discovered_period_units=years&format=tsv"

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
#define FC_PATH_SDSS13 "/dr16/SkyServerWS/ImgCutout/getjpeg?TaskName=Skyserver.Chart.image&ra=%lf&dec=%lf&scale=%f&width=%d&height=%d&opt=%s%s&query=%s%s"
#define FC_HOST_PANCOL "ps1images.stsci.edu"
#define FC_PATH_PANCOL "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=color&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANG "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=g&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANR "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=r&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANI "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=i&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANZ "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=z&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="
#define FC_PATH_PANY "/cgi-bin/ps1cutouts?pos=%lf+%+lf&filter=y&filetypes=stack&auxiliary=data&size=%d&output_size=1024&verbose=0&autoscale=99.500000&catlist="


static const gchar* cal_month[]={"Jan",
				 "Feb",
				 "Mar",
				 "Apr",
				 "May",
				 "Jun",
				 "Jul",
				 "Aug",
				 "Sep",
				 "Oct",
				 "Nov",
				 "Dec"};

#ifdef USE_GTK3
static GdkRGBA color_comment = {0.87, 0.00, 0.00, 1};
static GdkRGBA color_focus =   {0.53, 0.27, 0.00, 1};
static GdkRGBA color_calib =   {0.00, 0.53, 0.00, 1};
static GdkRGBA color_black =   {0.00, 0.00, 0.00, 1};
static GdkRGBA color_red   =   {1.00, 0.00, 0.00, 1};
static GdkRGBA color_blue =    {0.00, 0.00, 1.00, 1};
static GdkRGBA color_white =   {1.00, 1.00, 1.00, 1};
static GdkRGBA color_gray1 =   {0.40, 0.40, 0.40, 1};
static GdkRGBA color_gray2 =   {0.80, 0.80, 0.80, 1};
static GdkRGBA color_pink =    {1.00, 0.40, 0.40, 1};
static GdkRGBA color_pink2 =   {1.00, 0.80, 0.80, 1};
static GdkRGBA color_pale =    {0.40, 0.40, 1.00, 1};
static GdkRGBA color_pale2 =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_orange =  {1.00, 0.80, 0.40, 1};
static GdkRGBA color_orange2 = {1.00, 1.00, 0.80, 1};
static GdkRGBA color_green  =  {0.40, 0.80, 0.80, 1};
static GdkRGBA color_green2 =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_purple2 = {1.00, 0.80, 1.00, 1};
static GdkRGBA color_com1 =    {0.00, 0.53, 0.00, 1};
static GdkRGBA color_com2 =    {0.73, 0.53, 0.00, 1};
static GdkRGBA color_com3 =    {0.87, 0.00, 0.00, 1};
#else
static GdkColor color_comment = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_focus = {0, 0x8888, 0x4444, 0x0000};
static GdkColor color_calib = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_black = {0, 0, 0, 0};
static GdkColor color_red   = {0, 0xFFFF, 0, 0};
static GdkColor color_blue = {0, 0, 0, 0xFFFF};
static GdkColor color_white = {0, 0xFFFF, 0xFFFF, 0xFFFF};
static GdkColor color_gray1 = {0, 0x6666, 0x6666, 0x6666};
static GdkColor color_gray2 = {0, 0xBBBB, 0xBBBB, 0xBBBB};
static GdkColor color_pink = {0, 0xFFFF, 0x6666, 0x6666};
static GdkColor color_pink2 = {0, 0xFFFF, 0xCCCC, 0xCCCC};
static GdkColor color_pale = {0, 0x6666, 0x6666, 0xFFFF};
static GdkColor color_pale2 = {0, 0xCCCC, 0xCCCC, 0xFFFF};
static GdkColor color_orange = {0, 0xFFFF, 0xCCCC, 0x6666};
static GdkColor color_orange2 = {0, 0xFFFF, 0xFFFF, 0xCCCC};
static GdkColor color_green = {0, 0x6666, 0xCCCC, 0x6666};
static GdkColor color_green2 = {0, 0xCCCC, 0xFFFF, 0xCCCC};
static GdkColor color_purple2 = {0, 0xFFFF, 0xCCCC, 0xFFFF};
static GdkColor color_com1 = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_com2 = {0, 0xBBBB, 0x8888, 0x0000};
static GdkColor color_com3 = {0, 0xDDDD, 0x0000, 0x0000};
#endif


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
     FC_PANY,
     NUM_FC};

static const gchar* FC_name[]={
  "STScI: DSS1 (Red)",         // FC_STSCI_DSS1R, 
  "STScI: DSS1 (Blue)",        // FC_STSCI_DSS1B, 
  "STScI: DSS2 (Red)",         // FC_STSCI_DSS2R,
  "STScI: DSS2 (Blue)",        // FC_STSCI_DSS2B,
  "STScI: DSS2 (IR)",          // FC_STSCI_DSS2IR,
  NULL,                        // FC_SEP1,
  "ESO: DSS1 (Red)",           // FC_ESO_DSS1R,
  "ESO: DSS2 (Red)",           // FC_ESO_DSS2R,
  "ESO: DSS2 (Blue)",          // FC_ESO_DSS2B,
  "ESO: DSS2 (IR)",            // FC_ESO_DSS2IR,
  NULL,                        // FC_SEP2,
  "SkyView: GALEX (Far UV)",   // FC_SKYVIEW_GALEXF,
  "SkyView: GALEX (Near UV)",  // FC_SKYVIEW_GALEXN,
  "SkyView: DSS1 (Red)",       // FC_SKYVIEW_DSS1R,
  "SkyView: DSS1 (Blue)",      // FC_SKYVIEW_DSS1B,
  "SkyView: DSS2 (Red)",       // FC_SKYVIEW_DSS2R,
  "SkyView: DSS2 (Blue)",      // FC_SKYVIEW_DSS2B,
  "SkyView: DSS2 (IR)",        // FC_SKYVIEW_DSS2IR,
  "SkyView: SDSS (u)",         // FC_SKYVIEW_SDSSU,
  "SkyView: SDSS (g)",         // FC_SKYVIEW_SDSSG,
  "SkyView: SDSS (r)",         // FC_SKYVIEW_SDSSR,
  "SkyView: SDSS (i)",         // FC_SKYVIEW_SDSSI,
  "SkyView: SDSS (z)",         // FC_SKYVIEW_SDSSZ,
  "SkyView: 2MASS (J)",        // FC_SKYVIEW_2MASSJ,
  "SkyView: 2MASS (H)",        // FC_SKYVIEW_2MASSH,
  "SkyView: 2MASS (K)",        // FC_SKYVIEW_2MASSK,
  "SkyView: WISE (3.4um)",     // FC_SKYVIEW_WISE34,
  "SkyView: WISE (4.6um)",     // FC_SKYVIEW_WISE46,
  "SkyView: WISE (12um)",      // FC_SKYVIEW_WISE12,
  "SkyView: WISE (22um)",      // FC_SKYVIEW_WISE22,
  "SkyView: AKARI N60",        // FC_SKYVIEW_AKARIN60,
  "SkyView: AKARI WIDE-S",     // FC_SKYVIEW_AKARIWS,
  "SkyView: AKARI WIDE-L",     // FC_SKYVIEW_AKARIWL,
  "SkyView: AKARI N160",       // FC_SKYVIEW_AKARIN160,
  "SkyView: NVSS (1.4GHz)",    // FC_SKYVIEW_NVSS,
  "SkyView: RGB composite",    // FC_SKYVIEW_RGB,
  NULL,                        // FC_SEP3,
  "SDSS DR7 (color)",          // FC_SDSS,
  "SDSS DR16 (color)",         // FC_SDSS13,
  NULL,                        // FC_SEP4,
  "PanSTARRS-1 (color)",       // FC_PANCOL,
  "PanSTARRS-1 (g)",           // FC_PANG,
  "PanSTARRS-1 (r)",           // FC_PANR,
  "PanSTARRS-1 (i)",           // FC_PANI,
  "PanSTARRS-1 (z)",           // FC_PANZ,
  "PanSTARRS-1 (y)"};           // FC_PANY

static const gchar* FC_markup[]={
  "STScI: DSS1 (<span color=\"#FF7F7F\">Red</span>)",         // FC_STSCI_DSS1R, 
  "STScI: DSS1 (<span color=\"#7F7FFF\">Blue</span>)",        // FC_STSCI_DSS1B, 
  "STScI: DSS2 (<span color=\"#FF7F7F\">Red</span>)",         // FC_STSCI_DSS2R,
  "STScI: DSS2 (<span color=\"#7F7FFF\">Blue</span>)",        // FC_STSCI_DSS2B,
  "STScI: DSS2 (IR)",          // FC_STSCI_DSS2IR,
  NULL,                        // FC_SEP1,
  "ESO: DSS1 (<span color=\"#FF7F7F\">Red</span>)",           // FC_ESO_DSS1R,
  "ESO: DSS2 (<span color=\"#FF7F7F\">Red</span>)",           // FC_ESO_DSS2R,
  "ESO: DSS2 (<span color=\"#7F7FFF\">Blue</span>)",          // FC_ESO_DSS2B,
  "ESO: DSS2 (IR)",            // FC_ESO_DSS2IR,
  NULL,                        // FC_SEP2,
  "SkyView: GALEX (Far UV)",   // FC_SKYVIEW_GALEXF,
  "SkyView: GALEX (Near UV)",  // FC_SKYVIEW_GALEXN,
  "SkyView: DSS1 (<span color=\"#FF7F7F\">Red</span>)",       // FC_SKYVIEW_DSS1R,
  "SkyView: DSS1 (<span color=\"#7F7FFF\">Blue</span>)",      // FC_SKYVIEW_DSS1B,
  "SkyView: DSS2 (<span color=\"#FF7F7F\">Red</span>)",       // FC_SKYVIEW_DSS2R,
  "SkyView: DSS2 (<span color=\"#7F7FFF\">Blue</span>)",      // FC_SKYVIEW_DSS2B,
  "SkyView: DSS2 (IR)",        // FC_SKYVIEW_DSS2IR,
  "SkyView: SDSS (u)",         // FC_SKYVIEW_SDSSU,
  "SkyView: SDSS (g)",         // FC_SKYVIEW_SDSSG,
  "SkyView: SDSS (r)",         // FC_SKYVIEW_SDSSR,
  "SkyView: SDSS (i)",         // FC_SKYVIEW_SDSSI,
  "SkyView: SDSS (z)",         // FC_SKYVIEW_SDSSZ,
  "SkyView: 2MASS (J)",        // FC_SKYVIEW_2MASSJ,
  "SkyView: 2MASS (H)",        // FC_SKYVIEW_2MASSH,
  "SkyView: 2MASS (K)",        // FC_SKYVIEW_2MASSK,
  "SkyView: WISE (3.4&#xB5;m)",     // FC_SKYVIEW_WISE34,
  "SkyView: WISE (4.6&#xB5;m)",     // FC_SKYVIEW_WISE46,
  "SkyView: WISE (12&#xB5;m)",      // FC_SKYVIEW_WISE12,
  "SkyView: WISE (22&#xB5;m)",      // FC_SKYVIEW_WISE22,
  "SkyView: AKARI N60",        // FC_SKYVIEW_AKARIN60,
  "SkyView: AKARI WIDE-S",     // FC_SKYVIEW_AKARIWS,
  "SkyView: AKARI WIDE-L",     // FC_SKYVIEW_AKARIWL,
  "SkyView: AKARI N160",       // FC_SKYVIEW_AKARIN160,
  "SkyView: NVSS (1.4GHz)",    // FC_SKYVIEW_NVSS,
  "SkyView: <span color=\"#FF7F7F\">R</span><span color=\"#7FFF7F\">G</span><span color=\"#7F7FFF\">B</span> composite",    // FC_SKYVIEW_RGB,
  NULL,                        // FC_SEP3,
  "SDSS DR7 (color)",          // FC_SDSS,
  "SDSS DR16 (color)",         // FC_SDSS13,
  NULL,                        // FC_SEP4,
  "PanSTARRS-1 (color)",       // FC_PANCOL,
  "PanSTARRS-1 (g)",           // FC_PANG,
  "PanSTARRS-1 (r)",           // FC_PANR,
  "PanSTARRS-1 (i)",           // FC_PANI,
  "PanSTARRS-1 (z)",           // FC_PANZ,
  "PanSTARRS-1 (y)"};          // FC_PANY

static const gchar* FC_img[]={
  "DSS (POSS1 Red)",           // FC_STSCI_DSS1R, 
  "DSS (POSS1 Blue)",          // FC_STSCI_DSS1B, 
  "DSS (POSS2 Red)",           // FC_STSCI_DSS2R,
  "DSS (POSS2 Blue)",          // FC_STSCI_DSS2B,
  "DSS (POSS2 IR)",            // FC_STSCI_DSS2IR,
  NULL,                        // FC_SEP1,
  "DSS (POSS1 Red)",           // FC_ESO_DSS1R,
  "DSS (POSS2 Red)",           // FC_ESO_DSS2R,
  "DSS (POSS2 Blue)",          // FC_ESO_DSS2B,
  "DSS (POSS2 IR)",            // FC_ESO_DSS2IR,
  NULL,                        // FC_SEP2,
  "GALEX (Far UV)",            // FC_SKYVIEW_GALEXF,
  "GALEX (Near UV)",           // FC_SKYVIEW_GALEXN,
  "DSS (POSS1 Red)",           // FC_SKYVIEW_DSS1R,
  "DSS (POSS1 Blue)",          // FC_SKYVIEW_DSS1B,
  "DSS (POSS2 Red)",           // FC_SKYVIEW_DSS2R,
  "DSS (POSS2 Blue)",          // FC_SKYVIEW_DSS2B,
  "DSS (POSS2 IR)",            // FC_SKYVIEW_DSS2IR,
  "SDSS (u-band)",             // FC_SKYVIEW_SDSSU,
  "SDSS (g-band)",             // FC_SKYVIEW_SDSSG,
  "SDSS (r-band)",             // FC_SKYVIEW_SDSSR,
  "SDSS (i-band)",             // FC_SKYVIEW_SDSSI,
  "SDSS (z-band)",             // FC_SKYVIEW_SDSSZ,
  "2MASS (J-band)",            // FC_SKYVIEW_2MASSJ,
  "2MASS (H-band)",            // FC_SKYVIEW_2MASSH,
  "2MASS (K-band)",            // FC_SKYVIEW_2MASSK,
  "WISE (3.4um)",              // FC_SKYVIEW_WISE34,
  "WISE (4.6um)",              // FC_SKYVIEW_WISE46,
  "WISE (12um)",               // FC_SKYVIEW_WISE12,
  "WISE (22um)",               // FC_SKYVIEW_WISE22,
  "AKARI N60",                 // FC_SKYVIEW_AKARIN60,
  "AKARI WIDE-S",              // FC_SKYVIEW_AKARIWS,
  "AKARI WIDE-L",              // FC_SKYVIEW_AKARIWL,
  "AKARI N160",                // FC_SKYVIEW_AKARIN160,
  "NVSS (1.4GHz)",             // FC_SKYVIEW_NVSS,
  "RGB composite",             // FC_SKYVIEW_RGB,
  NULL,                        // FC_SEP3,
  "SDSS (DR7/color)",          // FC_SDSS,
  "SDSS (DR16/color)",         // FC_SDSS13,
  NULL,                        // FC_SEP4,
  "PanSTARRS-1 (color)",       // FC_PANCOL,
  "PanSTARRS-1 (g-band)",      // FC_PANG,
  "PanSTARRS-1 (r-band)",      // FC_PANR,
  "PanSTARRS-1 (i-band)",      // FC_PANI,
  "PanSTARRS-1 (z-band)",      // FC_PANZ,
  "PanSTARRS-1 (y-band)"};      // FC_PANY

static const gchar* FC_host[]={
  FC_HOST_STSCI,         // FC_STSCI_DSS1R, 
  FC_HOST_STSCI,         // FC_STSCI_DSS1B, 
  FC_HOST_STSCI,         // FC_STSCI_DSS2R,
  FC_HOST_STSCI,         // FC_STSCI_DSS2B,
  FC_HOST_STSCI,         // FC_STSCI_DSS2IR,
  NULL,                  // FC_SEP1,
  FC_HOST_ESO,           // FC_ESO_DSS1R,
  FC_HOST_ESO,           // FC_ESO_DSS2R,
  FC_HOST_ESO,           // FC_ESO_DSS2B,
  FC_HOST_ESO,           // FC_ESO_DSS2IR,
  NULL,                  // FC_SEP2,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_GALEXF,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_GALEXN,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS1R,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS1B,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2R,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2B,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_DSS2IR,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSU,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSG,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSR,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSI,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_SDSSZ,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSJ,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSH,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_2MASSK,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE34,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE46,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE12,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_WISE22,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIN60,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIWS,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIWL,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_AKARIN160,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_NVSS,
  FC_HOST_SKYVIEW,       // FC_SKYVIEW_RGB,
  NULL,                  // FC_SEP3,
  FC_HOST_SDSS,          // FC_SDSS,
  FC_HOST_SDSS,          // FC_SDSS13,
  NULL,                  // FC_SEP4,
  FC_HOST_PANCOL,        // FC_PANCOL,
  FC_HOST_PANCOL,        // FC_PANG,
  FC_HOST_PANCOL,        // FC_PANR,
  FC_HOST_PANCOL,        // FC_PANI,
  FC_HOST_PANCOL,        // FC_PANZ,
  FC_HOST_PANCOL};        // FC_PANY

#define PANSTARRS_MAX_ARCMIN 25

#define FC_WINSIZE 400
enum{ FC_OUTPUT_WINDOW, FC_OUTPUT_PDF, FC_OUTPUT_PRINT};
enum{ FC_INST_NONE,
      FC_INST_HDS,
      FC_INST_HDSAUTO,
      FC_INST_HDSZENITH,
      FC_INST_IRD,
      FC_INST_IRCS,
      FC_INST_COMICS,
      FC_INST_FOCAS,
      FC_INST_MOIRCS,
      FC_INST_SWIMS,
      FC_INST_FMOS,
      FC_INST_SPCAM,
      FC_INST_HSCDET,
      FC_INST_HSCA,
      FC_INST_PFS,
      FC_INST_SEP1,
      FC_INST_KOOLS,
      FC_INST_TRICCS,
      FC_INST_NO_SELECT,
      NUM_FC_INST};

static const gchar* FC_instname[]={
  "None",        //FC_INST_NONE,	  
  "HDS",         //FC_INST_HDS,	  
  "HDS (w/o ImR)", //FC_INST_HDSAUTO,	  
  "HDS (zenith)",  //FC_INST_HDSZENITH,  
  "IRD",         //FC_INST_IRD,	  
  "IRCS",        //FC_INST_IRCS,	  
  "COMICS",      //FC_INST_COMICS,	  
  "FOCAS",       //FC_INST_FOCAS,	  
  "MOIRCS",      //FC_INST_MOIRCS,	  
  "SWIMS",       //FC_INST_SWIMS,	  
  "FMOS",        //FC_INST_FMOS,	  
  "Suprime-Cam", //FC_INST_SPCAM,	  
  "HSC (Det-ID)",//FC_INST_HSCDET,	  
  "HSC (HSCA)",  //FC_INST_HSCA,	  
  "PFS",         //FC_INST_PFS,	  
  NULL,          //FC_INST_SEP1,	  
  "Seimei : KOOLS-IFU",//FC_INST_KOOLS,	  
  "Seimei : TriCCS",//FC_INST_TRICCS,
  NULL};


enum{ FC_SCALE_LINEAR, FC_SCALE_LOG, FC_SCALE_SQRT, FC_SCALE_HISTEQ, FC_SCALE_LOGLOG};

enum{ FCDB_SIMBAD_STRASBG, FCDB_SIMBAD_HARVARD };
enum{ FCDB_VIZIER_STRASBG, FCDB_VIZIER_NAOJ, 
      FCDB_VIZIER_HARVARD };

#define ADC_WINSIZE 400
#define ADC_SLIT_WIDTH 0.4
#define ADC_SIZE 5.0
#define ADC_SEEING 0.6
enum{ ADC_INST_IMR,
      ADC_INST_HDSAUTO,
      ADC_INST_HDSZENITH,
      ADC_INST_KOOLS,
      NUM_ADC_INST};

//  Instrument
#define HDS_SLIT_MASK_ARCSEC 9.2
// micron
#define HDS_SLIT_LENGTH 10000
#define HDS_SLIT_WIDTH 500
#define HDS_PA_OFFSET (-58.4)
//#define GAOES_PA_OFFSET (-90.0)
#define HDS_SIZE 3

#define FMOS_SIZE 40
#define FMOS_R_ARCMIN 30

#define SPCAM_X_ARCMIN 34
#define SPCAM_Y_ARCMIN 27
#define SPCAM_GAP_ARCSEC 14.
#define SPCAM_SIZE 40

#define HSC_R_ARCMIN 90
enum{ HSC_DITH_NO, HSC_DITH_5, HSC_DITH_N};
#define HSC_DRA 120
#define HSC_DDEC 120
#define HSC_TDITH 15
#define HSC_RDITH 120

#define PFS_R_ARCMIN 82.8

#define FOCAS_R_ARCMIN 6
#define FOCAS_GAP_ARCSEC 5.
#define FOCAS_SIZE 10

#define IRD_SIZE 3
#define IRD_X_ARCSEC 20.
#define IRD_Y_ARCSEC 10.
#define IRD_TTGS_ARCMIN 2

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

#define SWIMS_X_ARCMIN 6.6
#define SWIMS_Y_ARCMIN 3.3
#define SWIMS_R_ARCMIN 7.2
#define SWIMS_GAP_ARCSEC 0.95
#define SWIMS_SIZE 10

#define HSC_SIZE 110

#define KOOLS_SIZE 2
#define KOOLS_X_ARCSEC 7.7
#define KOOLS_Y_ARCSEC 8.1
#define ZWOCAM_X_ARCSEC 105.
#define ZWOCAM_Y_ARCSEC 70.
#define ZWOCAM_RETICLE1_ARCSEC 4.
#define ZWOCAM_RETICLE2_ARCSEC 20.
#define ZWOCAM_RETICLE3_ARCSEC 40.

#define TRICCS_SIZE 15
#define TRICCS_X_ARCMIN 12.6
#define TRICCS_Y_ARCMIN 7.5


// Object Type
enum{
  OBJTYPE_OBJ,
  OBJTYPE_STD,
  OBJTYPE_TTGS
}; 

#define ADDTYPE_OBJ   -1
#define ADDTYPE_STD   -2
#define ADDTYPE_TTGS  -3

// OpenSSL
enum {
  SSL_NONE,
  SSL_TUNNEL,
  SSL_STARTTLS
};

enum {
  SSL_CERT_NONE,
  SSL_CERT_ACCEPT,
  SSL_CERT_DENY
};


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
  COLUMN_OBJ_RA_COL,
  COLUMN_OBJ_DEC,
  COLUMN_OBJ_DEC_COL,
  COLUMN_OBJ_EQUINOX,
  COLUMN_OBJ_PAM,
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
  COLUMN_FCDB_EPLX,
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
  FCDB_TYPE_UCAC,
  FCDB_TYPE_GAIA,
  FCDB_TYPE_KEPLER,
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
  TRDB_TYPE_FCDB_GEMINI,
  ADDOBJ_TYPE_TRANSIENT
};

static const gchar* db_name[]={  
  "SIMBAD",         //FCDB_TYPE_SIMBAD,
  "NED",            //FCDB_TYPE_NED,
  "GSC 2.4.1",      //FCDB_TYPE_GSC,
  "PanSTARRS1",     //FCDB_TYPE_PS1,
  "SDSS DR16",      //FCDB_TYPE_SDSS,
  "LAMOST",         //FCDB_TYPE_LAMOST,
  "USNO",           //FCDB_TYPE_USNO,
  "UCAC4",          //FCDB_TYPE_UCAC,
  "GAIA DR3",       //FCDB_TYPE_GAIA, 
  "Kepler",         //FCDB_TYPE_KEPLER,
  "2MASS",          //FCDB_TYPE_2MASS,
  "WISE",           //FCDB_TYPE_WISE,
  "Akari/IRC",      //FCDB_TYPE_IRC,
  "Akari/FIS",      //FCDB_TYPE_FIS,
  "SMOKA",          //FCDB_TYPE_SMOKA,
  "HST archive",    //FCDB_TYPE_HST,
  "ESO archive",    //FCDB_TYPE_ESO,
  "Gemini archive", //FCDB_TYPE_GEMINI,
  "SMOKA",          //FCDB_TYPE_WWWDB_SMOKA,
  "HST archive",    //FCDB_TYPE_WWWDB_HST,
  "ESO archive",    //FCDB_TYPE_WWWDB_ESO,
  "SMOKA",          //TRDB_TYPE_SMOKA,
  "HST archive",    //TRDB_TYPE_HST,
  "ESO archive",    //TRDB_TYPE_ESO,
  "Gemini archive", //TRDB_TYPE_GEMINI,
  // Until here for FCDB on Finding Chart
  "SMOKA",          //TRDB_TYPE_WWWDB_SMOKA,
  "HST archive",    //TRDB_TYPE_WWWDB_HST,
  "ESO archive",    //TRDB_TYPE_WWWDB_ESO,
  "SMOKA",          //TRDB_TYPE_FCDB_SMOKA,
  "HST archive",    //TRDB_TYPE_FCDB_HST,
  "ESO archive",    //TRDB_TYPE_FCDB_ESO,
  "Gemini archive", //TRDB_TYPE_FCDB_GEMINI,
};


enum
{
  FCDB_LAMOST_DR5,
  FCDB_LAMOST_DR6,
  FCDB_LAMOST_DR6M,
  NUM_FCDB_LAMOST
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

static char* simbad_band[NUM_FCDB_BAND]=
  {"(Nop.)", "U", "B", "V", "R", "I", "J", "H", "K"};

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
#define SKYMON_FONT "Sans 10"
#endif

#define DEF_SIZE_EDGE 4

#define PLOT_WINSIZE 400

#define PLOT_WIDTH_MM 160
#define PLOT_HEIGHT_MM 160

// popup message
#define GTK_MSG
// time out for error popup window [sec]
#define POPUP_TIMEOUT 2


//#define VERSION "0.8.0"
#define ALLSKY_MONITOR_INTERVAL 1000
#define AZEL_INTERVAL 60*1000
#define TELSTAT_INTERVAL 3*1000
#define SKYCHECK_INTERVAL 500


#define SKYMON_INTERVAL 200
#define SKYMON_STEP 5

#define MAX_OBJECT 5000
#define MAX_ROPE 32
#define MAX_STD 100
#define MAX_FCDB 5000
#define MAX_TRDB_BAND 100

#ifdef USE_XMLRPC
enum{ ROPE_DIR, ROPE_ALL};
#endif

#define BUFFSIZE 65535

#define VEL_AZ_SUBARU 0.5  // [deg/sec]
#define VEL_EL_SUBARU 0.5  // [deg/sec]

#define WAVE1_SUBARU 3500   //A
#define WAVE0_SUBARU 6500   //A
#define TEMP_SUBARU 0       //C
#define PRES_SUBARU 625     //hPa

#define TEMP_SEIMEI 15       //C
#define PRES_SEIMEI 1015     //hPa

//#define PA_A0_SUBARU 0.32
//#define PA_A1_SUBARU 0.03
// Corrected to ZERO after precession correction (ver2.5.0)
#define PA_A0_SUBARU 0.00
#define PA_A1_SUBARU 0.00


enum{ AZEL_NORMAL, AZEL_POSI, AZEL_NEGA};

enum{ WWWDB_SIMBAD, 
      WWWDB_NED, 
      WWWDB_TRANSIENT, 
      WWWDB_DR8, 
      WWWDB_SDSS_DRNOW, 
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
      WWWDB_ESO};

enum{ STDDB_SSLOC, 
      STDDB_RAPID, 
      STDDB_MIRSTD, 
      STDDB_ESOSTD, 
      STDDB_IRAFSTD, 
      STDDB_CALSPEC, 
      STDDB_HDSSTD};

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
#define LIST4_EXTENSION "dat"
#define CSV_EXTENSION "csv"
#define NST1_EXTENSION "dat"
#define NST2_EXTENSION "tsc"
#define NST3_EXTENSION "eph"
#define PDF_EXTENSION "pdf"
#define HSKYMON_EXTENSION "hsk"


// SKYMON Mode
enum{ SKYMON_CUR, SKYMON_SET, SKYMON_LAST};

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



// All-Sky Camera
enum{ 
  ALLSKY_UH, 
  ALLSKY_ASIVAV, 
  ALLSKY_ASIVAR, 
  ALLSKY_SUBARU, 
  ALLSKY_MKVIS,
  ALLSKY_PALOMAR,
  ALLSKY_LICK,
  ALLSKY_KPNO,
  ALLSKY_MMT,
  ALLSKY_HET,
  ALLSKY_CPAC,
  ALLSKY_LASILLA,
  ALLSKY_GTC,
  ALLSKY_KANATA,
  ALLSKY_OAO,
  ALLSKY_NHAO,
  ALLSKY_GAO,
  ALLSKY_AAT,
  ALLSKY_MSO,
  NUM_ALLSKY
};


#define ALLSKY_DEF_SHORT "All-Sky Camera"

typedef struct _AllSkypara AllSkypara;
struct _AllSkypara{
  gchar *name;
  gchar *sname;
  gchar *host;
  gchar *path;
  gchar *file;
  gchar *lfile;
  gdouble angle;
  gint diam;
  gint x;
  gint y;
  gboolean limit;
  gboolean flip;
  gboolean ssl;
};


static const AllSkypara allsky_param[]={
  // ALLSKY_UH
  {"MaunaKea: UH 2.2m All-Sky Camera",
   "UH88 All-Sky Camera",
   "kree.ifa.hawaii.edu",
   "/allsky/allsky_last_eq.png",
   "allsky.png",
   "allsky-%ld.png",
   30.5,
   580,
   326,
   235,
   FALSE,
   FALSE,
   FALSE},
  
  // ALLSKY_ASIVAV
  {"MaunaKea: CFHT ASIVA (Visible)",
   "ASIVA [Visible]",
   "www.cfht.hawaii.edu",
   "/~asiva/images/mask_rot/current_vis.png",
   "allsky.png",
   "allsky-%ld.png",
   -0.5,
   570,
   394,
   282,
   TRUE,
   FALSE,
   TRUE},

  // ALLSKY_ASIVAR
  {"MaunaKea: CFHT ASIVA (Mid-IR)",
   "ASIVA [Mid-IR]",
   "www.cfht.hawaii.edu",
   "/~asiva/images/mask_rot/raw_a.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   14.0,
   550,
   333,
   253,
   FALSE,
   FALSE,
   TRUE},
  
  // ALLSKY_SUBARU
  {"Subaru Telescope (Visible)",
   "Subaru",
   "allsky.sum.subaru.nao.ac.jp",
   "/ftp/images/ImageLastFTP_AllSKY.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   158.0,
   3700,
   2760,
   1850,
   TRUE,
   TRUE,
   FALSE},
  
  //ALLSKY_MKVIS
  {"MaunaKea: Hale Pohaku (Visible)",
   "Hale Pohaku [Visible]",
   "www.ifa.hawaii.edu",
   "/info/vis/uploads/webcams/allsky/AllSkyCurrentImage.JPG",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -63.0,
   610,
   313,
   243,
   FALSE,
   FALSE,
   FALSE},
  
  // Palomar 640x480
  // ALLSKY_PALOMAR
  {"USA/CA: Palomar Observatory (Visual)",
   "Palomar [Visual]",
   "bianca.palomar.caltech.edu",
   "/images/allsky/AllSkyCurrentImage.JPG",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -10.0,
   610,
   352,
   248,
   FALSE,
   FALSE,
   FALSE},

  // Lick 765x521
  // ALLSKY_LICK
  {"USA/CA: Lick Observatory / LICK (Visual)",
   "Lick [Visual]",
   "mthamilton.ucolick.org",
   "/hamcam/skycam/current.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -17.0,
   850,
   400,
   275,
   TRUE,
   FALSE,
   FALSE},
   
  // ALLSKY_KPNO
  {"USA/AZ: Kitt Peak National Observatory (Visual/Red)",
   "KPNO [Visual/Red]",
   "kpasca-db.tuc.noao.edu",
   "/latestred.png",
   "allsky.png",
   "allsky-%ld.png",
   -0.9,
   490,
   252,
   247,
   FALSE,
   FALSE,
   FALSE},
  
  // ALLSKY_MMT
  {"USA/AZ: Mt. Hopkins / MMT (Visual)",
   "MMT [Visual]",
   "skycam.mmto.arizona.edu",
   "/skycam/latest_image.png",
   "allsky.png",
   "allsky-%ld.png",
   8.0,
   470,
   296,
   240,
   FALSE,
   FALSE,
   FALSE},

  // ALLSKY_HET
  {"USA/TX: McDonald Observatory / HET (Visual)",
   "HET [Visual]",
   "www.as.utexas.edu",
   "/mcdonald/webcams/monet-n-sky.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -5.0,
   740,
   332,
   247,
   FALSE,
   FALSE,
   FALSE},
   
  // ALLSKY_CPAC
  {"Chile: Cerro Pachon (Visual)",
   "Gemini-S [Visual]",
   "www.gemini.edu",
   "/sciops/telescopes-and-sites/weather/cerro-pachon/cameras/img.png",
   "allsky.png",
   "allsky-%ld.png",
   0.0,
   1620,
   930,
   844,
   TRUE,
   TRUE,
   FALSE},
  
  // ALLSKY_LASILLA
  {"Chile: La Silla (Visual)",
   "La Silla [Visual]",
   "www.ls.eso.org",
   "/lasilla/dimm/lasc/gifs/last.gif",
   "allsky.gif",
   "allsky-%ld.gif",
   3.0,
   510,
   205,
   220,
   FALSE,
   FALSE,
   FALSE},
  
  // ALLSKY_GTC
  {"Canary: La Palma / GTC (Visual)",
   "La Palma [Visual]",
   "www.gtc.iac.es",
   "/multimedia/netcam/camaraAllSky.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   35.6,
   600,
   327,
   252,
   FALSE,
   FALSE,
   FALSE},

  // ALLSKY_KANATA
  {"Japan: Higashi-Hiroshima / Kanata (Visual)",
   "Kanata [Visual]",
   "hasc.hiroshima-u.ac.jp",
   "/environ/current_srk.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   0.0,
   602,
   304,
   289,
   FALSE,
   FALSE,
   FALSE},

  // ALLSKY_OAO
  {"Japan: Okayama Astrophysical Observatory (Visual)",
   "OAO [Visual]",
   "www.oao.nao.ac.jp",
   "/weather/skymonitor/optsky.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -3.5,
   2500,
   1278,
   1353,
   TRUE,
   FALSE,
   FALSE},

  // ALLSKY_NHAO
  {"Japan: Nishi-Harima / Nayuta (Visual)",
   "Nayuta [Visual]",
   "www.nhao.jp",
   "/nhao/live/images/skyview.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   -90.0,
   560,
   300,
   240,
   FALSE,
   FALSE,
   FALSE},

  // ALLSKY_GAO
  {"Japan: Gunma Astronomical Observatory (Visual)",
   "GAO [Visual]",
   "www.astron.pref.gunma.jp",
   "/webcam/allsky.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   0.0,
   460,
   332,
   238,
   FALSE,
   FALSE,
   FALSE},

  // ALLSKY_AAT
  {"Australia: Siding Spring Observatory (Visual)",
   "AAT [Visual]",
   "150.203.153.131",
   "/~dbayliss/allskycam/latest_image.png",
   "allsky.png",
   "allsky-%ld.png",
   -54.0,
   750,
   415,
   330,
   TRUE,
   FALSE,
   FALSE},
  
  // ALLSKY_MSO
  {"Australia: Mount Stromlo (Visual)",
   "Mt. Stromlo [Visual]",
   "www.mso.anu.edu.au",
   "/msoallsky/msoskycam.jpg",
   "allsky.jpg",
   "allsky-%ld.jpg",
   0.0,
   530,
   382,
   285,
   FALSE,
   FALSE,
   TRUE}
};


#define ALLSKY_INTERVAL 120

#define ALLSKY_LAST_MAX 40

#define ALLSKY_DIFF_BASE 64
#define ALLSKY_DIFF_MAG 12

#define ALLSKY_ALPHA (-20)

#define ALLSKY_LIMIT 1200

#define ALLSKY_CLOUD_THRESH 3.0
#define ALLSKY_SE_MIN 20.0
#define ALLSKY_SE_MAX 200.0

#define HSKYMON_HTTP_ERROR_GETHOST  -1
#define HSKYMON_HTTP_ERROR_SOCKET   -2
#define HSKYMON_HTTP_ERROR_CONNECT  -3
#define HSKYMON_HTTP_ERROR_TEMPFILE -4
#define HSKYMON_HTTP_ERROR_SSL -5

// Plot Mode
enum{ PLOT_EL, PLOT_AZ, PLOT_AD, PLOT_ADPAEL, PLOT_MOONSEP,  PLOT_HDSPA};
enum{ PLOT_ALL_SINGLE, PLOT_ALL_SELECTED,PLOT_ALL_ALL};
enum{ PLOT_OUTPUT_WINDOW, PLOT_OUTPUT_PDF, PLOT_OUTPUT_PRINT};
enum{ PLOT_CENTER_MIDNIGHT, PLOT_CENTER_CURRENT,PLOT_CENTER_MERIDIAN};
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
  gdouble pm_ra;
  gdouble pm_dec;
  gdouble equinox;
  gint pam;

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

  gchar *simbad_name;
  gchar *simbad_type;
  gdouble gaia_g;
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
  gdouble eplx;
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
  gdouble c_age;
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
  gdouble s_age;
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

  gint sz_skymon;
  gint sz_plot;
  gint sz_fc;
  gint sz_adc;

  GThread   *pthread;
  GCancellable   *pcancel;
  GMainLoop *ploop;
  GtkWidget *pdialog;
  GtkWidget *pbar;
  GtkWidget *plabel;
  GtkWidget *pbar2;
  GtkWidget *plabel2;
  GtkWidget *plabel3;
  glong psz;
  gboolean pabort;

  GThread        *asthread;
  GCancellable   *ascancel;
  GMainLoop      *asloop;
  gint           asret;
  gboolean       asabort;
  
  gint fc_pid;

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
  gint seimeistat_timer;

  gint pm_i;

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
  gchar *filename_lgs_pam;
  gchar *filename_pamout;
  gchar *dirname_pamout;
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
  gboolean obs_az_n0;
  gdouble obs_longitude;
  gdouble obs_latitude;
  gdouble obs_longitude_tmp;
  gdouble obs_latitude_tmp;
  gdouble obs_long_ss;
  gdouble obs_altitude;
  gchar *obs_tzname;
  GtkWidget *obs_combo_preset;
  GtkWidget *obs_combo_ew, *obs_combo_ns, *obs_combo_az_n0, *obs_entry_tz, *obs_frame_pos; 
  GtkAdjustment *obs_adj_lodd,*obs_adj_lomm,*obs_adj_loss;
  GtkAdjustment *obs_adj_ladd,*obs_adj_lamm,*obs_adj_lass;
  GtkAdjustment *obs_adj_alt, *obs_adj_tz;
  GtkAdjustment *obs_adj_vel_az, *obs_adj_vel_el;
  GtkAdjustment *obs_adj_wave1,*obs_adj_wave0,*obs_adj_temp,*obs_adj_pres;
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
  gboolean show_pam;
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
  GtkWidget *skymon_e_date;
  GtkWidget *skymon_frame_time;
  GtkWidget *skymon_frame_sz;
  GtkWidget *skymon_button_set;
  GtkWidget *skymon_button_fwd;
  GtkWidget *skymon_button_rev;
  GtkWidget *skymon_button_morn;
  GtkWidget *skymon_button_even;
  GtkAdjustment *skymon_adj_min;
  //GtkAdjustment *skymon_adj_objsz;
  gint skymon_mode;
  guint skymon_year,skymon_month,skymon_day;
  gint skymon_min,skymon_hour;
  gint skymon_time;
  gint skymon_objsz;

  GtkWidget *pam_button;
  gint plot_mode;
  gint plot_all;
  GtkWidget *plot_main;
  GtkWidget *plot_dw;
  gint plot_i;
  gboolean plot_moon;
  gboolean plot_pam;
  gint plot_center;
  gint plot_center_transit;
  gdouble plot_jd0;
  gdouble plot_jd1;
  gint plot_zoom;
  gint plot_zoomx;
  gdouble plot_zoomr;
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
  gint tree_width;
  gint tree_height;
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
  gint fc_shift_x;
  gint fc_shift_y;
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

  GtkWidget *hsc_show_main;
  gboolean hsc_show_ol;
  gint hsc_show_dith_i;
  gint hsc_show_dith_p;
  gint hsc_show_dith_ra;
  gint hsc_show_dith_dec;
  gint hsc_show_dith_n;
  gint hsc_show_dith_r;
  gint hsc_show_dith_t;
  gint hsc_show_osra;
  gint hsc_show_osdec;
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
  gboolean fcdb_post;
  gchar *fcdb_file;
  gint fcdb_simbad;
  gint fcdb_vizier;
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
  gint fcdb_ps1_mindet;
  gint fcdb_ps1_mode;
  gint fcdb_ps1_dr;
  gint fcdb_sdss_search;
  gint fcdb_sdss_magmin[NUM_SDSS_BAND];
  gint fcdb_sdss_magmax[NUM_SDSS_BAND];
  gboolean fcdb_sdss_fil[NUM_SDSS_BAND];
  gint fcdb_sdss_diam;
  gint fcdb_lamost_dr;
  gint fcdb_usno_mag;
  gboolean fcdb_usno_fil;
  gint fcdb_ucac_mag;
  gboolean fcdb_ucac_fil;
  gint fcdb_gaia_mag;
  gboolean fcdb_gaia_fil;
  gboolean fcdb_gaia_sat;
  gint fcdb_kepler_mag;
  gboolean fcdb_kepler_fil;
  gint fcdb_2mass_mag;
  gint fcdb_2mass_diam;
  gboolean fcdb_2mass_fil;
  gint fcdb_wise_mag;
  gboolean fcdb_wise_fil;
  gboolean fcdb_smoka_shot;
  gboolean fcdb_smoka_subaru[NUM_SMOKA_SUBARU];
  gboolean fcdb_smoka_kiso[NUM_SMOKA_KISO];
  gboolean fcdb_smoka_oao[NUM_SMOKA_OAO];
  gboolean fcdb_smoka_mtm[NUM_SMOKA_MTM];
  gboolean fcdb_smoka_kanata[NUM_SMOKA_KANATA];
  gboolean fcdb_smoka_nayuta[NUM_SMOKA_NAYUTA];
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
  gdouble addobj_pm_ra;
  gdouble addobj_pm_dec;
  gchar *addobj_magsp;
  GtkWidget *addobj_label;
  GtkWidget *addobj_entry_ra;
  GtkWidget *addobj_entry_dec;
  GtkWidget *addobj_entry_pm_ra;
  GtkWidget *addobj_entry_pm_dec;

  gint pm_type;
  GtkWidget *pm_label;
  GtkWidget *pm_label_radec;
  GtkWidget *pm_entry_pm_ra;
  GtkWidget *pm_entry_pm_dec;

  GtkWidget *adc_main;
  GtkWidget *adc_dw;
  GtkWidget *adc_button_flip; 
  GtkAdjustment *adc_adj_pa;
  GtkAdjustment *adc_adj_size;
  GtkAdjustment *adc_adj_seeing;
  GtkAdjustment *adc_adj_temp;
  GtkAdjustment *adc_adj_pres;
  gint adc_inst;
  gint adc_pa;
  gboolean adc_flip;
  gdouble adc_slit_width;
  gdouble adc_seeing;
  gdouble adc_size;
  gint adc_timer;

  gboolean allsky_flag;
  gboolean allsky_diff_flag;
  gchar *allsky_name;
  gchar *allsky_host;
  gboolean allsky_ssl;
  gchar *allsky_file;
  gchar *allsky_path;
  //  gchar *allsky_date_path;
  gchar *allsky_date;
  gchar *allsky_date_old;
  gint allsky_get_timer;
  gboolean allsky_get_flag;
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
  GtkWidget *allsky_check_ssl;
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
  gint allsky_repeat;
  GtkWidget *allsky_button;
  gint allsky_data_status;
  gint allsky_http_status;
  gdouble allsky_ratio;


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

  gboolean proxy_flag;
  gchar *proxy_host;
  gint  proxy_port;

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

  gboolean seimei_flag;
  glong seimei_id;
  int seimei_socket;
  gdouble seimei_az;
  gdouble seimei_el;
  
  FILE *fp_log;
  gchar *filename_log;

  gint ope_max;
  gint add_max;
  gint nst_max;
#ifdef USE_GTK3
  GdkRGBA *col[MAX_ROPE];
  GdkRGBA *col_edge;
#else
  GdkColor *col[MAX_ROPE];
  GdkColor *col_edge;
  gint alpha_edge;
#endif
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

  LGS_PAM_Entry lgs_pam[MAX_LGS_PAM];
  struct ln_zonedate pam_zonedate;
  gint lgs_pam_i_max;
  gchar *pam_name;
  GtkWidget* pam_main;
  GtkWidget *pam_tree;
  GtkWidget *pam_label_obj;
  GtkWidget *pam_label_pam;
  gint pam_slot_i;
  gint pam_obj_i;
  gint pam_x[MAX_LGS_PAM_TIME];
  gint pam_y[MAX_LGS_PAM_TIME];

  gchar *nst_found[MAX_ROPE];
  gchar i_nst_found;
};


// Struct for Callback
typedef struct{
#ifdef USE_GTK3  
  GdkRGBA *col;
#else
  GdkColor *col;
#endif
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



#define is_num_char(c) ((c>='0' && c<='9')||(c==' ')||(c=='\t')||(c=='.')||(c=='+')||(c=='-')||(c=='\n'))





//// Functions' proto-type
// main.c
#ifdef USE_GTK3
void css_change_col();
void css_change_pbar_height();
#endif
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
void get_current_obs_time();
void add_day();
void do_save_FCDB_List();
void do_save_TRDB_CSV();
void do_quit();
void do_plot();
void do_update_azel();
void popup_message(GtkWidget*, gchar*, gint , ...);
void create_fcdb_para_dialog();
gboolean is_separator();
GtkWidget *make_menu();
gboolean check_allsky();
#ifdef USE_XMLRPC
gint update_telstat();
#endif
int create_seimei_socket();
gint update_seimeistat();
void clear_trdb();
void init_obj();
gboolean check_ttgs();
gchar *cut_spc();
gint is_number();
gchar* to_utf8();
gchar* to_locale();
gchar* check_ext();
gchar* make_head();
gchar *make_filehead();
void update_c_label();
gboolean ObjOverlap();
char *my_strcasestr();
void CheckTargetDefOPE();
gint CheckTargetDefOPE2();
gchar *trdb_csv_name();
gchar *fcdb_csv_name();
void ReadTRDB();
void WriteTRDB();


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
void get_plot_time_meridian();
gdouble deg_sep();
void ext_play();

//fc.c
void thread_cancel_fcdb();
gboolean delete_fcdb();
void pdf_fc ();
void set_fc_mode();
void fc_item2 ();
void fcdb_item2();
void fcdb_dl();
void trdb_run();
gboolean draw_fc_cairo();
void fcdb_tree_update_azel_item();
void trdb_tree_update_azel_item();
void ver_dl();
void addobj_dl();
void pm_dl();
gdouble current_yrs();
void make_trdb_label();
void fcdb_make_tree();
void trdb_make_tree();

//fc_output.c
void Export_FCDB_CSV();
void Export_TRDB_CSV();

//http-client.c
int start_get_allsky();
void cancel_allsky();
void terminate_allsky();
GdkPixbuf* diff_pixbuf();
gpointer thread_get_dss();
gpointer thread_get_stddb();
gpointer thread_get_fcdb();
void allsky_debug_print (const gchar *format, ...) G_GNUC_PRINTF(1, 2);
int month_from_string_short();

int get_seimei_azel();

// json_parse
void fcdb_gemini_json_parse();
void trdb_gemini_json_parse();

//julian_day.c
void my_get_local_date();
int get_gmtoff_from_sys ();

//progress.c
glong get_file_size();
void create_pdialog();
gboolean progress_timeout();

//skymon.c
void skymon_set_time_current();
void skymon_debug_print(const gchar *format, ...);
gboolean draw_skymon();
void create_skymon_dialog();
gboolean draw_skymon_cairo();
#ifdef USE_XMLRPC
gboolean draw_skymon_with_telstat_cairo();
#endif
gboolean draw_skymon_with_seimei_cairo();

//remoteObjects.c
int ro_init();

//telstat.c
#ifdef USE_XMLRPC
int close_telstat();
int get_telstat();
int get_rope();
#endif

//treeview.c
void move_focus_item();
void tree_update_azel_item();
void make_tree();
void remake_tree();
void rebuild_fcdb_tree();
gint tree_update_azel();
gint trdb_tree_update_azel();
gchar* make_tgt();
gchar *make_simbad_id();
void addobj_dialog();
void raise_tree();
void str_replace();
void fcdb_append_tree();
gchar *strip_spc();

//utility.c
void ln_deg_to_dms();
double ln_dms_to_deg();
void ln_equ_to_hequ();

//votable.c
void make_band_str();
void fcdb_simbad_vo_parse();
void fcdb_ned_vo_parse();
void fcdb_gsc_vo_parse();
void fcdb_ps1_vo_parse();
void fcdb_sdss_vo_parse();
void fcdb_usno_vo_parse();
void fcdb_ucac_vo_parse();
void fcdb_gaia_vo_parse();
void fcdb_kepler_vo_parse();
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
void addobj_transient_txt_parse();

//jason_parse.c
void fcdb_gemini_jason_parse();
void trdb_gemini_jason_parse();
