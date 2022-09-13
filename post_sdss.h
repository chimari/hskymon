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

typedef enum{ FCDB_SDSS_SEARCH_IMAG, FCDB_SDSS_SEARCH_SPEC} enum_SDSSSearch;
extern enum_SDSSSearch SDSSSearch;


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
