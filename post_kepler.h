//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post_kepler.h  --- POST body for FCDB access to Kepler web search
//   
//                                           2018.7.23  A.Tajitsu

static const PARAMpost kepler_post[] = { 
  {POST_NULL,  "target",    NULL},
  {POST_CONST, "resolver",  "Resolve"},
  {POST_INPUT, "radius", NULL}, 
  {POST_INPUT, "ra", NULL}, 
  {POST_INPUT, "dec", NULL}, 
  {POST_CONST, "equinox", "J2000"}, 
  {POST_NULL,  "ktc_kepler_id",    NULL},
  {POST_NULL,  "ktc_investigation_id",    NULL},
  {POST_NULL,  "kic_2mass_id",    NULL},
  {POST_NULL,  "kic_kepmag",    NULL},
  {POST_CONST, "ktc_target_type[]",   "LC"},
  {POST_CONST, "ktc_target_type[]",   "SC"},
  {POST_NULL,  "sci_release_date",   NULL},
  {POST_NULL,  "kic_teff",    NULL},
  {POST_NULL,  "kic_logg",    NULL},
  {POST_NULL,  "sci_data_quarter",   NULL},
  {POST_NULL,  "condition_flag",   NULL},
  {POST_CONST, "extra_column_name_1",  "ktc_kepler_id"},
  {POST_NULL,  "extra_column_value_1",  NULL},
  {POST_CONST, "extra_column_name_2",  "ktc_kepler_id"},
  {POST_NULL,  "extra_column_value_2",  NULL},
  {POST_CONST, "extra_column_name_3",  "ktc_kepler_id"},
  {POST_NULL,  "extra_column_value_3",  NULL},
  {POST_CONST, "extra_column_name_4",  "ktc_kepler_id"},
  {POST_NULL,  "extra_column_value_4",  NULL},
  {POST_CONST,  "selectedColumnsCsv", "Mark,ktc_kepler_id,ktc_investigation_id,sci_data_set_name,sci_data_quarter,sci_ra,sci_dec,ktc_target_type,sci_archive_class,refnum,sci_start_time,sci_end_time,sci_release_date,kic_rmag,twomass_jmag,kic_kepmag,twomass_2mass_id,twomass_conflictflag,kic_teff,kic_logg,kic_feh,kic_ebminusv,kic_radius,kic_pmtotal,kic_grcolor,sci_module,sci_output,sci_channel,sci_skygroup_id,condition_flag,ang_sep"},
  {POST_CONST, "availableColumns",  "Mark"},
  {POST_CONST, "ordercolumn1",  "ang_sep"},
  {POST_CONST, "ordercolumn2",  "ktc_kepler_id"},
  {POST_NULL,  "ordercolumn3",  NULL},
  {POST_CONST, "coordformat",  "dec"},
  {POST_CONST,  "outputformat", "VOTable"},
  {POST_CONST,  "max_records",      "5001"},
  {POST_CONST,  "max_rpp",       "5000"},
  {POST_CONST,  "action", "Search"}, 
  {POST_NULL,  NULL, NULL}};
