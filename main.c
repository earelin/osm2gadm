#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libpq-fe.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <geos_c.h>
#include "database.h"
#include "datatypes.h"
#include "process.h"
#include "utils.h"

#define TESTING_COUNTRY 49898

void notice (const char *fmt, ...);
void log_and_exit (const char *fmt, ...);
void process_boundaries (country_type country);

PGconn *dbconn;

/*
 * 
 */
int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      printf ("You must enter a connection string as first parameter.\n");
      return EXIT_FAILURE;
    }

  dbconn = PQconnectdb (argv[1]);

  if (PQstatus (dbconn) != CONNECTION_OK)
    {
      printf ("Can't open database\n");
      return EXIT_FAILURE;
    }
  printf ("Opened database successfully: %s\n", PQdb (dbconn));

  initGEOS (notice, log_and_exit);
  printf ("GEOS version %s\n", GEOSversion ());

  // Relations procesing loop.
  GArray *relations = database_get_relations ();

  //database_create_tables();

  for (int i = 0; i < relations->len; i++)
    {				//relations->len; i++) {
      country_type country = g_array_index (relations, country_type, i);
      printf ("%s (%s %d)\n", country.name, country.iso2, country.id);
      process_boundaries (country);
    }

  // Closing application.
  country_type_array_free (relations);
  finishGEOS ();
  PQfinish (dbconn);

  return EXIT_SUCCESS;
}

void
process_boundaries (country_type country)
{
  GPtrArray *outer_lines;
  GPtrArray *inner_lines;
  GPtrArray *outer_polygons;
  GPtrArray *inner_polygons;
  GPtrArray *polygons;

  printf ("\tOuter lines\n");
  outer_lines = database_get_admin_lines (country.id, "outer");
  printf ("\t\tLines: %d\n", outer_lines->len);
  outer_lines = process_lines_merge (outer_lines);
  outer_lines = process_lines_validation (outer_lines);
  printf ("\t\tLines merged: %d\n", outer_lines->len);
  outer_polygons = process_polygons_generate (outer_lines);
  outer_polygons = process_polygon_validation (outer_polygons);
  printf ("\t\tPolygons generated: %d\n", outer_polygons->len);

  printf ("\tInner lines\n");
  inner_lines = database_get_admin_lines (country.id, "inner");
  printf ("\t\tLines: %d\n", inner_lines->len);
  inner_lines = process_lines_merge (inner_lines);
  printf ("\t\tLines merged: %d\n", inner_lines->len);
  inner_lines = process_lines_validation (inner_lines);
  inner_polygons = process_polygons_generate (inner_lines);
  printf ("\t\tPolygons generated: %d\n", inner_polygons->len);

  polygons = process_polygons_merge (outer_polygons, inner_polygons);
  polygons = process_polygons_cut_coastile (polygons);

  utils_geom_array_free (inner_polygons);

  database_save_country_polygons (country, polygons);
  printf ("\tPolygons written: %d\n", polygons->len);

  utils_geom_array_free (outer_polygons);
  //utils_geom_array_free (polygons);
}

void
notice (const char *fmt, ...)
{
  va_list ap;

  fprintf (stdout, "\nNOTICE: ");

  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  va_end (ap);
  fprintf (stdout, "\n");
}

void
log_and_exit (const char *fmt, ...)
{
  va_list ap;

  fprintf (stdout, "\nERROR: ");

  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  va_end (ap);
  fprintf (stdout, "\n");
  exit (1);
}
