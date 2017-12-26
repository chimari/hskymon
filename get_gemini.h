//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      get_gemini.h  --- GET URL for FCDB access to Gemini web search
//   
//                                           2017.11.15  A.Tajitsu

typedef struct _PARAMgeminiinst PARAMgeminiinst;
struct _PARAMgeminiinst{
  gchar *name;
  gchar *prm;
};


// Gemini Obs mode
enum
{
  TRDB_GEMINI_MODE_ANY,
  TRDB_GEMINI_MODE_IMAGE,
  TRDB_GEMINI_MODE_SPEC
};

enum{
  GEMINI_INST_ANY,
  GEMINI_INST_GMOS,
  GEMINI_INST_GNIRS,
  GEMINI_INST_GRACES,
  GEMINI_INST_NIRI,
  GEMINI_INST_NIFS,
  GEMINI_INST_GSAOI,
  GEMINI_INST_F2,
  GEMINI_INST_GPI,
  GEMINI_INST_NICI,
  GEMINI_INST_MICHELLE,
  GEMINI_INST_TRECS,
  GEMINI_INST_BHORS,
  GEMINI_INST_HRWFS,
  GEMINI_INST_OSCIR,
  GEMINI_INST_FLAMINGOS,
  GEMINI_INST_HOKUPAA,
  GEMINI_INST_PHOENIX,
  GEMINI_INST_TEXES,
  GEMINI_INST_ABU,
  GEMINI_INST_CIRPASS,
  NUM_GEMINI_INST
};

static const PARAMgeminiinst gemini_inst[NUM_GEMINI_INST] = {
  {"(Any)",             NULL},  
  {"GMOS-N & GMOS-S", "GMOS"},  
  {"GNIRS",           "GNIRS"},  
  {"GRACES",          "GRACES"},  
  {"NIRI",            "NIRI"},  
  {"NIFS",            "NIFS"},  
  {"GSAOI",           "GSAOI"},  
  {"F2",              "F2"},  
  {"GPI",             "GPI"},
  {"NICI",            "NICI"},
  {"Michelle",        "michelle"},
  {"T-ReCS",          "TReCS"},
  {"bHROS",           "bHROS"},
  {"HRWFS / AcqCam",  "hrwfs"},
  {"OSCIR",           "OSCIR"},
  {"FLAMINGOS",       "FLAMINGOS"},
  {"Hokupaa+QUIRC",   "Hokupaa+QUIRC"},
  {"PHOENIX",         "PHOENIX"},
  {"TEXES",           "TEXES"},
  {"ABU",             "ABU"},
  {"CIRPASS",         "CIRPASS"}
};

