#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libpq-fe.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <geos_c.h>
#include <string.h>
#include "database.h"
#include "datatypes.h"
#include "process.h"
#include "utils.h"

#define TESTING_COUNTRY 49898
#define GADM_CREATE_TABLES_COMMAND "create-tables"
#define GADM_CREATE_LINES_COMMAND "lines"
#define GADM_CREATE_POLYGONS_COMMAND "polygons"
#define GADM_CUT_COASTLINE_COMMAND "coastline"
#define GADM_SPLIT_POLYGONS_COMMAND "split-polygons"
#define GADM_INFO_COMMAND "info"
#define GADM_ALL_COMMAND "all"

void notice (const char *fmt, ...);
void log_and_exit (const char *fmt, ...);
void process_boundaries (country_type country);
void extract_lines (country_type * country);
void extract_polygons (country_type country);

PGconn *dbconn;
PGconn *dbconn_output;

app_configuration configuration;
char **command_argv = NULL;

static GOptionEntry entries[] = {
  {"host", 'h', 0, G_OPTION_ARG_STRING, &(configuration.db_host),
   "Database host",
   NULL},
  {"database", 'd', 0, G_OPTION_ARG_STRING, &(configuration.db_name),
   "Database name", NULL},
  {"user", 'u', 0, G_OPTION_ARG_STRING, &(configuration.db_username),
   "Database user name", NULL},
  {"password", 'p', 0, G_OPTION_ARG_STRING, &(configuration.db_password),
   "Database user password", NULL},
  {"country", 'c', 0, G_OPTION_ARG_STRING, &(configuration.country_iso),
   "Country to be processed in iso 2 code", NULL},
  {"level", 'l', 0, G_OPTION_ARG_INT, &(configuration.admin_level),
   "Administrative level", NULL},
  {"threads", 't', 0, G_OPTION_ARG_INT, &(configuration.threads),
   "Maximum number of threads.", NULL},
  {"output-database", 'o', 0, G_OPTION_ARG_STRING,
   &(configuration.output_db_name),
   "Database name for output data", NULL},
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &(configuration.verbose),
   "Provide more output information", NULL},
  {"truncate", 0, 0, G_OPTION_ARG_NONE, &(configuration.truncate),
   "Truncate tables", NULL},
  {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command_argv,
   "Execute command.", "[COMMAND]"},
  {NULL}
};

/*
 * 
 */
int
main (int argc, char **argv)
{
  // Parse command line options
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("%s\n", error->message);
      exit (1);
    }
  if (configuration.threads == 0)
    configuration.threads = 1;

  // Validate command line options
  int i = 0;
  while (command_argv[i] != NULL)
    {
      char *command = command_argv[i];
      if (strcmp (command, GADM_ALL_COMMAND) == 0)
	{
	  configuration.operations |= GADM_ALL;
	  break;
	}
      else if (strcmp (command, GADM_CREATE_TABLES_COMMAND) == 0)
	{
	  configuration.operations |= GADM_CREATE_TABLES;
	}
      else if (strcmp (command, GADM_CREATE_LINES_COMMAND) == 0)
	{
	  configuration.operations |= GADM_CREATE_LINES;
	}
      else if (strcmp (command, GADM_CREATE_POLYGONS_COMMAND) == 0)
	{
	  configuration.operations |= GADM_CREATE_POLYGONS;
	}
      else if (strcmp (command, GADM_CUT_COASTLINE_COMMAND) == 0)
	{
	  configuration.operations |= GADM_CUT_COASTLINE;
	}
      else if (strcmp (command, GADM_SPLIT_POLYGONS_COMMAND) == 0)
	{
	  configuration.operations |= GADM_SPLIT_POLYGONS;
	}
      else if (strcmp (command, GADM_INFO_COMMAND) == 0)
	{
	  configuration.operations |= GADM_INFO;
	}
      else
	{
	  printf ("Unknown command %s\n", command);
	  return EXIT_FAILURE;
	}
      i++;
    }

  if (database_connect ())
    {
      printf ("Database connection established\n");
    }

  initGEOS (notice, log_and_exit);
  printf ("GEOS version %s\n", GEOSversion ());

  // Relations procesing loop.
  GArray *relations = database_get_relations (configuration.country_iso);
  printf ("Countries: %u\n", relations->len);

  if ((configuration.operations & GADM_CREATE_TABLES) == GADM_CREATE_TABLES)
    {
      printf ("\nCreting database tables.\n");
      database_tables_create ();
      printf ("Database tables created.\n");
    }

  if ((configuration.operations & GADM_CREATE_LINES) == GADM_CREATE_LINES)
    {
      if (configuration.truncate)
	database_tables_lines_truncate ();

      printf ("\nExtracting lines.\n\n");
      printf ("\nCountry name, iso2, Outer lines, Outer lines merged, "
	      "Outer lines saved, Inner lines, Inner lines merged, "
	      "Inner lines saved\n");
      for (int i = 0; i < relations->len; i++)
	{
	  country_type *country = &g_array_index (relations, country_type, i);
          extract_lines (country);
	}
    }

  // Closing application.
  country_type_array_free (relations);
  finishGEOS ();
  PQfinish (dbconn);
  if (configuration.output_db_name != NULL)
    {
      PQfinish (dbconn_output);
    }

  return EXIT_SUCCESS;
}

