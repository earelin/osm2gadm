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

#define TESTING_COUNTRY 49898

void notice (const char *fmt, ...);
void log_and_exit (const char *fmt, ...);
void process_boundaries (const int relation_id);

PGconn *dbconn;
char *tmp_folder_name;

const char *TMP_FOLDER_NAME_TEMPLATE = "osm2gadm-XXXXXX";

/*
 * 
 */
int main (int argc, char** argv) {    
  if (argc < 2) {
      printf ("You must enter a connection string as first parameter.\n");
      return EXIT_FAILURE;	
  }

  dbconn = PQconnectdb (argv[1]);

  if (PQstatus (dbconn) != CONNECTION_OK) {
     printf ("Can't open database\n");
     return EXIT_FAILURE;
  }
  printf ("Opened database successfully: %s\n", PQdb (dbconn));

  initGEOS (notice, log_and_exit);
  printf ("GEOS version %s\n", GEOSversion ());

  tmp_folder_name = tempnam (NULL, TMP_FOLDER_NAME_TEMPLATE);
  g_mkdir(tmp_folder_name, S_IRWXU);
  
  // Relations procesing loop.
  GArray *relations = database_get_relations ();

  for (int i = 0; i < 5; i++) { //relations->len; i++) {
    country_type country = g_array_index (relations, country_type, i);
    printf ("%s (%s %d)\n", country.name, country.iso2, country.id);
    process_boundaries (country.id);
  }
  
  // Closing application.
  country_type_array_free(relations);
  //g_remove (tmp_folder_name);
  free(tmp_folder_name);
  finishGEOS ();
  PQfinish (dbconn);  

  return EXIT_SUCCESS;
}

void process_boundaries (const int relation_id)
{
  printf ("\tLoading lines\n");
  database_get_admin_lines(relation_id, "outer");
  database_get_admin_lines(relation_id, "inner");
  printf ("\tMerging lines\n");
  process_merge_lines("outer");
  process_merge_lines("inner");
}

void notice (const char *fmt, ...) {
  va_list ap;

  fprintf (stdout, "\nNOTICE: ");

  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  va_end (ap);
  fprintf (stdout, "\n");
}

void log_and_exit (const char *fmt, ...) {
  va_list ap;

  fprintf (stdout, "\nERROR: ");

  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  va_end (ap);
  fprintf (stdout, "\n" );
  exit (1);
}
