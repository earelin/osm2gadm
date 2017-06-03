/* 
 * File:   utils.h
 * Author: xavier
 *
 * Created on 02 June 2017, 22:20
 */

#ifndef UTILS_H
#define UTILS_H

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  GPtrArray *utils_geom_array_clone (GPtrArray * geoms);
  void utils_geom_array_free (GPtrArray * geoms);

#ifdef __cplusplus
}
#endif

#endif				/* UTILS_H */