void
extract_lines (country_type * country)
{

  GPtrArray *lines;

  unsigned int outer_lines, outer_lines_merged, outer_lines_saved,
    inner_lines, inner_lines_merged, inner_lines_saved;

  lines = database_get_admin_lines (dbconn, country->id, "outer");
  outer_lines = lines->len;

  lines = process_lines_merge (lines);
  outer_lines_merged = lines->len;

  lines = process_lines_validation (lines);
  database_save_country_lines (dbconn_output, *country, "outer", lines);

  outer_lines_saved = lines->len;
  utils_geom_array_free (lines);

  lines = database_get_admin_lines (dbconn, country->id, "inner");
  inner_lines = lines->len;

  lines = process_lines_merge (lines);
  inner_lines_merged = lines->len;

  lines = process_lines_validation (lines);
  database_save_country_lines (dbconn_output, *country, "inner", lines);

  inner_lines_saved = lines->len;
  utils_geom_array_free (lines);

  if (configuration.verbose)
    printf ("%s,%s,%u,%u,%u,%u,%u,%u\n", country->name, country->iso2, outer_lines,
	    outer_lines_merged, outer_lines_saved, inner_lines,
	    inner_lines_merged, inner_lines_saved);

}

void
process_boundaries (country_type country)
{
/*
  GPtrArray *outer_lines;
  GPtrArray *inner_lines;
  GPtrArray *outer_polygons;
  GPtrArray *inner_polygons;
  GPtrArray *polygons;

  printf ("\tOuter lines\n");
  outer_lines = database_get_admin_lines (country.id, "outer");
  printf ("\t\tLines: %u\n", outer_lines->len);
  outer_lines = process_lines_merge (outer_lines);
  outer_lines = process_lines_validation (outer_lines);
  printf ("\t\tLines merged: %u\n", outer_lines->len);
  outer_polygons = process_polygons_generate (outer_lines);
  printf ("\t\tPolygons generated: %u\n", outer_polygons->len);

  printf ("\tInner lines\n");
  inner_lines = database_get_admin_lines (country.id, "inner");
  printf ("\t\tLines: %u\n", inner_lines->len);
  inner_lines = process_lines_merge (inner_lines);
  printf ("\t\tLines merged: %u\n", inner_lines->len);
  inner_lines = process_lines_validation (inner_lines);
  inner_polygons = process_polygons_generate (inner_lines);
  printf ("\t\tPolygons generated: %u\n", inner_polygons->len);

  polygons = process_polygons_merge (outer_polygons, inner_polygons);
  utils_geom_array_free (inner_polygons);
  polygons = process_polygons_cut_coastile (polygons);

  database_save_country_polygons (country, polygons);
  printf ("\tPolygons written: %u\n", polygons->len);

  //utils_geom_array_free (outer_polygons);
  utils_geom_array_free (polygons);
*/
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
