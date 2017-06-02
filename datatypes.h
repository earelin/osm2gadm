/* 
 * File:   datatypes.h
 * Author: xavier
 *
 * Created on 02 June 2017, 18:26
 */

#ifndef DATATYPES_H
#define DATATYPES_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _country_type {
  int id;
  char *name;
  char *iso2;
} country_type;

void country_type_array_free (GArray *countries);

#ifdef __cplusplus
}
#endif

#endif /* DATATYPES_H */

