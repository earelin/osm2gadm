/* 
 * File:   process.h
 * Author: xavier
 *
 * Created on 02 June 2017, 21:34
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <glib.h>
#include <geos_c.h>

#define GADM_MAX_PRECISSION 0.0000001

#ifdef __cplusplus
extern "C"
{
#endif

  GPtrArray *process_lines_merge (GPtrArray * lines);
  GPtrArray *process_lines_validation (GPtrArray * lines);
  GPtrArray *process_polygons_generate (GPtrArray * lines);
  GPtrArray *process_polygons_merge (GPtrArray * outer_polygons,
				     GPtrArray * inner_polygons);
  GEOSGeometry *process_line_close (GEOSGeometry * linestring);
  GPtrArray *process_polygons_validation (GPtrArray * polygons);
  GPtrArray *process_polygons_cut_coastile (GPtrArray * polygons);

#ifdef __cplusplus
}
#endif

#endif				/* PROCESS_H */
