/* 
 * File:   database.h
 * Author: xavier
 *
 * Created on 02 June 2017, 07:36
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "datatypes.h"
#include <glib.h>
#include <geos_c.h>

#define GADM_DB_TRUNCATE "TRUNCATE osm2gadm_polygons"

#define GADM_DB_LOAD_RELATIONS "SELECT id, tags -> 'name:en' AS name, tags -> 'ISO3166-1:alpha2' as iso2 " \
        "FROM relations r " \
        "WHERE tags -> 'boundary' = 'administrative' " \
                "AND tags -> 'admin_level' = '2' " \
                "AND tags ? 'ISO3166-1:alpha2' " \
                /* "AND id IN (365307, 1993208, 555017, 36989, 305099, 536773, 50046, 295480) " */ " " \
"  ORDER BY name ASC"
#define GADM_DB_LOAD_ADMIN_LINES "SELECT ST_AsText(linestring) " \
	"FROM relation_members rm " \
	"JOIN ways AS w ON rm.member_id = w.id " \
	"WHERE relation_id = %d AND member_role = '%s' " \
	"ORDER BY sequence_id ASC"
#define GADM_DB_INSERT_POLYGONS "INSERT INTO osm2gadm_polygons(relation_id, name, iso2, max_x, min_x, max_y, min_y, geom) " \
	"VALUES (%d, '%s', '%s', %f, %f, %f, %f, ST_GeomFromText('%s', 4326))"
#define GADM_DB_INSERT_LINES "INSERT INTO osm2gadm_lines(relation_id, type, geom) " \
	"VALUES (%d, '%s', ST_GeomFromText('%s', 4326))"
#define GADM_DB_LOAD_WATER_POLYGONS "SELECT ST_AsText(p.geom) AS geom " \
        "FROM osm2gadm_water_polygons_envelope e " \
        "JOIN water_polygons p ON e.gid = p.gid " \
        "WHERE ((e.max_x < %f AND e.max_x > %f) OR (e.min_x < %f AND e.min_x > %f)) " \
        "  AND ((e.max_y < %f AND e.max_y > %f) OR (e.min_y < %f AND e.min_y > %f)) "
#define GADM_DB_LOAD_WATER_POLYGONS_1 "SELECT ST_AsText(geom) AS geom " \
        "FROM water_polygons p " \
        "WHERE ST_Intersects (geom, ST_GeomFromText ('%s', 4326))"
#define GADM_WGS_84_SRID 4326

#ifdef __cplusplus
extern "C"
{
#endif

  GArray *database_get_relations (void);
  GPtrArray *database_get_admin_lines (const int relation_id,
				       const char *type);
  void database_save_country_polygons (country_type country,
				       GPtrArray * polygons);
  void database_tables_create (void);
  void database_tables_truncate (void);
  GPtrArray *database_get_water_polygons (GEOSGeometry * polygon);
#ifdef __cplusplus
}
#endif

#endif				/* DATABASE_H */
