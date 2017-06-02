#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <glib.h>

void utils_copy_file (const char *original, const char *copy)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  
  FILE *original_file = fopen (original, "r");  
  FILE *copy_file = fopen (copy, "w+");
  
  while ((nread = getline (&line, &len, original_file)) != -1) {    
    fwrite (line, nread, 1, copy_file);
  }
  
  g_free(line);
  fclose (copy_file);
  fclose (original_file);   
}

void utils_file_append_line (const char *filename, const char *line)
{
  FILE *file = fopen (filename, "a");
  fwrite (line, strlen(line), 1, file);
  fclose(file);
}
