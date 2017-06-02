#include "database.h"
#include "datatypes.h"
#include <stdlib.h>
#include <glib.h>
#include <libpq-fe.h>
#include <geos_c.h>

extern PGconn *dbconn;
extern char *tmp_folder_name;

GArray* database_get_relations (void)
{   
  PGresult *result = PQexec (dbconn, GADM_DB_LOAD_RELATIONS);
  int resultCount = PQntuples (result);
  printf ("Relations: %d\n", resultCount);
  
  GArray* relations = g_array_sized_new (FALSE, FALSE, sizeof (country_type), resultCount);
  
  for (int row = 0; row < resultCount; ++row) {
    country_type country;
    country.id = atoi (PQgetvalue (result, row, 0));
    country.name = g_strdup (PQgetvalue (result, row, 1));
    country.iso2 = g_strdup (PQgetvalue (result, row, 2));
    g_array_append_val (relations, country);
  }

  PQclear (result);

  return relations;
}

void database_get_admin_lines (const int relation_id, const char *type) {    
  char *sql = g_strdup_printf(GADM_DB_LOAD_ADMIN_LINES, relation_id, type);
  PGresult *result = PQexec (dbconn, sql);
  int result_count = PQntuples (result);    
  printf("\t\t%s: %d\n", type, result_count);

  char *lines_file_name = g_strdup_printf("%s/%s_lines", tmp_folder_name, type);
  FILE *lines_file = fopen(lines_file_name, "w+");

  for (int row = 0; row < result_count; ++row) {
    fprintf(lines_file, "%s\n", PQgetvalue (result, row, 0));
  }

  free(lines_file_name);
  fclose(lines_file);
  PQclear(result);
  free(sql);
}