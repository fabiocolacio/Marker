#include "em.h"
#include <stdio.h>
#include <stdlib.h>
#include "string_dup.h"
#include "deps/uthash/utstring.h"

float mtex2MML_extract_number_from_pxstring(char * str)
{
  float dbl;
  int match = 0;

  match = sscanf (str, "%*[^-0123456789]%f", &dbl);
  if (match == 1) {
    return dbl;
  }

  /* must not be a float */
  sscanf (str, "%d", &match);
  return (float) match;
}

char *mtex2MML_extract_string_from_pxstring(char * str)
{
  char *pixel;
  float dbl;
  pixel = malloc(3*sizeof(char));
  sscanf (str, "%f%s", &dbl, pixel);
  return pixel;
}

char *mtex2MML_dbl2em(char *str)
{
  float dbl;
  char *em_str;
  UT_string *em;
  utstring_new(em);

  dbl = mtex2MML_extract_number_from_pxstring(str);
  dbl *= 0.056f;

  utstring_printf(em, "%.3fem", dbl);
  em_str = string_dup(utstring_body(em));

  utstring_free(em);

  return em_str;
}

char *mtex2MML_double_pixel(float f, char *pixel)
{
  float dbl;
  char *em_str;
  UT_string *em;
  utstring_new(em);

  dbl = f * 2;
  utstring_printf(em, "%.3f%s", dbl, pixel);
  em_str = string_dup(utstring_body(em));

  utstring_free(em);

  return em_str;
}

char *mtex2MML_implement_skew(char *base_str, char *em_skew, char *pattern)
{
  char *skew_mathml_str;
  UT_string *skew_mathml;
  utstring_new(skew_mathml);

  utstring_printf(skew_mathml, "%s%s%s", "<mrow><mrow><mrow><mover><mrow>", base_str, "<mspace width=\"");
  utstring_printf(skew_mathml, "%s%s%s", em_skew, "\" /></mrow>", "<mo stretchy=\"false\">");
  utstring_printf(skew_mathml, "%s%s%s", pattern, "</mo></mover></mrow><mspace width=\"-", em_skew);
  utstring_printf(skew_mathml, "%s", "\" /></mrow><mrow></mrow></mrow>");

  skew_mathml_str = string_dup(utstring_body(skew_mathml));

  utstring_free(skew_mathml);

  return skew_mathml_str;
}

char *mtex2MML_root_pos_to_em(char * str)
{
  float dbl;
  char *em_str;
  UT_string *em;
  utstring_new(em);

  dbl = mtex2MML_extract_number_from_pxstring(str);
  dbl /= 15;

  utstring_printf(em, "%.3fem", dbl);
  em_str = string_dup(utstring_body(em));

  utstring_free(em);

  return em_str;
}
