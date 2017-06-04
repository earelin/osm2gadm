#include "process.h"
#include "utils.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <glib.h>
#include <geos_c.h>

extern char *tmp_folder_name;

GPtrArray *
process_lines_merge (GPtrArray * lines)
{
  GPtrArray *merged_lines = NULL;
  GPtrArray *lines_tmp = lines;

  do
    {
      if (merged_lines != NULL)
	utils_geom_array_free (merged_lines);

      merged_lines = lines_tmp;
      lines_tmp = g_ptr_array_new ();

      for (int i = 0; i < merged_lines->len; i++)
	{
	  gboolean merged = FALSE;
	  GEOSGeometry *line = g_ptr_array_index (merged_lines, i);

	  for (int j = 0; j < lines_tmp->len; j++)
	    {
	      GEOSGeometry *tmp_line = g_ptr_array_index (lines_tmp, j);
	      if (GEOSIntersects (line, tmp_line))
		{
		  GEOSGeometry *united_geom = GEOSUnion (line, tmp_line);
		  GEOSGeom_destroy (tmp_line);
		  GEOSGeometry *merged_geom = GEOSLineMerge (united_geom);
		  GEOSGeom_destroy (united_geom);
		  g_ptr_array_remove_index (lines_tmp, j);
		  g_ptr_array_add (lines_tmp, merged_geom);
		  merged = TRUE;
		  break;
		}
	    }

	  if (!merged)
	    g_ptr_array_add (lines_tmp, GEOSGeom_clone (line));
	}
    }
  while (merged_lines->len != lines_tmp->len);

  utils_geom_array_free (merged_lines);

  return lines_tmp;
}

GPtrArray *
process_lines_validation (GPtrArray * lines)
{
  GPtrArray *processed = g_ptr_array_sized_new (lines->len);

  for (int i = 0; i < lines->len; i++)
    {
      GEOSGeometry *line = g_ptr_array_index (lines, i);
      int geom_type = GEOSGeomTypeId (line);


      if (geom_type != GEOS_MULTILINESTRING && geom_type != GEOS_LINESTRING)
	{
	  printf ("%s\n", GEOSGeomType (line));
	}

/*
          int collection_size = GEOSGetNumGeometries(line);
          for (int j = 0; j < collection_size; j++)
            {
                GEOSGeometry *splitted_line = GEOSGetGeometryN(line, i);
                g_ptr_array_add (processed, splitted_line);
            }
          GEOSGeom_destroy (line);
*/
      else
	{
	  if (geom_type == GEOS_MULTILINESTRING)
	    {

	    }
	  else
	    g_ptr_array_add (processed, line);

	}
    }

  g_ptr_array_free (lines, TRUE);

  return processed;
}

GPtrArray *
process_polygons_validation (GPtrArray * polygons)
{
  GPtrArray *processed = g_ptr_array_sized_new (polygons->len);
  for (int i = 0; i < polygons->len; i++)
    {
      GEOSGeometry *polygon = g_ptr_array_index (polygons, i);

      int geom_type = GEOSGeomTypeId (polygon);
      if (geom_type == GEOS_POLYGON)
	{
	  g_ptr_array_add (processed, polygon);
	}
      else
	{
	  if (geom_type == GEOS_GEOMETRYCOLLECTION
	      || geom_type == GEOS_MULTIPOLYGON)
	    {
	      int collection_size = GEOSGetNumGeometries (polygon);
	      GPtrArray *collection_polygons =
		g_ptr_array_sized_new (collection_size);
	      for (int j = 0; j < collection_size; j++)
		{
		  GEOSGeometry *polygon_item =
		    GEOSGeom_clone (GEOSGetGeometryN (polygon, j));
		  g_ptr_array_add (collection_polygons, polygon_item);
		}

	      collection_polygons =
		process_polygons_validation (collection_polygons);

	      for (int j = 0; j < collection_polygons->len; j++)
		{
		  g_ptr_array_add (processed,
				   g_ptr_array_index (collection_polygons,
						      j));
		}

	      g_ptr_array_free (collection_polygons, TRUE);
	    }
	  else
	    {
	      printf ("%s\n", GEOSGeomType (polygon));
	    }
	  GEOSGeom_destroy (polygon);
	}
    }

  g_ptr_array_free (polygons, TRUE);

  return processed;
}

GPtrArray *
process_polygons_generate (GPtrArray * lines)
{
  GPtrArray *polygons = g_ptr_array_sized_new (lines->len);
  for (int i = 0; i < lines->len; i++)
    {
      GEOSGeometry *line = g_ptr_array_index (lines, i);
      GEOSGeometry *polygon = GEOSPolygonize_full (line, NULL, NULL, NULL);
      GEOSGeom_destroy (line);
      g_ptr_array_add (polygons, polygon);
    }
  return polygons;
}

