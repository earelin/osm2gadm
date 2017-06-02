/* 
 * File:   utils.h
 * Author: xavier
 *
 * Created on 02 June 2017, 22:20
 */

#ifndef UTILS_H
#define UTILS_H

#define UTILS_BUFFER_SIZE 1024 * 1024

#ifdef __cplusplus
extern "C" {
#endif

void utils_file_append_line (const char *filename, const char *line);
void utils_copy_file (const char *original, const char *copy);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

