#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <glib.h>
#include <geos_c.h>

GPtrArray *
utils_geom_array_clone (GPtrArray * geoms)
{
  GPtrArray *copy = g_ptr_array_new ();
  for (int i = 0; i < geoms->len; i++)
    {
      GEOSGeometry *geom = GEOSGeom_clone (g_ptr_array_index (geoms, i));
      g_ptr_array_add (copy, geom);
    }
  return copy;
}

void
utils_geom_array_free (GPtrArray * geoms)
{
  for (int i = 0; i < geoms->len; i++)
    {
      GEOSGeometry *geom = g_ptr_array_index (geoms, i);
      if (geom != NULL)
	{
	  GEOSGeom_destroy (geom);
	}
    }
  g_ptr_array_free (geoms, TRUE);
}
