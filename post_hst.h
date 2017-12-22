//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_hst.h  --- POST body for FCDB access to HST web search
//   
//                                           2017.11.15  A.Tajitsu

typedef struct _PARAMhstinst PARAMhstinst;
struct _PARAMhstinst{
  gchar *name;
};

#define NUM_HST_IMAGE 8
#define NUM_HST_SPEC 8
#define NUM_HST_OTHER 2

// HST Obs mode
enum
{
  TRDB_HST_MODE_IMAGE,
  TRDB_HST_MODE_SPEC,
  TRDB_HST_MODE_OTHER
};

static const PARAMhstinst hst_image[NUM_HST_IMAGE] = {
  {"STIS"},  
  {"NICMOS"},  
  {"WFPC2"},  
  {"WFPC"},  
  {"FOC"},  
  {"ACS"},  
  {"WFC3"},  
  {"COS"}
};

static const PARAMhstinst hst_spec[NUM_HST_SPEC] = {
  {"STIS"},  
  {"NICMOS"},  
  {"GHRS"},  
  {"FOS"},  
  {"FOC"},  
  {"ACS"},  
  {"WFC3"},  
  {"COS"}
};

static const PARAMhstinst hst_other[NUM_HST_OTHER] = {
  {"FGS"},  
  {"HSP"}  
};

static const PARAMpost hst_post[] = { 
  {POST_CONST,   "action",        "Search"},
  {POST_NULL,    "target",        NULL},
  {POST_CONST,   "resolver",      "Don%27t+Resolve"},
  {POST_INPUT,   "radius",        NULL},
  {POST_INPUT,   "ra",            NULL},
  {POST_INPUT,   "dec",           NULL},
  {POST_CONST,   "equinox",       "J2000"},
  {POST_INST1,   "image%5B%5D",       NULL},
  {POST_INST2,   "spectrum%5B%5D",    NULL},
  {POST_INST3,   "other%5B%5D",       NULL},
  {POST_NULL,    "sci_start_time",       NULL},
  {POST_NULL,    "sci_actual_duration",       NULL},
  {POST_NULL,    "sci_pep_id",       NULL},
  {POST_NULL,    "sci_release_date",       NULL},
  {POST_NULL,    "sci_data_set_name",       NULL},
  {POST_NULL,    "sci_spec_1234",       NULL},
  {POST_NULL,    "sci_obset_id",       NULL},
  {POST_NULL,    "sci_archive_date",       NULL},
  {POST_NULL,    "sci_target_descrip",       NULL},
  {POST_NULL,    "sci_aper_1234",       NULL},
  {POST_CONST,   "sci_aec%5B%5D",       "S"},
  {POST_NULL,    "sci_pi_last_name",       NULL},
  {POST_CONST,   "extra_column_name_1",       "sci_data_set_name"},
  {POST_NULL,    "extra_column_value_1",       NULL},
  {POST_CONST,   "extra_column_name_2",       "sci_data_set_name"},
  {POST_NULL,    "extra_column_value_2",       NULL},
  {POST_CONST,   "selectedColumnsCsv",       "Mark%2Csci_data_set_name%2Csci_targname%2Csci_ra%2Csci_dec%2Csci_refnum%2Csci_start_time%2Csci_stop_time%2Csci_actual_duration%2Csci_instrume%2Csci_aper_1234%2Csci_spec_1234%2Csci_central_wavelength%2Csci_pep_id%2Csci_release_date%2Csci_preview_name%2Cscp_scan_type%2Csci_hlsp%2Cang_sep"},
  {POST_CONST,   "selectedColumnsList%5B%5D",       "sci_central_wavelength"},
  {POST_CONST,   "availableColumns",       "Mark"},
  {POST_CONST,   "ordercolumn1",       "ang_sep"},
  {POST_CONST,   "ordercolumn2",       "sci_targname"},
  {POST_CONST,   "ordercolumn3",       "sci_data_set_name"},
  {POST_CONST,   "coordformat",       "dec"},
  {POST_INPUT,   "outputformat",       NULL},  // VOTable or HTML_Table
  {POST_CONST,   "max_records",      "5001"},
  {POST_CONST,   "max_rpp",       "500"},
  {POST_NULL,  NULL, NULL}};
