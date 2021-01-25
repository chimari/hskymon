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

#define NUM_SMOKA_SUBARU 16
#define NUM_SMOKA_KISO 3
#define NUM_SMOKA_OAO 7
#define NUM_SMOKA_MTM 2
#define NUM_SMOKA_KANATA 2
#define NUM_SMOKA_NAYUTA 1

static const PARAMsmokainst smoka_subaru[NUM_SMOKA_SUBARU] = {
  {"Suprime-Cam","SUP"},  
  {"FOCAS",      "FCS"},  
  {"HDS",        "HDS"},  
  {"OHS/CISCO",  "OHS"},  
  {"IRCS",       "IRC"},  
  {"CIAO",       "CIA"},  
  {"COMICS",     "COM"},  
  {"MOIRCS",     "MCS"},  
  {"Kyoto3DII",  "K3D"},  
  {"HiCIAO",     "HIC"},  
  {"FMOS",       "FMS"},  
  {"Hyper Suprime-Cam", "HSC"},
  {"CHARIS", "CRS"},
  {"IRD", "IRD"},
  {"SWIMS", "SWS"},
  {"MIMIZUKU", "MMZ"}
};

static const PARAMsmokainst smoka_kiso[NUM_SMOKA_KISO] = {
  {"1k CCD",      "KCC"},  
  {"2k CCD",      "KCD"},  
  {"KWFC",        "KWF"}  
};

static const PARAMsmokainst smoka_oao[NUM_SMOKA_OAO] = {
  {"ISLE",   "ISL"},  
  {"KOOLS",  "KLS"},  
  {"HIDES",  "HID"},
  {"OASIS",  "OAS"},
  {"SNG",    "CSD"},
  {"MuSCAT", "MCT"},
  {"KOOLS-IFU", "KIF"}
};

static const PARAMsmokainst smoka_mtm[NUM_SMOKA_MTM] = {
  {"AKENO",   "MTA"},  
  {"OAO",     "MTO"}
};

static const PARAMsmokainst smoka_kanata[NUM_SMOKA_KANATA] = {
  {"HOWPol",   "HWP"},  
  {"HONIR",    "HNR"}
};

static const PARAMsmokainst smoka_nayuta[NUM_SMOKA_NAYUTA] = {
  {"NIC",   "NIC"}  
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
  {POST_INPUT,  "date_obs",      NULL},
  {POST_NULL,   "exptime",       NULL},
  {POST_NULL,   "observer",      NULL},
  {POST_NULL,   "prop_id",       NULL},
  {POST_NULL,   "frameid",       NULL},
  {POST_NULL,   "exp_id",        NULL},
  {POST_NULL,   "dataset",       NULL},
  {POST_NULL,   "dataset",       NULL},
  {POST_INPUT,  "asciitable",    NULL},  // "Table" or "Ascii"
  {POST_INPUT,  "frameorshot",   NULL}, // "Frame" or "Shot"
  {POST_CONST,  "action",        "Search"},
  {POST_INST1,  "instruments",   NULL},
  {POST_INST2,  "instruments",   NULL},
  {POST_INST3,  "instruments",   NULL},
  {POST_INST4,  "instruments",   NULL},
  {POST_INST5,  "instruments",   NULL},
  {POST_INST1,  "multiselect_0",   NULL},
  {POST_INST2,  "multiselect_0",   NULL},
  {POST_INST3,  "multiselect_0",   NULL},
  {POST_INST4,  "multiselect_0",   NULL},
  {POST_INST5,  "multiselect_0",   NULL},
  {POST_IMAG,   "obs_mod",   "IMAG"},
  {POST_SPEC,   "obs_mod",   "SPEC"},
  {POST_IPOL,   "obs_mod",   "IPOL"},
  {POST_IMAG,   "multiselect_1",   "IMAG"},
  {POST_SPEC,   "multiselect_1",   "SPEC"},
  {POST_IPOL,   "multiselect_1",   "IPOL"},
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
  {POST_CONST,  "diff",      "5000"},
  {POST_CONST,  "output_equinox",      "J2000"},
  {POST_CONST,  "from",      "0"},
  {POST_NULL,  NULL, NULL}};
