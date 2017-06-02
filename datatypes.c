#include "datatypes.h"
#include <stdlib.h>

void country_type_array_free (GArray *countries) {
  for (int i = 0; i < countries->len; i++) {
    country_type country = g_array_index(countries, country_type, i);
    free(country.name);
    free(country.iso2);
  }
  g_array_free(countries, TRUE);
}
