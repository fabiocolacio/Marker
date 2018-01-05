#ifndef MTEX2MML_H
#define MTEX2MML_H

#define MTEX2MML_VERSION "1.3.1"

#ifdef __cplusplus
extern "C" {
#endif

#define mtex2MML_CAPTURE 1

#include "environment.h"

/* Step 1. Parse a buffer with mtex source; return value is mathml, or 0 on failure (e.g., parse error).
 */
extern char * mtex2MML_parse (const char * buffer, unsigned long length, const int options);

/* Step 2. Free the string from Step 1.
 */
extern void   mtex2MML_free_string (char * str);


/* Alternatively, to filter generic source and converting embedded equations, use:
 */
extern int    mtex2MML_filter (const char * buffer, unsigned long length, const int options);
extern int    mtex2MML_text_filter (const char * buffer, unsigned long length, const int options);
extern int    mtex2MML_strict_filter (const char * buffer, unsigned long length, const int options);

extern int mtex2MML_do_filter (const char * buffer, unsigned long length, const int forbid_markup, const int write, const int options);

/** Types of delimiters
 */
#define MTEX2MML_DELIMITER_DEFAULT          0
#define MTEX2MML_DELIMITER_DOLLAR           1
#define MTEX2MML_DELIMITER_DOUBLE           2
#define MTEX2MML_DELIMITER_PARENS           4
#define MTEX2MML_DELIMITER_BRACKETS         8
#define MTEX2MML_DELIMITER_ENVIRONMENTS     16

/* To change output methods:
 *
 * Note: If length is 0, then buffer is treated like a string; otherwise only length bytes are written.
 */
extern void (*mtex2MML_write) (const char * buffer, unsigned long length); /* default writes to stdout */
extern void (*mtex2MML_write_mathml) (const char * mathml);                /* default calls mtex2MML_write(mathml,0) */
extern void (*mtex2MML_error) (const char * msg);                          /* default writes to stderr */


/* Other stuff:
 */

extern char * mtex2MML_global_parse (const char * buffer, unsigned long length, const int options, const int global_start);

extern int mtex2MML_delimiter_type();

extern void   mtex2MML_setup (const char * buffer, unsigned long length);

extern void   mtex2MML_restart ();
extern void   mtex2MML_reset_parsing_environment ();

extern int mtex2MML_do_capture_eqn();
extern int mtex2MML_capture_eqn_src();

extern char * mtex2MML_output ();

extern char * mtex2MML_copy_string (const char * str);
extern char * mtex2MML_copy_string_extra (const char * str, unsigned extra);
extern char * mtex2MML_copy2 (const char * first, const char * second);
extern char * mtex2MML_copy3 (const char * first, const char * second, const char * third);
extern char * mtex2MML_copy_escaped (const char * str);
extern  UT_array ** mtex2MML_get_environment_data_stack();

extern char * mtex2MML_empty_string;

extern int    mtex2MML_lineno;

extern int    mtex2MML_rowposn;
extern int    mtex2MML_displaymode;
extern int    display_style;

extern int    delimiter_options;
extern int    global_label;
extern int    line_counter;

#ifdef __cplusplus
}
#endif

#endif /* ! MTEX2MML_H */
