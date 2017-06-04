/* 
 * File:   datatypes.h
 * Author: xavier
 *
 * Created on 02 June 2017, 18:26
 */

#ifndef DATATYPES_H
#define DATATYPES_H

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum _main_command
  {
    GADM_CREATE_TABLES = 1,
    GADM_CREATE_LINES = 2,
    GADM_CREATE_POLYGONS = 4,
    GADM_CUT_COASTLINE = 8,
    GADM_SPLIT_POLYGONS = 16,
    GADM_INFO = 32,
    GADM_ALL = 63
  } main_command;

  typedef struct _app_configuration
  {
    char *db_host;
    char *db_name;
    char *db_username;
    char *db_password;
    char *output_db_name;
    gboolean truncate;

    char *country_iso;
    int admin_level;

    int threads;

    int operations;

    gboolean verbose;
  } app_configuration;

  typedef struct _country_type
  {
    int id;
    char *name, *iso2;
  } country_type;

  void country_type_array_free (GArray * countries);

#ifdef __cplusplus
}
#endif

#endif				/* DATATYPES_H */
