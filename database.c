#include "database.h"
#include "datatypes.h"
#include "process.h"
#include <stdlib.h>
#include <glib.h>
#include <libpq-fe.h>
#include <geos_c.h>

extern PGconn *dbconn;
extern char *tmp_folder_name;

GArray *
database_get_relations (void)
{
  PGresult *result = PQexec (dbconn, GADM_DB_LOAD_RELATIONS);
  int resultCount = PQntuples (result);
  printf ("Relations: %d\n", resultCount);

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

  PQclear (result);

  return relations;
}

GPtrArray *
database_get_water_polygons (GEOSGeometry * polygon)
{
  char *wkt_geom = GEOSGeomToWKT (polygon);
  char *sql =
    g_strdup_printf (GADM_DB_LOAD_WATER_POLYGONS, wkt_geom);
  PGresult *result = PQexec (dbconn, sql);
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
database_get_admin_lines (const int relation_id, const char *type)
{
  char *sql = g_strdup_printf (GADM_DB_LOAD_ADMIN_LINES, relation_id, type);
  PGresult *result = PQexec (dbconn, sql);
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
database_save_country_polygons (country_type country, GPtrArray * polygons)
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
      PGresult *result = PQexec (dbconn, sql);
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
  PGresult *result = PQexec (dbconn, sql);
  PQclear (result);
  free (sql);
}

void
database_tables_truncate (void)
{
  char *sql = "TRUNCATE TABLE osm2gadm_polygons;"
    "TRUNCATE TABLE osm2gadm_lines;";
  PGresult *result = PQexec (dbconn, sql);
  PQclear (result);
}
