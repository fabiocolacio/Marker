#ifndef COLORS_H
#define COLORS_H

#include "deps/uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

struct css_colors {
  char name[22];             /* key */
  char color[10];
  UT_hash_handle hh;         /* makes this structure hashable */
};

/* Assemble a mapping of color names */
void mtex2MML_create_css_colors(struct css_colors **colors);

#ifdef __cplusplus
}
#endif

#endif /* ! COLORS_H */
