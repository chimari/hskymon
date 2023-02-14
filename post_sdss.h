//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_sdss.h  --- POST body for FCDB access to SDSS skyserver
//   
//                                           2017.12.04  A.Tajitsu

#define NUM_SDSS_BAND 5

static gchar *sdss_band[NUM_SDSS_BAND]=
{
  "u", "g", "r", "i", "z"
};

static gchar *sdss_band_min[NUM_SDSS_BAND]=
{
  "min_u", "min_g", "min_r", "min_i", "min_z"
};

static gchar *sdss_band_max[NUM_SDSS_BAND]=
{
  "max_u", "max_g", "max_r", "max_i", "max_z"
};

enum{ FCDB_SDSS_SEARCH_IMAG, FCDB_SDSS_SEARCH_SPEC};



static const PARAMpost sdss_post[] = {
  {POST_INPUT, "searchtool", NULL},  //Imaging or Spectro
  {POST_INPUT, "TaskName", NULL},  //"Skyserver.Search.IQS" or "Skyserver.Search.SQS"
  {POST_CONST, "ReturnHtml", "false"},
  {POST_CONST, "limit", "5000"},
  {POST_CONST, "format", "votable"},
  {POST_NULL, "TableName", NULL},
  {POST_CONST, "imgparams", "typical"},
  {POST_CONST, "imgparams", "model_mags"},
  {POST_CONST, "imgparams", "objID"},
  {POST_CONST, "specparams", "typical"},
  {POST_NULL, "raMin", NULL},
  {POST_NULL, "decMin", NULL},
  {POST_NULL, "raMax", NULL},
  {POST_NULL, "decMax", NULL},
  {POST_CONST, "positionType", "cone"},
  {POST_INPUT, "ra", NULL},
  {POST_INPUT, "dec", NULL},
  {POST_INPUT, "radius", NULL},
  {POST_NULL, "radecTextarea", NULL},
  {POST_CONST, "searchNearBy", "nearest"},
  {POST_CONST, "radiusDefault", "1.0"},
  {POST_CONST, "magType", "model"},
  {POST_INPUT, "magMin", NULL},  // [u,g,r,i,z]Min
  {POST_INPUT, "magMax", NULL},  // [u,g,r,i,z]Max
  {POST_NULL, "ugMin", NULL},
  {POST_NULL, "grMin", NULL},
  {POST_NULL, "riMin", NULL},
  {POST_NULL, "izMin", NULL},
  {POST_NULL, "ugMax", NULL},
  {POST_NULL, "grMax", NULL},
  {POST_NULL, "riMax", NULL},
  {POST_NULL, "izMax", NULL},
  {POST_CONST, "doGalaxy", "on"},
  {POST_CONST, "doStar", "on"},
  {POST_NULL, "minQA", NULL},
  {POST_CONST, "flagsOnList", "ignore"},
  {POST_CONST, "flagsOffList", "ignore"},
  {POST_NULL,  NULL, NULL}};
/*
static const PARAMpost sdss_post[] = {
  {POST_CONST, "searchtool", "Rectangular"},  
  {POST_CONST, "TaskName", "Skyserver.Search.Rectangular"},
  {POST_CONST, "ReturnHtml", "false"},
  {POST_CONST, "whichphotometry", "optical"},
  {POST_CONST, "coordtype", "equatrial"},
  {POST_INPUT, "min_ra", NULL},
  {POST_INPUT, "max_ra", NULL},
  {POST_INPUT, "min_dec", NULL},
  {POST_INPUT, "max_dec", NULL},
  {POST_INPUT, "min_u", NULL},
  {POST_INPUT, "max_u", NULL},
  {POST_INPUT, "min_g", NULL},
  {POST_INPUT, "max_g", NULL},
  {POST_INPUT, "min_r", NULL},
  {POST_INPUT, "max_r", NULL},
  {POST_INPUT, "min_i", NULL},
  {POST_INPUT, "max_i", NULL},
  {POST_INPUT, "min_z", NULL},
  {POST_INPUT, "max_z", NULL},
  {POST_CONST, "min_j", "-10"},
  {POST_CONST, "max_j", "99"},
  {POST_CONST, "min_h", "-10"},
  {POST_CONST, "max_h", "99"},
  {POST_CONST, "min_k", "-10"},
  {POST_CONST, "max_k", "99"},
  {POST_CONST, "format", "VOTable"},
  {POST_NULL, "TableName", NULL},
  {POST_CONST, "limit", "5000"},
  {POST_NULL,  NULL, NULL}
};
*/
