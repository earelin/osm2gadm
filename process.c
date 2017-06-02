#include "process.h"
#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <glib.h>
#include <geos_c.h>

extern char *tmp_folder_name;

void process_merge_lines (const char *type)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  
  char *lines_file_name = g_strdup_printf ("%s/%s_lines", tmp_folder_name, type);
  char *merged_lines_file_name = g_strdup_printf ("%s/%s_merged_lines", tmp_folder_name, type);
  
  utils_copy_file (lines_file_name, merged_lines_file_name);  
  
  FILE *merged_lines_file = fopen (merged_lines_file_name, "r");
  
  GPtrArray *merged_lines = g_ptr_array_new ();
  while ((nread = getline (&line, &len, merged_lines_file)) != -1) {    
    gboolean merged = FALSE;    
    GEOSGeometry *geom = GEOSGeomFromWKT (line);

    if (!merged) {
      g_ptr_array_add (merged_lines, geom);
    }
  }  
  fclose (merged_lines_file);
  
  merged_lines_file = fopen (merged_lines_file_name, "w");
  for (int i = 0; i < merged_lines->len; i++) {
    GEOSGeometry *geom = g_ptr_array_index (merged_lines, i);
    char *wkt = GEOSGeomToWKT (geom);
    GEOSGeom_destroy (geom);
    fprintf (merged_lines_file, "%s\n", wkt);
    g_free (wkt);
  }
  fclose (merged_lines_file);
  
  g_free (line);
  g_free (lines_file_name);  
  g_free (merged_lines_file_name);
}
