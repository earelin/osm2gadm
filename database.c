#include "database.h"
#include "datatypes.h"
#include "process.h"
#include <stdlib.h>
#include <glib.h>
#include <libpq-fe.h>
#include <geos_c.h>

extern PGconn *dbconn;
extern PGconn *dbconn_output;
extern app_configuration configuration;

int
database_connect (void)
{
  char *connection_string = "";
  char *ouput_connection_string = NULL;

  if (configuration.db_host != NULL)
    {
      connection_string =
	g_strdup_printf ("host=%s %s", configuration.db_host,
			 connection_string);
    }
  if (configuration.db_username != NULL)
    {
      connection_string =
	g_strdup_printf ("user=%s %s", configuration.db_username,
			 connection_string);
    }
  if (configuration.db_password != NULL)
    {
      connection_string =
	g_strdup_printf ("password=%s %s", configuration.db_password,
			 connection_string);
    }
  if (configuration.output_db_name != NULL)
    {
      ouput_connection_string =
	g_strdup_printf ("dbname=%s %s", configuration.output_db_name,
			 connection_string);
    }
  if (configuration.db_name != NULL)
    {
      connection_string =
	g_strdup_printf ("dbname=%s %s", configuration.db_name,
			 connection_string);
    }

  dbconn = PQconnectdb (connection_string);
  g_free (connection_string);
  if (PQstatus (dbconn) != CONNECTION_OK)
    return 0;


  if (configuration.output_db_name != NULL)
    {
      dbconn_output = PQconnectdb (ouput_connection_string);
      g_free (ouput_connection_string);
      if (PQstatus (dbconn_output) != CONNECTION_OK)
	return 0;
    }
  else
    dbconn_output = dbconn;

  return 1;
}

PGconn *
database_connection_new (char *database)
{
  PGconn *connection;

  char *connection_string = "";

  if (configuration.db_host != NULL)
    {
      connection_string =
	g_strdup_printf ("host=%s %s", configuration.db_host,
			 connection_string);
    }
  if (configuration.db_username != NULL)
    {
      connection_string =
	g_strdup_printf ("user=%s %s", configuration.db_username,
			 connection_string);
    }
  if (configuration.db_password != NULL)
    {
      connection_string =
	g_strdup_printf ("password=%s %s", configuration.db_password,
			 connection_string);
    }
  if (database != NULL)
    {
      connection_string =
	g_strdup_printf ("dbname=%s %s", database, connection_string);
    }

  connection = PQconnectdb (connection_string);
  g_free (connection_string);

  return connection;
}

GArray *
database_get_relations (char *country_iso)
{
  char *sql;

  if (country_iso == NULL)
    sql = g_strdup (GADM_DB_LOAD_RELATIONS);
  else
    sql = g_strdup_printf (GADM_DB_LOAD_COUNTRY_RELATIONS, country_iso);

  PGresult *result = PQexec (dbconn, sql);
  int resultCount = PQntuples (result);

  GArray *relations =
    g_array_sized_new (FALSE, FALSE, sizeof (country_type), resultCount);

  for (int row = 0; row < resultCount; ++row)
    {
      country_type country;
      country.id = atoi (PQgetvalue (result, row, 0));
      country.name = g_strdup (PQgetvalue (result, row, 1));
      country.iso2 = g_strdup (PQgetvalue (result, row, 2));
      g_array_append_val (relations, country);
    }

  g_free (sql);
  PQclear (result);

  return relations;
}

void
database_save_country_lines (PGconn * connection,
			     country_type country,
			     const char *type, GPtrArray * lines)
{
  for (int i = 0; i < lines->len; i++)
    {
      GEOSGeometry *line = g_ptr_array_index (lines, i);
      char *wkt_geom = GEOSGeomToWKT (line);
      char *sql =
	g_strdup_printf (GADM_DB_INSERT_LINES, country.name, country.iso2,
			 country.id, type, wkt_geom);
      free (wkt_geom);
      PGresult *result = PQexec (connection, sql);
      PQclear (result);
      free (sql);
    }

}

GPtrArray *
database_get_water_polygons (PGconn * connection, GEOSGeometry * polygon)
{
  char *wkt_geom = GEOSGeomToWKT (polygon);
  char *sql = g_strdup_printf (GADM_DB_LOAD_WATER_POLYGONS, wkt_geom);
  PGresult *result = PQexec (connection, sql);
  int result_count = PQntuples (result);

  GPtrArray *water_polygons = g_ptr_array_sized_new (result_count);

  for (int row = 0; row < result_count; row++)
    {
      GEOSGeometry *geom = GEOSGeomFromWKT (PQgetvalue (result, row, 0));
      g_ptr_array_add (water_polygons, geom);
    }

  PQclear (result);
  free (sql);

  return water_polygons;
}

GPtrArray *
database_get_admin_lines (PGconn * connection, const int relation_id,
			  const char *type)
{
  char *sql = g_strdup_printf (GADM_DB_LOAD_ADMIN_LINES, relation_id, type);
  PGresult *result = PQexec (connection, sql);
  int result_count = PQntuples (result);

  GPtrArray *lines = g_ptr_array_sized_new (result_count);
  for (int row = 0; row < result_count; ++row)
    {
      GEOSGeometry *geom = GEOSGeomFromWKT (PQgetvalue (result, row, 0));
      g_ptr_array_add (lines, geom);
    }

  PQclear (result);
  free (sql);

  return lines;
}

void
database_save_country_polygons (PGconn * connection, country_type country,
				GPtrArray * polygons)
{
  for (int i = 0; i < polygons->len; i++)
    {
      GEOSGeometry *polygon = g_ptr_array_index (polygons, i);
      GEOSSetSRID (polygon, GADM_WGS_84_SRID);
      double max_x, min_x, max_y, min_y;
      process_polygon_get_bounds (polygon, &max_x, &min_x, &max_y, &min_y);

      char *wkt_geom = GEOSGeomToWKT (polygon);
      char *sql =
	g_strdup_printf (GADM_DB_INSERT_POLYGONS, country.id, country.name,
			 country.iso2, max_x, min_x, max_y, min_y, wkt_geom);
      free (wkt_geom);
      PGresult *result = PQexec (connection, sql);
      PQclear (result);
      free (sql);
    }
}

void
database_tables_create (void)
{
  gsize size = 0;
  GBytes *data =
    g_resources_lookup_data ("/osm2gadm/create_tables.sql", 0, NULL);
  char *sql = g_bytes_get_data (data, size);
  PGresult *result = PQexec (dbconn_output, sql);
  PQclear (result);
  free (sql);
}

void
database_tables_truncate (void)
{
  gchar *sql = "TRUNCATE TABLE osm2gadm_polygons;"
    "TRUNCATE TABLE osm2gadm_lines;";
  PGresult *result = PQexec (dbconn_output, sql);
  PQclear (result);
}

void
database_tables_lines_truncate (void)
{
  gchar *sql = "TRUNCATE TABLE osm2gadm_lines;";
  PGresult *result = PQexec (dbconn_output, sql);
  PQclear (result);
}

void
database_tables_polygons_truncate (void)
{
  gchar *sql = "TRUNCATE TABLE osm2gadm_polygons;";
  PGresult *result = PQexec (dbconn_output, sql);
  PQclear (result);
}