GPtrArray *
process_polygons_merge (GPtrArray * outer_polygons,
			GPtrArray * inner_polygons)
{
  GPtrArray *polygons = g_ptr_array_sized_new (outer_polygons->len);

  for (int i = 0; i < outer_polygons->len; i++)
    {
      GEOSGeometry *outer_polygon = g_ptr_array_index (outer_polygons, i);

/*
      for (int j = 0; j < inner_polygons->len; j++)
	{
	  GEOSGeometry *inner_polygon = g_ptr_array_index (inner_polygons, j);
	  
	}
*/

      g_ptr_array_add (polygons, outer_polygon);
    }

  return polygons;
}

void
process_polygon_get_bounds (GEOSGeometry * polygon, int *max_x, int *min_x,
			    int *max_y, int *min_y)
{
  GEOSSetSRID (polygon, GADM_WGS_84_SRID);
  GEOSGeometry *envelope = GEOSEnvelope (polygon);
  GEOSCoordSequence *envelope_seq =
    GEOSGeom_getCoordSeq (GEOSGetExteriorRing (envelope));

  GEOSCoordSeq_getX (envelope_seq, 0, min_x);
  GEOSCoordSeq_getY (envelope_seq, 0, min_y);
  GEOSCoordSeq_getY (envelope_seq, 1, max_y);
  GEOSCoordSeq_getX (envelope_seq, 2, max_x);
}

GEOSGeometry *
process_line_close (GEOSGeometry * line)
{
  GEOSGeometry *closed_line = NULL;
  GEOSGeometry *start_point = GEOSGeomGetStartPoint (line);
  GEOSGeometry *end_point = GEOSGeomGetEndPoint (line);

  double start_point_x, start_point_y, end_point_x, end_point_y;

  GEOSGeomGetX (start_point, &start_point_x);
  GEOSGeomGetY (start_point, &start_point_y);
  GEOSGeomGetX (end_point, &end_point_x);
  GEOSGeomGetY (end_point, &end_point_y);

  if (abs (start_point_x - end_point_x) > GADM_MAX_PRECISSION
      || abs (start_point_y - end_point_y) > GADM_MAX_PRECISSION)
    {
      printf ("%f %f - %f %f\n", start_point_x, start_point_y, end_point_x,
	      end_point_y);
    }

  unsigned int line_seq_size;
  GEOSCoordSequence *line_seq = GEOSGeom_getCoordSeq (line);
  GEOSCoordSeq_getSize (line_seq, &line_seq_size);
  GEOSCoordSequence *closed_seq = GEOSCoordSeq_create (line_seq_size + 1, 2);

  for (unsigned int i = 0; i < line_seq_size; i++)
    {
      double point_x, point_y;
      GEOSCoordSeq_getX (line_seq, i, &point_x);
      GEOSCoordSeq_getY (line_seq, i, &point_y);
      GEOSCoordSeq_setX (closed_seq, i, point_x);
      GEOSCoordSeq_setY (closed_seq, i, point_y);
    }

  GEOSCoordSeq_setX (closed_seq, line_seq_size, start_point_x);
  GEOSCoordSeq_setY (closed_seq, line_seq_size, start_point_y);

  closed_line = GEOSGeom_createLineString (closed_seq);

  GEOSCoordSeq_destroy (line_seq);
  GEOSCoordSeq_destroy (closed_seq);
  GEOSGeom_destroy (start_point);
  GEOSGeom_destroy (end_point);

  return closed_line;
}

GPtrArray *
process_polygons_cut_coastile (GPtrArray * polygons)
{
  GPtrArray *cutted_polygons = g_ptr_array_sized_new (polygons->len);

  for (int i = 0; i < polygons->len; i++)
    {
      GEOSGeometry *polygon = g_ptr_array_index (polygons, i);
      GEOSSetSRID (polygon, GADM_WGS_84_SRID);
      GPtrArray *water_polygons = database_get_water_polygons (polygon);
      //water_polygons = process_polygons_validation (water_polygons);

      for (int j = 0; j < water_polygons->len; j++)
	{
	  GEOSGeometry *water_polygon = g_ptr_array_index (water_polygons, j);
	  GEOSGeometry *tmp = GEOSDifference (polygon, water_polygon);
	  GEOSGeom_destroy (polygon);
	  polygon = tmp;
	}

      utils_geom_array_free (water_polygons);
      g_ptr_array_add (cutted_polygons, polygon);
    }

  g_ptr_array_free (polygons, TRUE);
  cutted_polygons = process_polygons_validation (cutted_polygons);

  return cutted_polygons;
}
