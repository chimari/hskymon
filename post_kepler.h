//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_kepler.h  --- POST body for FCDB access to Kepler web search
//   
//                                           2018.7.23  A.Tajitsu

static const PARAMpost kepler_post[] = { 
  {POST_NULL,  "target",    NULL},
  {POST_CONST, "resolver",  "Don%27t+Resolve"},
  {POST_INPUT, "radius", NULL}, 
  {POST_INPUT, "ra", NULL}, 
  {POST_INPUT, "dec", NULL}, 
  {POST_CONST, "equinox", "J2000"}, 
  {POST_NULL,  "kic_kepler_id",    NULL},
  {POST_NULL,  "kic_pmtotal",    NULL},
  {POST_NULL,  "kic_teff",    NULL},
  {POST_NULL,  "kic_radius",    NULL},
  {POST_NULL,  "kic_ebminusv",    NULL},
  {POST_NULL,  "kic_logg",    NULL},
  {POST_NULL,  "kic_feh",    NULL},
  {POST_CONST,  "extra_column_name_1", "kic_kepmag"},
  {POST_INPUT,  "extra_column_value_1",    NULL},
  {POST_CONST,  "extra_column_name_2", "kic_kepler_id"},
  {POST_NULL,   "extra_column_value_2",    NULL},
  {POST_CONST,  "selectedColumnsCsv", "kic_kepler_id,kic_degree_ra,kic_dec,kic_rmag,kic_jmag,kic_kepmag,kic_2mass_id,kic_galaxy,kic_teff,kic_logg,kic_feh,kic_ebminusv,kic_radius,kic_parallax,kic_pmtotal,kic_grcolor,ang_sep"},
  {POST_CONST,  "availableColumns", "kic_kepler_id"},
  {POST_CONST,  "ordercolumn1", "kic_kepler_id"},
  {POST_CONST,  "ordercolumn2", "kic_kepler_id"},
  {POST_NULL,   "ordercolumn3",    NULL},
  {POST_CONST,  "coordformat", "dec"},
  {POST_CONST,  "outputformat", "VOTable"},
  {POST_CONST,  "max_records",      "5001"},
  {POST_CONST,  "max_rpp",       "5000"},
  {POST_CONST,  "action", "Search"}, 
  {POST_NULL,  NULL, NULL}};
