//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_smoka.h  --- POST body for FCDB access to SMOKA web search
//   
//                                           2017.11.15  A.Tajitsu

typedef struct _PARAMsmokainst PARAMsmokainst;
struct _PARAMsmokainst{
  gchar *name;
  gchar *prm;
};

#define NUM_SMOKA_INST 12

static const PARAMsmokainst smoka_inst[NUM_SMOKA_INST] = {
  {"Suprime-Cam","SUP"},  
  {"FOCAS",      "FOC"},  
  {"HDS",        "HDS"},  
  {"OHS/CISCO",  "OHS"},  
  {"IRCS",       "IRC"},  
  {"CIAO",       "CIA"},  
  {"COMICS",     "COM"},  
  {"MOIRCS",     "MCS"},  
  {"Kyoto3DII",  "K3D"},  
  {"HiCIAO",     "HIC"},  
  {"FMOS",       "FMS"},  
  {"Hyper Suprime-Cam", "HSC"}
};

static const PARAMpost smoka_post[] = { 
  {POST_NULL,   "object",        NULL},
  {POST_CONST,  "resolver",      "SIMBAD"},
  {POST_CONST,  "coordsys",      "Equatorial"},
  {POST_CONST,  "equinox",       "J2000"},
  {POST_CONST,  "fieldofview",   "auto"},
  {POST_CONST,  "RadOrRec",      "radius"},
  {POST_INPUT,  "longitudeC",    NULL},
  {POST_INPUT,  "latitudeC",     NULL},
  {POST_INPUT,  "radius",        NULL},
  {POST_NULL,   "longitudeF",    NULL},
  {POST_NULL,   "latitudeF",     NULL},
  {POST_NULL,   "longitudeT",    NULL},
  {POST_NULL,   "latitudeT",     NULL},
  {POST_NULL,   "date_obs",      NULL},
  {POST_NULL,   "exptime",       NULL},
  {POST_NULL,   "observer",      NULL},
  {POST_NULL,   "prop_id",       NULL},
  {POST_NULL,   "frameid",       NULL},
  {POST_NULL,   "exp_id",        NULL},
  {POST_NULL,   "dataset",       NULL},
  {POST_NULL,   "dataset",       NULL},
  {POST_CONST,  "asciitable",    "Ascii"},
  {POST_CONST,  "frameorshot",   "Frame"},
  {POST_CONST,  "action",        "Search"},
  {POST_INST1,  "instruments",   NULL},
  {POST_INST1,  "multiselect_0",   NULL},
  {POST_CONST,  "obs_mod",   "IMAG"},
  {POST_CONST,  "obs_mod",   "SPEC"},
  {POST_CONST,  "obs_mod",   "IPOL"},
  {POST_CONST,  "multiselect_1",   "IMAG"},
  {POST_CONST,  "multiselect_1",   "SPEC"},
  {POST_CONST,  "multiselect_1",   "IPOL"},
  {POST_CONST,  "data_typ",   "OBJECT"},
  {POST_CONST,  "multiselect_2",   "OBJECT"},
  {POST_CONST,  "obs_cat",   "Science+Observation"},
  {POST_CONST,  "multiselect_3",   "Science+Observation"},
  {POST_CONST,  "bandwidth_type",   "FILTER"},
  {POST_NULL,   "band",   NULL},
  {POST_CONST,  "dispcol",   "FRAMEID"},
  {POST_CONST,  "dispcol",   "DATE_OBS"},
  {POST_CONST,  "dispcol",   "FITS_SIZE"},
  {POST_CONST,  "dispcol",   "OBS_MODE"},
  {POST_CONST,  "dispcol",   "DATA_TYPE"},
  {POST_CONST,  "dispcol",   "OBJECT"},
  {POST_CONST,  "dispcol",   "FILTER"},
  {POST_CONST,  "dispcol",   "WVLEN"},
  {POST_CONST,  "dispcol",   "DISPERSER"},
  {POST_CONST,  "dispcol",   "RA2000"},
  {POST_CONST,  "dispcol",   "DEC2000"},
  {POST_CONST,  "dispcol",   "GALLONG"},
  {POST_CONST,  "dispcol",   "GALLAT"},
  {POST_CONST,  "dispcol",   "ECLLONG"},
  {POST_CONST,  "dispcol",   "ECLLAT"},
  {POST_CONST,  "dispcol",   "UT_START"},
  {POST_CONST,  "dispcol",   "EXPTIME"},
  {POST_CONST,  "dispcol",   "OBSERVER"},
  {POST_CONST,  "dispcol",   "EXP_ID"},
  {POST_CONST,  "dispcol",   "ENG_FLAG"},
  {POST_CONST,  "orderby",   "DATE_OBS"},
  {POST_CONST,  "asc",       "1"},
  {POST_CONST,  "diff",      "500"},
  {POST_CONST,  "output_equinox",      "J2000"},
  {POST_CONST,  "from",      "0"},
  {POST_NULL,  NULL, NULL}};
