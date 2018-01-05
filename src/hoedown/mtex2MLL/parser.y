%parse-param {char **ret_str}

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mtex2MML.h"
#include "colors.h"
#include "em.h"
#include "environment.h"
#include "string_extras.h"

#include "string_dup.h"
#include "deps/uthash/uthash.h"

struct css_colors *colors = NULL;

#define YYSTYPE char *
#define YYPARSE_PARAM_TYPE char **
#define YYPARSE_PARAM ret_str
/* set max nesting. utterly arbitrary number determined from http://git.io/FlWHfw */
#define YYMAXDEPTH 430

/*
#define YYDEBUG 1
yydebug = 1;
*/

#define YYERROR_VERBOSE 1

#define yytext mtex2MML_yytext

 UT_array *environment_data_stack;

 encaseType encase = NONE;

 extern int yylex ();

 extern char * yytext;

 int global_label = 1;
 int line_counter = 1;
 int delimiter_options = 0;

 static void mtex2MML_default_error (const char * msg)
 {
   if (msg) {
     fprintf(stderr, "Line: %d Error: %s\n", mtex2MML_lineno, msg);
   }
 }

 void (*mtex2MML_error) (const char * msg) = mtex2MML_default_error;

 static void yyerror (char **ret_str, const char * s)
 {
   char * msg = mtex2MML_copy3 (s, " at token ", yytext);
   if (mtex2MML_error) {
     (*mtex2MML_error) (msg);
   }
   mtex2MML_free_string (msg);
 }

 /* Note: If length is 0, then buffer is treated like a string; otherwise only length bytes are written.
  */

 static void mtex2MML_default_write (const char * buffer, unsigned long length)
 {
   if (buffer) {
     if (length) {
       fwrite (buffer, 1, length, stdout);
     } else {
       fputs (buffer, stdout);
     }
   }
 }

 static void mtex2MML_default_write_mathml (const char * mathml)
 {
   if (mtex2MML_write) {
     (*mtex2MML_write) (mathml, 0);
   }
 }

 #ifdef mtex2MML_CAPTURE
 static char * mtex2MML_output_string = "" ;

 char * mtex2MML_output ()
 {
   char * copy = (char *) malloc((mtex2MML_output_string ? strlen(mtex2MML_output_string) : 0) + 1);
   if (copy) {
     if (mtex2MML_output_string) {
       strcpy(copy, mtex2MML_output_string);
       if (*mtex2MML_output_string != '\0') {
         mtex2MML_free_string(mtex2MML_output_string);
       }
     } else {
       copy[0] = 0;
     }
     mtex2MML_output_string = "";
   }
   return copy;
 }

 static void mtex2MML_capture (const char * buffer, unsigned long length)
 {
   if (buffer) {
     if (length) {
       unsigned long first_length = mtex2MML_output_string ? strlen(mtex2MML_output_string) : 0;
       char * copy  = (char *) malloc(first_length + length + 1);
       if (copy) {
         if (mtex2MML_output_string) {
           strcpy(copy, mtex2MML_output_string);
           if (*mtex2MML_output_string != '\0') {
             mtex2MML_free_string(mtex2MML_output_string);
           }
         } else {
           copy[0] = 0;
         }
         strncat(copy, buffer, length);
         mtex2MML_output_string = copy;
       }
     } else {
       char * copy = mtex2MML_copy2(mtex2MML_output_string, buffer);
       if (*mtex2MML_output_string != '\0') {
         mtex2MML_free_string(mtex2MML_output_string);
       }
       mtex2MML_output_string = copy;
     }
   }
 }

 static void mtex2MML_capture_mathml (const char * buffer)
 {
   char * temp = mtex2MML_copy2(mtex2MML_output_string, buffer);
   if (*mtex2MML_output_string != '\0') {
     mtex2MML_free_string(mtex2MML_output_string);
   }
   mtex2MML_output_string = temp;
 }
 void (*mtex2MML_write) (const char * buffer, unsigned long length) = mtex2MML_capture;
 void (*mtex2MML_write_mathml) (const char * mathml) = mtex2MML_capture_mathml;
 #else
 void (*mtex2MML_write) (const char * buffer, unsigned long length) = mtex2MML_default_write;
 void (*mtex2MML_write_mathml) (const char * mathml) = mtex2MML_default_write_mathml;
 #endif

 char * mtex2MML_empty_string = (char *) "";

 /* Create a copy of a string, adding space for extra chars
  */
 char * mtex2MML_copy_string_extra (const char * str, unsigned extra)
 {
   char * copy = (char *) malloc(extra + (str ? strlen (str) : 0) + 1);
   if (copy) {
     if (str) {
       strcpy(copy, str);
     } else {
       copy[0] = 0;
     }
   }
   return copy ? copy : mtex2MML_empty_string;
 }

 /* Create a copy of a string, appending two strings
  */
 char * mtex2MML_copy3 (const char * first, const char * second, const char * third)
 {
   int  first_length =  first ? strlen( first) : 0;
   int second_length = second ? strlen(second) : 0;
   int  third_length =  third ? strlen( third) : 0;

   char * copy = (char *) malloc(first_length + second_length + third_length + 1);

   if (copy) {
     if (first) {
       strcpy(copy, first);
     } else {
       copy[0] = 0;
     }

     if (second) { strcat(copy, second); }
     if ( third) { strcat(copy,  third); }
   }
   return copy ? copy : mtex2MML_empty_string;
 }

 /* Create a copy of a string, appending a second string
  */
 char * mtex2MML_copy2 (const char * first, const char * second)
 {
   return mtex2MML_copy3(first, second, 0);
 }

 /* Create a copy of a string
  */
 char * mtex2MML_copy_string (const char * str)
 {
   return mtex2MML_copy3(str, 0, 0);
 }

 /* Create a copy of a string, escaping unsafe characters for XML
  */
 char * mtex2MML_copy_escaped (const char * str)
 {
   unsigned long length = 0;

   const char * ptr1 = str;

   char * ptr2 = 0;
   char * copy = 0;

   if ( str == 0) { return mtex2MML_empty_string; }
   if (*str == 0) { return mtex2MML_empty_string; }

   while (*ptr1) {
     switch (*ptr1) {
     case '<':  /* &lt;   */
     case '>':  /* &gt;   */
       length += 4;
       break;
     case '&':  /* &amp;  */
       length += 5;
       break;
     case '\'': /* &apos; */
     case '"':  /* &quot; */
     case '-':  /* &#x2d; */
       length += 6;
       break;
     default:
       length += 1;
       break;
     }
     ++ptr1;
   }

   copy = (char *) malloc (length + 1);

   if (copy) {
     ptr1 = str;
     ptr2 = copy;

     while (*ptr1) {
       switch (*ptr1) {
       case '<':
         strcpy (ptr2, "&lt;");
         ptr2 += 4;
         break;
       case '>':
         strcpy (ptr2, "&gt;");
         ptr2 += 4;
         break;
       case '&':  /* &amp;  */
         strcpy (ptr2, "&amp;");
         ptr2 += 5;
         break;
       case '\'': /* &apos; */
         strcpy (ptr2, "&apos;");
         ptr2 += 6;
         break;
       case '"':  /* &quot; */
         strcpy (ptr2, "&quot;");
         ptr2 += 6;
         break;
       case '-':  /* &#x2d; */
         strcpy (ptr2, "&#x2d;");
         ptr2 += 6;
         break;
       default:
         *ptr2++ = *ptr1;
         break;
       }
       ++ptr1;
     }
     *ptr2 = 0;
   }
   return copy ? copy : mtex2MML_empty_string;
 }

 /* Returns a string representation of the global_label, and increments the label */
 char * mtex2MML_global_label()
 {
   char * n = (char *) malloc(256);
   #ifdef _WIN32
   _snprintf(n, 256, "%d", global_label);
   #else
   snprintf(n, 256, "%d", global_label);
   #endif
   global_label++;
   char *prefix = mtex2MML_copy3("<mtd><mtext>(", n, ")</mtext></mtd>");
   mtex2MML_free_string(n);

   return prefix;
 }

 /* Create a hex character reference string corresponding to code
  */
 char * mtex2MML_character_reference (unsigned long int code)
 {
 #define ENTITY_LENGTH 10
   char * entity = (char *) malloc(ENTITY_LENGTH);
   sprintf(entity, "&#x%05lx;", code);
   return entity;
 }

 void mtex2MML_free_string (char * str)
 {
   if (str && str != mtex2MML_empty_string) {
     free(str);
   }
 }


%}

%left TEXOVER TEXOVERWITHDELIMS TEXATOP TEXATOPWITHDELIMS TEXABOVE TEXABOVEWITHDELIMS
%token CHAR STARTMATH STARTDMATH ENDMATH MTEXT MI MIB MN MO LIMITS NOLIMITS SUP SUB MROWOPEN MROWCLOSE MATHOPEN MATHCLOSE MATHORD MATHPUNCT VCENTER LEFT RIGHT BIG BBIG BIGG BBIGG BIGL BBIGL BIGGL BBIGGL BIGM BBIGM BIGGM BBIGGM FRAC TFRAC DFRAC CFRAC GENFRAC OPERATORNAME MATHOP MATHBIN MATHREL MATHINNER MOP MOL MOLL MOF MOR PERIODDELIM COMMADELIM OTHERDELIM LEFTDELIM RIGHTDELIM MOS MOB SQRT ROOT OF LEFTROOT UPROOT BINOM TBINOM BRACE BRACK CHOOSE DBINOM UNDER BUILDREL OVER OVERBRACE OVERBRACKET UNDERLINE UNDERBRACE UNDERBRACKET UNDEROVER TENSOR MULTI ALIGNATVALUE ARRAYALIGN COLUMNALIGN ARRAY SPACECUBE HSPACE MOVELEFT MOVERIGHT RAISE LOWER PXSTRING COLSEP COLSEPSPACE ROWSEP NEWLINE ARRAYOPTS COLLAYOUT COLALIGN ROWALIGN ALIGN EQROWS EQCOLS ROWLINES COLLINES FRAME PADDING ATTRLIST ITALICS SANS TT ENCLOSE ENCLOSENOTATION ENCLOSEATTR ENCLOSETEXT BOLD BOXED FBOX HBOX MBOX BCANCELED XCANCELED CANCELEDTO NOT SLASHED PMB SCR RM BB ST END BBLOWERCHAR BBUPPERCHAR BBDIGIT CALCHAR FRAKCHAR CAL FRAK CLAP LLAP RLAP ROWOPTS TEXTSIZE OLDSTYLE SCSTY SCSIZE SCSCSIZE TINY TTINY SMALL NORMALSIZE LARGE LLARGE LLLARGE HUGE HHUGE DISPLAY TEXTSTY TEXTBOX TEXTSTRING COLORSTRING STYLESTRING VERBBOX VERBSTRING ACUTE GRAVE BREVE MATHRING XMLSTRING CELLOPTS ROWSPAN COLSPAN THINSPACE ENSPACE MEDSPACE THICKSPACE QUAD QQUAD NEGSPACE NEGMEDSPACE NEGTHICKSPACE STRUT MATHSTRUT SMASH PHANTOM HPHANTOM VPHANTOM HREF UNKNOWNCHAR EMPTYMROW STATLINE TOOLTIP TOGGLE TOGGLESTART TOGGLEEND FGHIGHLIGHT BGHIGHLIGHT COLORBOX SPACE NUMBER INTONE INTTWO INTTHREE OVERLEFTARROW OVERLEFTRIGHTARROW OVERRIGHTARROW UNDERLEFTARROW UNDERLEFTRIGHTARROW UNDERRIGHTARROW BAR WIDEBAR SKEW VEC WIDEVEC HAT WIDEHAT CHECK WIDECHECK TILDE WIDETILDE DOT DDOT DDDOT DDDDOT UNARYMINUS UNARYPLUS BEGINENV ENDENV EQUATION EQUATION_STAR EQALIGN EQALIGNNO MATRIX PMATRIX BMATRIX BBMATRIX VMATRIX VVMATRIX SUBARRAY SVG ENDSVG SMALLMATRIX CASES ALIGNED ALIGNENV ALIGNENV_STAR ALIGNAT ALIGNAT_STAR ALIGNEDAT GATHERED EQNARRAY EQNARRAY_STAR MULTLINE MULTLINE_STAR GATHER_STAR GATHER SUBSTACK SIDESET BMOD PMOD POD RMCHAR SCRCHAR PMBCHAR COLOR BGCOLOR LABEL TAG BBOX XARROW OPTARGOPEN OPTARGCLOSE MTEXNUM RAISEBOX NEG LATEXSYMBOL TEXSYMBOL VARINJLIM VARLIMINF VARLIMSUP VARPROJLIM

%%

doc:  xmlmmlTermList {/* all processing done in body*/};

xmlmmlTermList:
{/* nothing - do nothing*/}
| char {/* proc done in body*/}
| expression {/* all proc. in body*/}
| xmlmmlTermList char {/* all proc. in body*/}
| xmlmmlTermList expression {/* all proc. in body*/};

char: CHAR { mtex2MML_free_string($1); /* Do nothing...but what did this used to do? printf("%s", $1); */ };

expression: STARTMATH ENDMATH {/* empty math group - ignore*/}
| STARTDMATH ENDMATH {/* ditto */}
| STARTMATH compoundTermList ENDMATH {
  char ** r = (char **) ret_str;
  char * p = mtex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='inline'><semantics><mrow>", $2, "</mrow><annotation encoding='application/x-tex'>");
  char * s = mtex2MML_copy3(p, $3, "</annotation></semantics></math>");
  mtex2MML_free_string(p);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  if (r) {
    (*r) = (s == mtex2MML_empty_string) ? 0 : s;
  }
  else {
    if (mtex2MML_write_mathml)
      (*mtex2MML_write_mathml) (s);
    mtex2MML_free_string(s);
  }
}
| STARTDMATH compoundTermList ENDMATH {
  char ** r = (char **) ret_str;
  char * p = mtex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'><semantics><mrow>", $2, "</mrow><annotation encoding='application/x-tex'>");
  char * s = mtex2MML_copy3(p, $3, "</annotation></semantics></math>");
  mtex2MML_free_string(p);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  if (r) {
    (*r) = (s == mtex2MML_empty_string) ? 0 : s;
  }
  else {
    if (mtex2MML_write_mathml)
      (*mtex2MML_write_mathml) (s);
    mtex2MML_free_string(s);
  }
}
| mathenv {
  char ** r = (char **) ret_str;
  char * p = mtex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'><semantics><mrow>", $1, "</mrow><annotation encoding='application/x-tex'>");
  char * s = mtex2MML_copy2(p, "</annotation></semantics></math>");
  mtex2MML_free_string(p);
  mtex2MML_free_string($1);
  if (r) {
    (*r) = (s == mtex2MML_empty_string) ? 0 : s;
  }
  else {
    if (mtex2MML_write_mathml)
      (*mtex2MML_write_mathml) (s);
    mtex2MML_free_string(s);
  }
}
/* plugs away some memory leaks when errors occur */
| compoundTermList error { mtex2MML_free_string($1); mtex2MML_free_string($2);  YYABORT; }
| compoundTerm error { mtex2MML_free_string($1); mtex2MML_free_string($2); YYABORT; }
| error { YYABORT; }; /* tosses away the final token so that future parses are not affected */

compoundTermList: compoundTerm {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| compoundTermList compoundTerm {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

compoundTerm: mob SUB closedTerm SUP closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munderover>", $1, " ");
    char * s2 = mtex2MML_copy3($3, " ", $5);
    $$ = mtex2MML_copy3(s1, s2, "</munderover>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  else {
    char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = mtex2MML_copy3($3, " ", $5);
    $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| MATHOP closedTerm SUB closedTerm SUP closedTerm {
  char * s1 = mtex2MML_copy3("<munderover><mrow>", $2, "</mrow>");
  char * s2 = mtex2MML_copy3(s1, $4, $6);
  $$ = mtex2MML_copy2(s2, "</munderover>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);

  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
}
| OPERATORNAME closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mi>", $2, "</mi>");
  $$ = mtex2MML_copy2(s1, $3);

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| OPERATORNAME closedTerm SUB closedTerm SUP closedTerm closedTerm {
  char *s1 = mtex2MML_copy3("<msubsup><mi>", $2, "</mi>");
  char *s2 = mtex2MML_copy3(s1, $4, $6);
  $$ = mtex2MML_copy3(s2, "</msubsup><mo>&#x2061;</mo>", $7);

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
  mtex2MML_free_string($7);
}
| mob SUB closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munder>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</munder>");
    mtex2MML_free_string(s1);
  }
  else {
    char * s1 = mtex2MML_copy3("<msub>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</msub>");
    mtex2MML_free_string(s1);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| mob SUP closedTerm SUB closedTerm {
  char * s1 = mtex2MML_copy3("<munderover>", $1, " ");
  char * s2 = mtex2MML_copy3($5, " ", $3);
  $$ = mtex2MML_copy3(s1, s2, "</munderover>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| mob SUP closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<mover>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</mover>");
    mtex2MML_free_string(s1);
  }
  else {
    char * s1 = mtex2MML_copy3("<msup>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</msup>");
    mtex2MML_free_string(s1);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| mob SUP closedTerm LIMITS SUB closedTerm {
  char * mo = mtex2MML_str_replace($1, "<mo>", "<mo movablelimits=\"false\">");
  char * s1 = mtex2MML_copy2("<munderover>", mo);
  char * s2 = mtex2MML_copy3($6, " ", $3);
  $$ = mtex2MML_copy3(s1, s2, "</munderover>");

  mtex2MML_free_string(mo);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($6);
}
| mob SUP closedTerm NOLIMITS SUB closedTerm {
  char * mo = mtex2MML_str_replace($1, "<mo>", "<mo movablelimits=\"false\">");
  char * s1 = mtex2MML_copy2("<msubsup>", mo);
  char * s2 = mtex2MML_copy3($6, " ", $3);
  $$ = mtex2MML_copy3(s1, s2, "</msubsup>");

  mtex2MML_free_string(mo);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($6);
}
| mib SUB closedTerm SUP closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munderover>", $1, " ");
    char * s2 = mtex2MML_copy3($3, " ", $5);
    $$ = mtex2MML_copy3(s1, s2, "</munderover>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  else {
    char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = mtex2MML_copy3($3, " ", $5);
    $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| mib SUB closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munder>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</munder>");
    mtex2MML_free_string(s1);
  }
  else {
    char * s1 = mtex2MML_copy3("<msub>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</msub>");
    mtex2MML_free_string(s1);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| mib SUP closedTerm SUB closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munderover>", $1, " ");
    char * s2 = mtex2MML_copy3($5, " ", $3);
    $$ = mtex2MML_copy3(s1, s2, "</munderover>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  else {
    char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = mtex2MML_copy3($5, " ", $3);
    $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| mib SUP closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<mover>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</mover>");
    mtex2MML_free_string(s1);
  }
  else {
    char * s1 = mtex2MML_copy3("<msup>", $1, " ");
    $$ = mtex2MML_copy3(s1, $3, "</msup>");
    mtex2MML_free_string(s1);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| mib LIMITS SUB closedTerm {
  char * s1 = mtex2MML_copy3("<munder>", $1, " ");
  $$ = mtex2MML_copy3(s1, $4, "</munder>");
  mtex2MML_free_string(s1);

  mtex2MML_free_string($1);
  mtex2MML_free_string($4);
}
| closedTerm SUB closedTerm SUP closedTerm {
  char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
  char * s2 = mtex2MML_copy3($3, " ", $5);
  $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| closedTerm NOLIMITS SUB closedTerm SUP closedTerm {
  char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
  char * s2 = mtex2MML_copy3($4, " ", $6);
  $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
}
| closedTerm SUP closedTerm SUB closedTerm {
  char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
  char * s2 = mtex2MML_copy3($5, " ", $3);
  $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| closedTerm SUB closedTerm {
  char * s1 = mtex2MML_copy3("<msub>", $1, " ");
  $$ = mtex2MML_copy3(s1, $3, "</msub>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| closedTerm SUP closedTerm {
  char * s1 = mtex2MML_copy3("<msup>", $1, " ");
  $$ = mtex2MML_copy3(s1, $3, "</msup>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| SUB closedTerm {
  $$ = mtex2MML_copy3("<msub><mo/>", $2, "</msub>");
  mtex2MML_free_string($2);
}
| SUP closedTerm {
  $$ = mtex2MML_copy3("<msup><mo/>", $2, "</msup>");
  mtex2MML_free_string($2);
}
| closedTerm {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

closedTerm: array
| cases
| eqalign
| eqalignno
| unaryminus
| unaryplus
| mib
| mtext {
  $$ = mtex2MML_copy3("<mtext>", $1, "</mtext>");
  mtex2MML_free_string($1);
}
| mi {
  $$ = mtex2MML_copy3("<mi>", $1, "</mi>");
  mtex2MML_free_string($1);
}
| mn {
  $$ = mtex2MML_copy3("<mn>", $1, "</mn>");
  mtex2MML_free_string($1);
}
| mo
| tensor
| multi
| mfrac
| mathopen
| mathclose
| mathord
| mathpunct
| vcenter
| enclose
| binom
| brace
| brack
| choose
| msqrt
| mroot
| leftroot
| uproot
| raisebox
| munder
| mover
| bar
| vec
| hat
| skew
| acute
| grave
| breve
| mathring
| dot
| ddot
| dddot
| ddddot
| check
| tilde
| overleftarrow
| overleftrightarrow
| overrightarrow
| underleftarrow
| underleftrightarrow
| underrightarrow
| moverbrace
| moverbracket
| munderbrace
| munderbracket
| munderline
| munderover
| emptymrow
| mathclap
| mathllap
| mathrlap
| displaystyle
| textstyle
| textsize
| scriptstyle
| scriptsize
| tiny
| ttiny
| small
| normalsize
| large
| llarge
| lllarge
| huge
| hhuge
| scriptscriptsize
| oldstyle
| moveleft
| moveright
| raise
| lower
| italics
| sans
| mono
| bold
| roman
| rmchars
| script
| scrchars
| pmb
| pmbchars
| bbold
| frak
| not
| slashed
| bcanceled
| xcanceled
| canceledto
| boxed
| fbox
| hbox
| mbox
| cal
| space
| textstring
| verbstring
| thinspace
| medspace
| thickspace
| enspace
| spacecube
| quad
| qquad
| negspace
| negmedspace
| negthickspace
| strut
| mathstrut
| smash
| phantom
| hphantom
| vphantom
| tex
| latex
| varinjlim
| varliminf
| varlimsup
| varprojlm
| href
| statusline
| tooltip
| toggle
| label
| tag
| fghighlight
| bghighlight
| colorbox
| color
| hspace
| newline
| buildrel
| texover
| texoverwithdelims
| texatop
| texatopwithdelims
| texabove
| texabovewithdelims
| MROWOPEN closedTerm MROWCLOSE {
  $$ = mtex2MML_copy_string($2);
  mtex2MML_free_string($2);
}
| MROWOPEN compoundTermList MROWCLOSE {
  $$ = mtex2MML_copy3("<mrow>", $2, "</mrow>");
  mtex2MML_free_string($2);
}
| left compoundTermList right {
  char * s1 = mtex2MML_copy3("<mrow>", $1, $2);
  $$ = mtex2MML_copy3(s1, $3, "</mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| mathenv
| substack
| sideset
| bmod
| pmod
| pod
| unrecognized;

left: LEFT LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo>", $2, "</mo>");
  mtex2MML_free_string($2);
}
| LEFT OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo>", $2, "</mo>");
  mtex2MML_free_string($2);
}
| LEFT PERIODDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy_string("");
  mtex2MML_free_string($2);
};

right: RIGHT RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo>", $2, "</mo>");
  mtex2MML_free_string($2);
}
| RIGHT OTHERDELIM {
  $$ = mtex2MML_copy3("<mo>", $2, "</mo>");
  mtex2MML_free_string($2);
}
| RIGHT PERIODDELIM {
  $$ = mtex2MML_copy_string("");
  mtex2MML_free_string($2);
};

bigdelim: BIG LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIG RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIG OTHERDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIG LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIG RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIG OTHERDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGG LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGG RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGG OTHERDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGG LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGG RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGG OTHERDELIM {
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGL LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGL OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGL LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGL OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGGL LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGGL OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGGL LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGGL OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGM LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGM OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGM LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGM OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGGM LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BIGGM OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGGM LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| BBIGGM OTHERDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\" fence=\"true\" stretchy=\"true\" symmetric=\"true\">", $2, "</mo>");
  mtex2MML_free_string($2);
};

unrecognized: UNKNOWNCHAR {
  $$ = mtex2MML_copy_string("<merror><mtext>Unknown character</mtext></merror>");
};

unaryminus: UNARYMINUS {
  $$ = mtex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">&minus;</mo>");
};

unaryplus: UNARYPLUS {
  $$ = mtex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">+</mo>");
};

varinjlim: VARINJLIM {
  $$ = mtex2MML_copy_string("<munder>lim<mo>&#x2192;</mo></munder>");
};

varliminf: VARLIMINF {
  $$ = mtex2MML_copy_string("<munder>lim<mo>_</mo></munder>");
};

varlimsup: VARLIMSUP {
  $$ = mtex2MML_copy_string("<mover>lim<mo>&#xAF;</mo></mover>");
};

varprojlm: VARPROJLIM {
  $$ = mtex2MML_copy_string("<munder>lim<mo>&#x2190;</mo></munder>");
};

mi: MI;

mtext: MTEXT;

mib: MIB {
  mtex2MML_rowposn=2;
  $$ = mtex2MML_copy3("<mi>", $1, "</mi>");
  mtex2MML_free_string($1);
};

mn: MN
| MTEXNUM TEXTSTRING {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy_string($2);
  mtex2MML_free_string($2);
};

mob: MOB {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo>", $1, "</mo>");
  mtex2MML_free_string($1);
};

mo: mob
| bigdelim
| MO {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo>", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOL {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo>", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOLL {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mstyle scriptlevel=\"0\"><mo>", $1, "</mo></mstyle>");
  mtex2MML_free_string($1);
}
| MOL LIMITS SUB closedTerm SUP closedTerm {
  if (mtex2MML_displaymode == 1) {
    char * s1 = mtex2MML_copy3("<munderover>", $1, " ");
    char * s2 = mtex2MML_copy3($4, " ", $6);
    $$ = mtex2MML_copy3(s1, s2, "</munderover>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  else {
    char * s1 = mtex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = mtex2MML_copy3($4, " ", $6);
    $$ = mtex2MML_copy3(s1, s2, "</msubsup>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string(s2);
  }
  mtex2MML_free_string($1);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
}
| RIGHTDELIM {
  $$ = mtex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| LEFTDELIM {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| OTHERDELIM {
  $$ = mtex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOF {
  $$ = mtex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| PERIODDELIM {
  $$ = mtex2MML_copy3("<mo>", $1, "</mo>");
  mtex2MML_free_string($1);
}
| COMMADELIM {
  $$ = mtex2MML_copy3("<mo>", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOS {
  mtex2MML_rowposn=2;
  $$ = mtex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOP {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo lspace=\"0em\" rspace=\"thinmathspace\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MOR {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo lspace=\"verythinmathspace\">", $1, "</mo>");
  mtex2MML_free_string($1);
}
| MATHBIN TEXTSTRING {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| MATHINNER TEXTSTRING {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", $2, "</mo>");
  mtex2MML_free_string($2);
}
| MATHREL TEXTSTRING {
  mtex2MML_rowposn = 2;
  $$ = mtex2MML_copy3("<mo lspace=\"thickmathspace\" rspace=\"thickmathspace\">", $2, "</mo>");
  mtex2MML_free_string($2);
};

space: SPACE ST INTONE END ST INTTWO END ST INTTHREE END {
  char * s1 = mtex2MML_copy3("<mspace height=\"", $3, "ex\" depth=\"");
  char * s2 = mtex2MML_copy3($6, "ex\" width=\"", $9);
  $$ = mtex2MML_copy3(s1, s2, "em\"/>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($6);
  mtex2MML_free_string($9);
};

/* TODO: intentional no op -- figure this out */
label: LABEL ATTRLIST {
  $$ = mtex2MML_copy_string("");
  mtex2MML_free_string($2);
};

/* TODO: intentional no op -- figure this out */
tag: TAG ATTRLIST {
  $$ = mtex2MML_copy_string("");
  mtex2MML_free_string($2);
};

statusline: STATLINE TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<maction actiontype=\"statusline\">", $3, "<mtext>");
  $$ = mtex2MML_copy3(s1, $2, "</mtext></maction>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

tooltip: TOOLTIP TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<maction actiontype=\"tooltip\">", $3, "<mtext>");
  $$ = mtex2MML_copy3(s1, $2, "</mtext></maction>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

toggle: TOGGLE closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<maction actiontype=\"toggle\" selection=\"2\">", $2, " ");
  $$ = mtex2MML_copy3(s1, $3, "</maction>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| TOGGLESTART compoundTermList TOGGLEEND {
  $$ = mtex2MML_copy3("<maction actiontype=\"toggle\">", $2, "</maction>");
  mtex2MML_free_string($2);
};

fghighlight: FGHIGHLIGHT ATTRLIST closedTerm {
  char * s1 = mtex2MML_copy3("<maction actiontype=\"highlight\" other='color=", $2, "'>");
  $$ = mtex2MML_copy3(s1, $3, "</maction>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

bghighlight: BGHIGHLIGHT ATTRLIST closedTerm {
  char * s1 = mtex2MML_copy3("<maction actiontype=\"highlight\" other='background=", $2, "'>");
  $$ = mtex2MML_copy3(s1, $3, "</maction>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

colorbox: COLORBOX ATTRLIST closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded width=\"+10px\" height=\"+5px\" depth=\"+5px\" lspace=\"5px\" mathbackground=", $2, ">");
  $$ = mtex2MML_copy3(s1, $3, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

color: COLOR ATTRLIST compoundTermList {
  char * s1;
  struct css_colors *c = NULL;

  HASH_FIND_STR( colors, $2, c );

  if (HASH_COUNT(c) > 0)
    s1 = mtex2MML_copy3("<mstyle mathcolor=", c->color, ">");
  else
    s1 = mtex2MML_copy3("<mstyle mathcolor=", $2, ">");

  $$ = mtex2MML_copy3(s1, $3, "</mstyle>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| BGCOLOR ATTRLIST compoundTermList {
  char * s1 = mtex2MML_copy3("<mstyle mathbackground=", $2, ">");
  $$ = mtex2MML_copy3(s1, $3, "</mstyle>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| BBOX ST PXSTRING END closedTerm {
  float padding = mtex2MML_extract_number_from_pxstring($3);
  char *px = mtex2MML_extract_string_from_pxstring($3);
  char *dbl_px = mtex2MML_double_pixel(padding, px);

  char *s1 = mtex2MML_copy3("<mpadded width=\"+", dbl_px, "\" height=\"+");
  char *s2 = mtex2MML_copy3(s1, $3, "\" depth=\"+");
  char *s3 = mtex2MML_copy3(s2, $3, "\" lspace=\"");
  char *s4 = mtex2MML_copy3(s3, $3, "\">");
  $$ = mtex2MML_copy3(s4, $5, "</mpadded>");

  mtex2MML_free_string(px);
  mtex2MML_free_string(dbl_px);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| BBOX ST COLORSTRING END closedTerm {
  char *s1 = mtex2MML_copy3("<mstyle mathbackground=\"", $3, "\">");
  $$ = mtex2MML_copy3(s1, $5, "</mstyle>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| BBOX ST COLORSTRING PXSTRING END closedTerm {
  char *s1 = mtex2MML_copy3("<mstyle mathbackground=\"", $3, "\">");

  float padding = mtex2MML_extract_number_from_pxstring($4);
  char *px = mtex2MML_extract_string_from_pxstring($4);
  char *dbl_px = mtex2MML_double_pixel(padding, px);

  char *s2 = mtex2MML_copy3(s1, "<mpadded width=\"+", dbl_px);
  char *s3 = mtex2MML_copy3(s2, "\" height=\"+", $4);
  char *s4 = mtex2MML_copy3(s3, "\" depth=\"+", $4);
  char *s5 = mtex2MML_copy3(s4, "\" lspace=\"", $4);
  char *s6 = mtex2MML_copy3(s5, "\">", $6);
  $$ = mtex2MML_copy2(s6, "</mpadded>");

  mtex2MML_free_string(px);
  mtex2MML_free_string(dbl_px);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string(s5);
  mtex2MML_free_string(s6);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
}
| BBOX ST COLORSTRING STYLESTRING PXSTRING END closedTerm {
  char *s1 = mtex2MML_copy3("<mstyle mathbackground=\"", $3, "\" style=\"");
  char *s2 = mtex2MML_copy3(s1, $4, "\">");

  float padding = mtex2MML_extract_number_from_pxstring($5);
  char *px = mtex2MML_extract_string_from_pxstring($5);
  char *dbl_px = mtex2MML_double_pixel(padding, px);

  char *s3 = mtex2MML_copy3(s2, "<mpadded width=\"+", dbl_px);
  char *s4 = mtex2MML_copy3(s3, "\" height=\"+", $5);
  char *s5 = mtex2MML_copy3(s4, "\" depth=\"+", $5);
  char *s6 = mtex2MML_copy3(s5, "\" lspace=\"", $5);
  char *s7 = mtex2MML_copy3(s6, "\">", $7);
  $$ = mtex2MML_copy2(s7, "</mpadded>");

  mtex2MML_free_string(px);
  mtex2MML_free_string(dbl_px);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string(s5);
  mtex2MML_free_string(s6);
  mtex2MML_free_string(s7);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
  mtex2MML_free_string($7);
};

mathrlap: RLAP closedTerm {
  $$ = mtex2MML_copy3("<mpadded width=\"0\">", $2, "</mpadded>");
  mtex2MML_free_string($2);
};

mathllap: LLAP closedTerm {
  $$ = mtex2MML_copy3("<mpadded width=\"0\" lspace=\"-100%width\">", $2, "</mpadded>");
  mtex2MML_free_string($2);
};

mathclap: CLAP closedTerm {
  $$ = mtex2MML_copy3("<mpadded width=\"0\" lspace=\"-50%width\">", $2, "</mpadded>");
  mtex2MML_free_string($2);
};

textstring: TEXTBOX TEXTSTRING {
  $$ = mtex2MML_copy3("<mtext>", $2, "</mtext>");
  mtex2MML_free_string($2);
};

verbstring: VERBBOX VERBSTRING {
  $$ = mtex2MML_copy3("<mstyle mathvariant=\"monospace\"><mtext>", $2, "</mtext></mstyle>");
  mtex2MML_free_string($2);
};

displaystyle: DISPLAY MROWOPEN texover MROWCLOSE {
  $$ = mtex2MML_copy3("<mstyle displaystyle=\"true\" scriptlevel=\"0\"><mrow>", $3, "</mrow></mstyle>");
  mtex2MML_free_string($3);
}
| DISPLAY compoundTermList {
  $$ = mtex2MML_copy3("<mstyle displaystyle=\"true\" scriptlevel=\"0\"><mrow>", $2, "</mrow></mstyle>");
  mtex2MML_free_string($2);
};

textstyle: TEXTSTY compoundTermList {
  $$ = mtex2MML_copy3("<mstyle displaystyle=\"false\" scriptlevel=\"0\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

textsize: TEXTSIZE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle scriptlevel=\"0\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

scriptstyle: SCSTY compoundTermList {
  $$ = mtex2MML_copy3("<mstyle displaystyle=\"false\" scriptlevel=\"1\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

scriptsize: SCSIZE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"0.7em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

scriptscriptsize: SCSCSIZE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle displaystyle=\"false\" scriptlevel=\"2\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

oldstyle: OLDSTYLE compoundTermList {
  $$ = mtex2MML_copy3("<mi mathvariant=\"normal\">", $2, "</mi>");
  mtex2MML_free_string($2);
};

tiny: TINY compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"0.5em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

ttiny: TTINY compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"0.6em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

small: SMALL compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"0.85em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

normalsize: NORMALSIZE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"1em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

large: LARGE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"1.2em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

llarge: LLARGE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"1.44em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

lllarge: LLLARGE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"1.73em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

huge: HUGE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"2.07em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

hhuge: HHUGE compoundTermList {
  $$ = mtex2MML_copy3("<mstyle mathsize=\"2.49em\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

lower: LOWER MROWOPEN PXSTRING MROWCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded height=\"-", $3, "\" depth=\"+");
  char * s2 = mtex2MML_copy3(s1, $3, "\" voffset=\"-");
  char * s3 = mtex2MML_copy3(s2, $3, "\">");
  $$ = mtex2MML_copy3(s3, $5, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| LOWER PXSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded height=\"-", $2, "\" depth=\"+");
  char * s2 = mtex2MML_copy3(s1, $2, "\" voffset=\"-");
  char * s3 = mtex2MML_copy3(s2, $2, "\">");
  $$ = mtex2MML_copy3(s3, $3, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

moveleft: MOVELEFT MROWOPEN PXSTRING MROWCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mspace width=\"-", $3, "\"/>");
  char * s2 = mtex2MML_copy3(s1, $5, "<mspace width=\"");
  $$        = mtex2MML_copy3(s2, $3, "\"/>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| MOVELEFT PXSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mspace width=\"-", $2, "\"/>");
  char * s2 = mtex2MML_copy3(s1, $3, "<mspace width=\"");
  $$        = mtex2MML_copy3(s2, $2, "\"/>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

moveright: MOVERIGHT MROWOPEN PXSTRING MROWCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mspace width=\"", $3, "\"/>");
  char * s2 = mtex2MML_copy3(s1, $5, "<mspace width=\"-");
  $$        = mtex2MML_copy3(s2, $3, "\"/>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| MOVERIGHT PXSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mspace width=\"", $2, "\"/>");
  char * s2 = mtex2MML_copy3(s1, $3, "<mspace width=\"-");
  $$        = mtex2MML_copy3(s2, $2, "\"/>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

raise: RAISE MROWOPEN PXSTRING MROWCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded height=\"+", $3, "\" depth=\"-");
  char * s2 = mtex2MML_copy3(s1, $3, "\" voffset=\"+");
  char * s3 = mtex2MML_copy3(s2, $3, "\">");
  $$ = mtex2MML_copy3(s3, $5, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| RAISE PXSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded height=\"+", $2, "\" depth=\"-");
  char * s2 = mtex2MML_copy3(s1, $2, "\" voffset=\"+");
  char * s3 = mtex2MML_copy3(s2, $2, "\">");
  $$ = mtex2MML_copy3(s3, $3, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

italics: ITALICS closedTerm {
  $$ = mtex2MML_copy3("<mstyle mathvariant=\"italic\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

sans: SANS closedTerm {
  $$ = mtex2MML_copy3("<mstyle mathvariant=\"sans-serif\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

mono: TT closedTerm {
  $$ = mtex2MML_copy3("<mstyle mathvariant=\"monospace\">", $2, "</mstyle>");
  mtex2MML_free_string($2);
};

not: NOT closedTerm {
  $$ = mtex2MML_copy3("<mi>", $2, "&#x0338;</mi>");
  mtex2MML_free_string($2);
};

slashed: SLASHED closedTerm {
  $$ = mtex2MML_copy3("<menclose notation=\"updiagonalstrike\">", $2, "</menclose>");
  mtex2MML_free_string($2);
};

bcanceled: BCANCELED closedTerm {
  $$ = mtex2MML_copy3("<menclose notation=\"downdiagonalstrike\">", $2, "</menclose>");
  mtex2MML_free_string($2);
};

xcanceled: XCANCELED closedTerm {
  $$ = mtex2MML_copy3("<menclose notation=\"updiagonalstrike downdiagonalstrike\">", $2, "</menclose>");
  mtex2MML_free_string($2);
};

canceledto: CANCELEDTO closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<msup><menclose notation=\"updiagonalstrike updiagonalarrow\"><mn>", $3, "</mn></menclose><mpadded height=\"+.1em\" depth=\"-.1em\" voffset=\".1em\"><mn>");
  $$ = mtex2MML_copy3(s1, $2, "</mn></mpadded></msup>");

  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
};

mathopen: MATHOPEN closedTerm {
  $$ = mtex2MML_copy3("<mrow>", $2, "</mrow>");

  mtex2MML_free_string($2);
};

mathclose: MATHCLOSE closedTerm {
  $$ = mtex2MML_copy3("<mrow>", $2, "</mrow>");

  mtex2MML_free_string($2);
};

mathord: MATHORD closedTerm {
  $$ = mtex2MML_copy3("<mrow>", $2, "</mrow>");

  mtex2MML_free_string($2);
};

mathpunct: MATHPUNCT closedTerm {
  $$ = mtex2MML_copy3("<mrow>", $2, "<mspace width=\"0.3em\"/></mrow>");

  mtex2MML_free_string($2);
};

enclose: ENCLOSE ST ENCLOSENOTATION ST ENCLOSETEXT ST {
  char * notation = mtex2MML_str_replace($3, ",", " ");
  char * s1 = mtex2MML_copy3("<menclose notation=\"", notation, "\"><mi>");
  $$ = mtex2MML_copy3(s1, $5, "</mi></menclose>");
  mtex2MML_free_string(notation);
  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| ENCLOSE ST ENCLOSENOTATION ST ENCLOSEATTR ST ENCLOSETEXT ST {
  char * notation = mtex2MML_str_replace($3, ",", " ");
  char * attr = mtex2MML_str_replace($5, ",", " ");
  if (strstr(attr, "arrow=1") != NULL) {
    mtex2MML_free_string(attr);
    attr = mtex2MML_str_replace($5, "arrow=1", "");

    char *t = mtex2MML_copy_string(notation);
    mtex2MML_free_string(notation);
    notation = mtex2MML_copy2(t, " updiagonalarrow");
    mtex2MML_free_string(t);
  }
  char * s1 = mtex2MML_copy3("<menclose notation=\"", notation, "\" ");
  char * s2 = mtex2MML_copy3(s1, attr, "><mi>");
  $$ = mtex2MML_copy3(s2, $7, "</mi></menclose>");
  mtex2MML_free_string(notation);
  mtex2MML_free_string(attr);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string($7);
};

vcenter: VCENTER compoundTerm {
  $$ = mtex2MML_copy3("<mrow>", $2, "</mrow>");
  mtex2MML_free_string($2);
};

boxed: BOXED closedTerm {
  $$ = mtex2MML_copy3("<menclose notation=\"box\">", $2, "</menclose>");
  mtex2MML_free_string($2);
};

fbox: FBOX closedTerm {
  $$ = mtex2MML_copy3("<menclose notation=\"box\"><mtext>", $2, "</mtext></menclose>");
  mtex2MML_free_string($2);
};

hbox: HBOX closedTerm {
  $$ = mtex2MML_copy3("<mtext>", $2, "</mtext>");
  mtex2MML_free_string($2);
};

mbox: MBOX closedTerm {
  $$ = mtex2MML_copy3("<mtext>", $2, "</mtext>");
  mtex2MML_free_string($2);
};

bold: BOLD closedTerm {
  /* TODO: stupid hack to get bold mover working */
  char * b = mtex2MML_str_replace($2, "<mi>", "<mi mathvariant=\"bold\">");

  $$ = mtex2MML_copy3("<mstyle mathvariant=\"bold\">", b, "</mstyle>");
  mtex2MML_free_string(b);
  mtex2MML_free_string($2);
};

roman: RM ST rmchars END {
  $$ = mtex2MML_copy3("<mrow><mi mathvariant=\"normal\">", $3, "</mi></mrow>");
  mtex2MML_free_string($3);
}
| RM rmchars {
  $$ = mtex2MML_copy3("<mrow><mi mathvariant=\"normal\">", $2, "</mi></mrow>");
  mtex2MML_free_string($2);
};

rmchars: RMCHAR {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rmchars rmchars {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

script: SCR ST scrchars END {
  $$ = mtex2MML_copy3("<mrow><mi mathvariant=\"script\">", $3, "</mi></mrow>");
  mtex2MML_free_string($3);
}
| SCR scrchars {
  $$ = mtex2MML_copy3("<mrow><mi mathvariant=\"script\">", $2, "</mi></mrow>");
  mtex2MML_free_string($2);
};

scrchars: SCRCHAR {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| scrchars SCRCHAR {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

pmb: PMB ST pmbchars END {
  char * s1 = mtex2MML_copy3("<mpadded width=\"0\"><mi>", $3, "</mi></mpadded>");
  char * s2 = mtex2MML_copy3(s1, "<mspace width=\"1px\"></mspace><mrow><mi>", $3);
  $$ = mtex2MML_copy2(s2, "</mi></mrow>");
  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
}
| PMB pmbchars {
  char * s1 = mtex2MML_copy3("<mpadded width=\"0\"><mi>", $2, "</mi></mpadded>");
  char * s2 = mtex2MML_copy3(s1, "<mspace width=\"1px\"></mspace><mrow><mi>", $2);
  $$ = mtex2MML_copy2(s2, "</mi></mrow>");
  mtex2MML_free_string($2);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
};

pmbchars: PMBCHAR {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| pmbchars PMBCHAR {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

bbold: BB ST bbchars END {
  $$ = mtex2MML_copy3("<mi>", $3, "</mi>");
  mtex2MML_free_string($3);
}
| BB bbchars {
  $$ = mtex2MML_copy3("<mi>", $2, "</mi>");
  mtex2MML_free_string($2);
};

bbchars: bbchar {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| bbchars bbchar {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

bbchar: BBLOWERCHAR {
  $$ = mtex2MML_copy3("&", $1, "opf;");
  mtex2MML_free_string($1);
}
| BBUPPERCHAR {
  $$ = mtex2MML_copy3("&", $1, "opf;");
  mtex2MML_free_string($1);
}
| BBDIGIT {
  /* Blackboard digits 0-9 correspond to Unicode characters 0x1D7D8-0x1D7E1 */
  char * end = $1 + 1;
  int code = 0x1D7D8 + strtoul($1, &end, 10);
  $$ = mtex2MML_character_reference(code);
  mtex2MML_free_string($1);
};

frak: FRAK ST frakletters END {
  $$ = mtex2MML_copy3("<mrow><mi>", $3, "</mi></mrow>");
  mtex2MML_free_string($3);
}
| FRAK frakletters {
  $$ = mtex2MML_copy3("<mrow><mi>", $2, "</mi></mrow>");
  mtex2MML_free_string($2);
};

frakletters: frakletter {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| frakletters frakletter {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

frakletter: FRAKCHAR {
  if (strcmp($1, " ") == 0)
    $$ = mtex2MML_copy_string(" ");
  else
    $$ = mtex2MML_copy3("&", $1, "fr;");
  mtex2MML_free_string($1);
};

cal: CAL ST calletters END {
  $$ = mtex2MML_copy3("<mrow><mi>", $3, "</mi></mrow>");
  mtex2MML_free_string($3);
}
| CAL calletters {
  $$ = mtex2MML_copy3("<mrow><mi>", $2, "</mi></mrow>");
  mtex2MML_free_string($2);
};

calletters: calletter {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| calletters calletter {
  $$ = mtex2MML_copy2($1, $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

calletter: CALCHAR {
  if (strcmp($1, " ") == 0)
    $$ = mtex2MML_copy_string(" ");
  else
    $$ = mtex2MML_copy3("&", $1, "scr;");
  mtex2MML_free_string($1);
};

thinspace: THINSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"thinmathspace\"/>");
};

medspace: MEDSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"mediummathspace\"/>");
};

thickspace: THICKSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"thickmathspace\"/>");
};

enspace: ENSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\".5em\"/>");
};

hspace: HSPACE MROWOPEN PXSTRING MROWCLOSE {
  $$ = mtex2MML_copy3("<mspace width=\"", $3, "\"/>");
  mtex2MML_free_string($3);
}
| HSPACE PXSTRING {
  $$ = mtex2MML_copy3("<mspace width=\"", $2, "\"/>");
  mtex2MML_free_string($2);
};

spacecube: SPACECUBE ST PXSTRING ST ST PXSTRING ST ST PXSTRING ST {
  char * s1 = mtex2MML_copy3("<mspace width=\"", $3, "\" height=\"");
  char * s2 = mtex2MML_copy3(s1, $6, "\" depth=\"");
  $$ = mtex2MML_copy3(s2, $9, "\"/>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($6);
  mtex2MML_free_string($9);
};

quad: QUAD {
  $$ = mtex2MML_copy_string("<mspace width=\"1em\"/>");
};

qquad: QQUAD {
  $$ = mtex2MML_copy_string("<mspace width=\"2em\"/>");
};

negspace: NEGSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"negativethinmathspace\"/>");
};

negmedspace: NEGMEDSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"negativemediummathspace\"/>");
};

negthickspace: NEGTHICKSPACE {
  $$ = mtex2MML_copy_string("<mspace width=\"negativethickmathspace\"/>");
};

strut: STRUT {
  $$ = mtex2MML_copy_string("<mpadded width=\"0\" height=\"8.6pt\" depth=\"3pt\"><mrow /></mpadded>");
};

mathstrut: MATHSTRUT {
  $$ = mtex2MML_copy_string("<mpadded width=\"0\"><mphantom><mo stretchy=\"false\">(</mo></mphantom></mpadded>");
};

smash: SMASH closedTerm {
  $$ = mtex2MML_copy3("<mpadded height=\"0\" depth=\"0\">", $2, "</mpadded>");
  mtex2MML_free_string($2);
};

phantom: PHANTOM closedTerm {
  $$ = mtex2MML_copy3("<mphantom>", $2, "</mphantom>");
  mtex2MML_free_string($2);
};

hphantom: HPHANTOM closedTerm {
  $$ = mtex2MML_copy3("<mpadded height=\"0\" depth=\"0\"><mphantom>", $2, "</mphantom></mpadded>");
  mtex2MML_free_string($2);
};

vphantom: VPHANTOM closedTerm {
  $$ = mtex2MML_copy3("<mpadded width=\"0\"><mphantom>", $2, "</mphantom></mpadded>");
  mtex2MML_free_string($2);
};

tex: TEXSYMBOL {
  $$ = mtex2MML_copy_string("<mi>T</mi><mspace width=\"-.14em\"></mspace><mpadded height=\"-.5ex\" depth=\"+.5ex\" voffset=\"-.5ex\"><mrow><mi>E</mi></mrow></mpadded><mspace width=\"-.115em\"></mspace><mi>X</mi>");
};

latex: LATEXSYMBOL {
  $$ = mtex2MML_copy_string("<mi>L</mi><mspace width=\"-.325em\"></mspace><mpadded height=\"+.21em\" depth=\"-.21em\" voffset=\"+.21em\"><mrow><mstyle scriptlevel=\"1\" displaystyle=\"false\"><mrow><mi>A</mi></mrow></mstyle></mrow></mpadded><mspace width=\"-.17em\"></mspace><mi>T</mi><mspace width=\"-.14em\"></mspace><mpadded height=\"-.5ex\" depth=\"+.5ex\" voffset=\"-.5ex\"><mrow><mi>E</mi></mrow></mpadded><mspace width=\"-.115em\"></mspace><mi>X</mi>");
};

href: HREF TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mrow href=\"", $2, "\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:type=\"simple\" xlink:href=\"");
  char * s2 = mtex2MML_copy3(s1, $2, "\">");
  $$ = mtex2MML_copy3(s2, $3, "</mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

tensor: TENSOR closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = mtex2MML_copy3("<mmultiscripts>", $2, $4);
  $$ = mtex2MML_copy2(s1, "</mmultiscripts>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| TENSOR closedTerm subsupList {
  char * s1 = mtex2MML_copy3("<mmultiscripts>", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mmultiscripts>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

multi: MULTI MROWOPEN subsupList MROWCLOSE closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = mtex2MML_copy3("<mmultiscripts>", $5, $7);
  char * s2 = mtex2MML_copy3("<mprescripts/>", $3, "</mmultiscripts>");
  $$ = mtex2MML_copy2(s1, s2);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string($7);
}
| MULTI MROWOPEN subsupList MROWCLOSE closedTerm EMPTYMROW {
  char * s1 = mtex2MML_copy2("<mmultiscripts>", $5);
  char * s2 = mtex2MML_copy3("<mprescripts/>", $3, "</mmultiscripts>");
  $$ = mtex2MML_copy2(s1, s2);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| MULTI EMPTYMROW closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = mtex2MML_copy3("<mmultiscripts>", $3, $5);
  $$ = mtex2MML_copy2(s1, "</mmultiscripts>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
};

subsupList: subsupTerm {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| subsupList subsupTerm {
  $$ = mtex2MML_copy3($1, " ", $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

subsupTerm: SUB closedTerm SUP closedTerm {
  $$ = mtex2MML_copy3($2, " ", $4);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SUB closedTerm {
  $$ = mtex2MML_copy2($2, " <none/>");
  mtex2MML_free_string($2);
}
| SUP closedTerm {
  $$ = mtex2MML_copy2("<none/> ", $2);
  mtex2MML_free_string($2);
}
| SUB SUP closedTerm {
  $$ = mtex2MML_copy2("<none/> ", $3);
  mtex2MML_free_string($3);
};

mfrac: FRAC closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mfrac>", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| TFRAC closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mstyle displaystyle=\"false\"><mfrac>", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac></mstyle>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| DFRAC closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mstyle displaystyle=\"true\" scriptlevel=\"0\"><mfrac>", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac></mstyle>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| CFRAC closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mfrac><mrow><mpadded width=\"0\" height=\"8.6pt\" depth=\"3pt\"><mrow /></mpadded><mstyle displaystyle=\"false\" scriptlevel=\"0\"><mrow>", $2, "</mrow></mstyle></mrow><mrow><mpadded width=\"0\" height=\"8.6pt\" depth=\"3pt\"><mrow /></mpadded><mstyle displaystyle=\"false\" scriptlevel=\"0\"><mrow>");
  $$ = mtex2MML_copy3(s1, $3, "</mrow></mstyle></mrow></mfrac>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| GENFRAC MROWOPEN LEFTDELIM MROWCLOSE MROWOPEN RIGHTDELIM MROWCLOSE MROWOPEN PXSTRING MROWCLOSE compoundTermList compoundTermList compoundTermList {
  char *s1 = NULL, *s2 = NULL, *s3 = NULL, *s4 = NULL;
  int style = 0;
  sscanf ($11,"%d", &style);

  switch (style) {
   case 0:
    s1 = mtex2MML_copy3("<mrow><mstyle displaystyle=\"true\" scriptlevel=\"0\"><mrow><mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">", $3, "</mo></mrow><mfrac linethickness=\"");
    s2 = mtex2MML_copy3(s1, $9, "\">");
    s3 = mtex2MML_copy3(s2, $12 , $13);
    s4 = mtex2MML_copy3(s3, "</mfrac><mrow>", "<mo maxsize=\"2.047em\" minsize=\"2.047em\">");
    break;
   case 1:
    s1 = mtex2MML_copy3("<mrow><mstyle displaystyle=\"false\" scriptlevel=\"0\"><mrow><mrow><mo maxsize=\"1.2em\" minsize=\"1.2em\">", $3, "</mo></mrow><mfrac linethickness=\"");
    s2 = mtex2MML_copy3(s1, $9, "\">");
    s3 = mtex2MML_copy3(s2, $12 , $13);
    s4 = mtex2MML_copy3(s3, "</mfrac><mrow>", "<mo maxsize=\"1.2em\" minsize=\"1.2em\">");
    break;
   case 2:
     s1 = mtex2MML_copy3("<mrow><mstyle displaystyle=\"false\" scriptlevel=\"1\"><mrow><mrow><mo maxsize=\"1.2em\" minsize=\"1.2em\">", $3, "</mo></mrow><mfrac linethickness=\"");
     s2 = mtex2MML_copy3(s1, $9, "\">");
     s3 = mtex2MML_copy3(s2, $12 , $13);
     s4 = mtex2MML_copy3(s3, "</mfrac><mrow>", "<mo maxsize=\"1.2em\" minsize=\"1.2em\">");
     break;
    case 3:
      s1 = mtex2MML_copy3("<mrow><mstyle displaystyle=\"false\" scriptlevel=\"2\"><mrow><mrow><mo maxsize=\"1.2em\" minsize=\"1.2em\">", $3, "</mo></mrow><mfrac linethickness=\"");
      s2 = mtex2MML_copy3(s1, $9, "\">");
      s3 = mtex2MML_copy3(s2, $12 , $13);
      s4 = mtex2MML_copy3(s3, "</mfrac><mrow>", "<mo maxsize=\"1.2em\" minsize=\"1.2em\">");
      break;
  }

  $$ = mtex2MML_copy3(s4, $6, "</mo></mrow></mrow></mstyle></mrow>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string($3);
  mtex2MML_free_string($6);
  mtex2MML_free_string($9);
  mtex2MML_free_string($11);
  mtex2MML_free_string($12);
  mtex2MML_free_string($13);
};

pod: POD closedTerm {
  $$ = mtex2MML_copy3( "<mrow><mo lspace=\"mediummathspace\">(</mo>", $2, "<mo rspace=\"mediummathspace\">)</mo></mrow>");
  mtex2MML_free_string($2);
};

pmod: PMOD closedTerm {
  $$ = mtex2MML_copy3( "<mrow><mo lspace=\"mediummathspace\">(</mo><mo rspace=\"thinmathspace\">mod</mo>", $2, "<mo rspace=\"mediummathspace\">)</mo></mrow>");
  mtex2MML_free_string($2);
};

bmod: BMOD closedTerm {
  $$ = mtex2MML_copy3( "<mrow><mo lspace=\"thickmathspace\" rspace=\"thickmathspace\">mod</mo>", $2, "</mrow>");
  mtex2MML_free_string($2);
};

buildrel: BUILDREL closedTerm TEXOVER closedTerm {
  char *s1 = mtex2MML_copy3("<mrow><mover>", $4, $2);
  $$ = mtex2MML_copy2(s1, "</mover></mrow>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
};

texover: closedTerm TEXOVER closedTerm {
  char * s1 = mtex2MML_copy3("<mfrac>", $1, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| left compoundTermList TEXOVER compoundTermList right {
  char * s1 = mtex2MML_copy3("<mrow>", $1, "<mfrac><mrow>");
  char * s2 = mtex2MML_copy3($2, "</mrow><mrow>", $4);
  char * s3 = mtex2MML_copy3("</mrow></mfrac>", $5, "</mrow>");
  $$ = mtex2MML_copy3(s1, s2, s3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
};

texoverwithdelims: closedTerm TEXOVERWITHDELIMS LEFTDELIM RIGHTDELIM closedTerm {
  char * s1 = mtex2MML_copy3("<mo fence=\"true\" stretchy=\"true\">", $3, "</mo><mfrac>");
  char * s2 = mtex2MML_copy3(s1, $1, $5);
  char * s3 = mtex2MML_copy2(s2, "</mfrac><mo fence=\"true\" stretchy=\"true\">");
  $$ = mtex2MML_copy3(s3, $4, "</mo>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
};

texatop: MROWOPEN compoundTermList TEXATOP compoundTermList MROWCLOSE {
  char * s1 = mtex2MML_copy3("<mfrac linethickness=\"0\"><mrow>", $2, "</mrow><mrow>");
  $$ = mtex2MML_copy3(s1, $4, "</mrow></mfrac>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| closedTerm TEXATOP closedTerm {
  char * s1 = mtex2MML_copy3("<mfrac linethickness=\"0\">", $1, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

texatopwithdelims: closedTerm TEXATOPWITHDELIMS LEFTDELIM RIGHTDELIM closedTerm {
  char * s1 = mtex2MML_copy3("<mo fence=\"true\" stretchy=\"true\">", $3, "</mo><mfrac linethickness=\"0\">");
  char * s2 = mtex2MML_copy3(s1, $1, $5);
  char * s3 = mtex2MML_copy2(s2, "</mfrac><mo fence=\"true\" stretchy=\"true\">");
  $$ = mtex2MML_copy3(s3, $4, "</mo>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
};

texabove: closedTerm TEXABOVE MROWOPEN PXSTRING MROWCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mfrac linethickness=\"", $4, "\">");
  char * s2 = mtex2MML_copy3(s1, $1, $6);
  $$ = mtex2MML_copy2(s2, "</mfrac>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
};

texabovewithdelims: closedTerm TEXABOVEWITHDELIMS LEFTDELIM RIGHTDELIM PXSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mo fence=\"true\" stretchy=\"true\">", $3, "</mo>");
  char * s2 = mtex2MML_copy3(s1, "<mfrac linethickness=\"", $5);
  char * s3 = mtex2MML_copy3(s2, "\">", $1);
  char * s4 = mtex2MML_copy3(s3, $6, "</mfrac><mo fence=\"true\" stretchy=\"true\">");
  $$ = mtex2MML_copy3(s4, $4, "</mo>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
  mtex2MML_free_string($6);
};

binom: BINOM closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">(</mo><mfrac linethickness=\"0\">", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac><mo maxsize=\"2.047em\" minsize=\"2.047em\">)</mo></mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| TBINOM closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">(</mo><mstyle displaystyle=\"false\"><mfrac linethickness=\"0\">", $2, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac></mstyle><mo maxsize=\"2.047em\" minsize=\"2.047em\">)</mo></mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| DBINOM closedTerm closedTerm {
    char * s1 = mtex2MML_copy3("<mstyle displaystyle=\"true\" scriptlevel=\"0\"><mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">(</mo><mfrac linethickness=\"0\">", $2, $3);
    $$ = mtex2MML_copy2(s1, "</mfrac><mo maxsize=\"2.047em\" minsize=\"2.047em\">)</mo></mrow></mstyle>");
    mtex2MML_free_string(s1);
    mtex2MML_free_string($2);
    mtex2MML_free_string($3);
};

brace: closedTerm BRACE closedTerm {
  char * s1 = mtex2MML_copy3("<mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">{</mo><mfrac linethickness=\"0\">", $1, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac><mo maxsize=\"2.047em\" minsize=\"2.047em\">}</mo></mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

brack: closedTerm BRACK closedTerm {
  char * s1 = mtex2MML_copy3("<mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">[</mo><mfrac linethickness=\"0\">", $1, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac><mo maxsize=\"2.047em\" minsize=\"2.047em\">]</mo></mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

choose: closedTerm CHOOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mrow><mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">(</mo></mrow><mfrac linethickness=\"0\">", $1, $3);
  $$ = mtex2MML_copy2(s1, "</mfrac><mrow><mo maxsize=\"2.047em\" minsize=\"2.047em\">)</mo></mrow></mrow>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

underleftarrow: UNDERLEFTARROW closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&larr;</mo></munder>");
  mtex2MML_free_string($2);
};

underleftrightarrow: UNDERLEFTRIGHTARROW closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&harr;</mo></munder>");
  mtex2MML_free_string($2);
};

underrightarrow: UNDERRIGHTARROW closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&rarr;</mo></munder>");
  mtex2MML_free_string($2);
};

/* TODO: can't seem to get Lasem to render the entity name; hardcode hex code */
munderbrace: UNDERBRACE closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&#x23DF;</mo></munder>");
  mtex2MML_free_string($2);
}
| UNDERBRACE closedTerm SUB closedTerm {
  char * s1 = mtex2MML_copy3("<munder><munder>", $2, "<mo>&#x23DF;</mo></munder>");
  $$ = mtex2MML_copy3(s1, $4, "</munder>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| UNDERBRACE closedTerm LIMITS SUB closedTerm {
  char * s1 = mtex2MML_copy3("<munder><munder>", $2, "<mo>&#x23DF;</mo></munder>");
  $$ = mtex2MML_copy3(s1, $5, "</munder>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($5);
}
| UNDERBRACE closedTerm NOLIMITS SUB closedTerm {
  char * s1 = mtex2MML_copy3("<msub><munder>", $2, "<mo>&#x23DF;</mo></munder>");
  $$ = mtex2MML_copy3(s1, $5, "</msub>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($5);
};

munderbracket: UNDERBRACKET closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&#9183;</mo></munder>");
  mtex2MML_free_string($2);
};

munderline: UNDERLINE closedTerm {
  $$ = mtex2MML_copy3("<munder>", $2, "<mo>&#x00332;</mo></munder>");
  mtex2MML_free_string($2);
};

moverbrace: OVERBRACE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#x23DE;</mo></mover>");
  mtex2MML_free_string($2);
};

moverbracket: OVERBRACKET closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#9183;</mo></mover>");
  mtex2MML_free_string($2);
};

overleftarrow: OVERLEFTARROW closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&larr;</mo></mover>");
  mtex2MML_free_string($2);
};

overleftrightarrow: OVERLEFTRIGHTARROW closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&harr;</mo></mover>");
  mtex2MML_free_string($2);
};

overrightarrow: OVERRIGHTARROW closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&rarr;</mo></mover>");
  mtex2MML_free_string($2);
};

bar: BAR closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x000AF;</mo></mover>");
  mtex2MML_free_string($2);
}
| WIDEBAR closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#x000AF;</mo></mover>");
  mtex2MML_free_string($2);
};

vec: VEC closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x2192;</mo></mover>");
  mtex2MML_free_string($2);
}
| WIDEVEC closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#x2192;</mo></mover>");
  mtex2MML_free_string($2);
};

acute: ACUTE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&acute;</mo></mover>");
  mtex2MML_free_string($2);
};

grave: GRAVE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x60;</mo></mover>");
  mtex2MML_free_string($2);
};

breve: BREVE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x2d8;</mo></mover>");
  mtex2MML_free_string($2);
};

mathring: MATHRING closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#730;</mo></mover>");
  mtex2MML_free_string($2);
};

dot: DOT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&dot;</mo></mover>");
  mtex2MML_free_string($2);
};

ddot: DDOT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&Dot;</mo></mover>");
  mtex2MML_free_string($2);
};

dddot: DDDOT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&tdot;</mo></mover>");
  mtex2MML_free_string($2);
};

ddddot: DDDDOT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&DotDot;</mo></mover>");
  mtex2MML_free_string($2);
};

tilde: TILDE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&tilde;</mo></mover>");
  mtex2MML_free_string($2);
}
| WIDETILDE closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&tilde;</mo></mover>");
  mtex2MML_free_string($2);
};

check: CHECK closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x2c7;</mo></mover>");
  mtex2MML_free_string($2);
}
| WIDECHECK closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#x2c7;</mo></mover>");
  mtex2MML_free_string($2);
};

hat: HAT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x5E;</mo></mover>");
  mtex2MML_free_string($2);
}
| WIDEHAT closedTerm {
  $$ = mtex2MML_copy3("<mover>", $2, "<mo>&#x5E;</mo></mover>");
  mtex2MML_free_string($2);
};

skew: SKEW closedTerm ACUTE closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&acute;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm BAR closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x000AF;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm BREVE closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x2d8;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm CHECK closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x2c7;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm DOT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&dot;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm DDOT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&Dot;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm DDDOT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&tdot;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm DDDDOT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&DotDot;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm GRAVE closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x60;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm HAT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x5E;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm TILDE closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&tilde;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm WIDEHAT closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x5E;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
}
| SKEW closedTerm WIDETILDE closedTerm {
  char * em_skew = mtex2MML_dbl2em($2);

  $$ = mtex2MML_implement_skew($4, em_skew, "&#x5E;");

  mtex2MML_free_string(em_skew);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
};

msqrt: SQRT closedTerm {
  $$ = mtex2MML_copy3("<msqrt>", $2, "</msqrt>");
  mtex2MML_free_string($2);
};

mroot: SQRT OPTARGOPEN compoundTermList OPTARGCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<mroot>", $5, $3);
  $$ = mtex2MML_copy2(s1, "</mroot>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| ROOT closedTerm OF closedTerm {
  char * s1 = mtex2MML_copy3("<mroot>", $4, $2);
  $$ = mtex2MML_copy2(s1, "</mroot>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($4);
};

leftroot: LEFTROOT ST NUMBER END closedTerm {
  char * spacing = mtex2MML_root_pos_to_em($3);

  char *s1 = mtex2MML_copy3("<mpadded width=\"", spacing, "\"><mrow>");
  $$ = mtex2MML_copy3(s1, $5, "</mrow></mpadded>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(spacing);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
};

uproot: UPROOT ST NUMBER END closedTerm {
  char * spacing = mtex2MML_root_pos_to_em($3);

  char *s1 = mtex2MML_copy3("<mpadded height=\"", spacing, "\" voffset=\"");
  char *s2 = mtex2MML_copy3(s1, spacing, "\"><mrow>");
  $$ = mtex2MML_copy3(s2, $5, "</mrow></mpadded>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(spacing);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
};

raisebox: RAISEBOX TEXTSTRING TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='", $2, "' height='");
  char * s2 = mtex2MML_copy3(s1, $3, "' depth='");
  char * s3 = mtex2MML_copy3(s2, $4, "'>");
  $$ = mtex2MML_copy3(s3, $5, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
}
| RAISEBOX NEG TEXTSTRING TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='-", $3, "' height='");
  char * s2 = mtex2MML_copy3(s1, $4, "' depth='");
  char * s3 = mtex2MML_copy3(s2, $5, "'>");
  $$ = mtex2MML_copy3(s3, $6, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
  mtex2MML_free_string($6);
}
| RAISEBOX TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='", $2, "' height='");
  char * s2 = mtex2MML_copy3(s1, $3, "' depth='depth'>");
  $$ = mtex2MML_copy3(s2, $4, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
}
| RAISEBOX NEG TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='-", $3, "' height='");
  char * s2 = mtex2MML_copy3(s1, $4, "' depth='+");
  char * s3 = mtex2MML_copy3(s2, $3, "'>");
  $$ = mtex2MML_copy3(s3, $5, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($5);
}
| RAISEBOX TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='", $2, "' height='+");
  char * s2 = mtex2MML_copy3(s1, $2, "' depth='depth'>");
  $$ = mtex2MML_copy3(s2, $3, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
}
| RAISEBOX NEG TEXTSTRING closedTerm {
  char * s1 = mtex2MML_copy3("<mpadded voffset='-", $3, "' height='0pt' depth='+");
  char * s2 = mtex2MML_copy3(s1, $3, "'>");
  $$ = mtex2MML_copy3(s2, $4, "</mpadded>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
};

munder: XARROW OPTARGOPEN compoundTermList OPTARGCLOSE EMPTYMROW {
  char * s1 = mtex2MML_copy3("<munder><mo>", $1, "</mo><mrow>");
  $$ = mtex2MML_copy3(s1, $3, "</mrow></munder>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
}
| UNDER closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<munder>", $3, $2);
  $$ = mtex2MML_copy2(s1, "</munder>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

mover: XARROW closedTerm {
  char * s1 = mtex2MML_copy3("<mover><mo>", $1, "</mo>");
  $$ =  mtex2MML_copy3(s1, $2, "</mover>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
}
| OVER closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<mover>", $3, $2);
  $$ = mtex2MML_copy2(s1, "</mover>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
};

munderover: XARROW OPTARGOPEN compoundTermList OPTARGCLOSE closedTerm {
  char * s1 = mtex2MML_copy3("<munderover><mo>", $1, "</mo><mrow>");
  char * s2 = mtex2MML_copy3(s1, $3, "</mrow>");
  $$ = mtex2MML_copy3(s2, $5, "</munderover>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
}
| UNDEROVER closedTerm closedTerm closedTerm {
  char * s1 = mtex2MML_copy3("<munderover>", $4, $2);
  $$ = mtex2MML_copy3(s1, $3, "</munderover>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
  mtex2MML_free_string($3);
  mtex2MML_free_string($4);
};

emptymrow: EMPTYMROW {
  $$ = mtex2MML_copy_string("<mrow/>");
};

mathenv: BEGINENV EQUATION tableRowList ENDENV EQUATION {
  $$ = mtex2MML_copy3("<mtable>", $3, "</mtable>");

  mtex2MML_free_string($3);
}
| BEGINENV EQUATION_STAR tableRowList ENDENV EQUATION_STAR {
  $$ = mtex2MML_copy3("<mtable>", $3, "</mtable>");

  mtex2MML_free_string($3);
}
| BEGINENV MATRIX tableRowList ENDENV MATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"false\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV GATHERED ARRAYALIGN END tableRowList ENDENV GATHERED {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" align=\"", $3, "\" ");
  char * s2 = mtex2MML_copy3(s1, row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s2, $5, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s2, $5, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(row_data);
}
| BEGINENV GATHERED tableRowList ENDENV GATHERED {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV EQNARRAY tableRowList ENDENV EQNARRAY {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnalign=\"right center left\" columnspacing=\"thickmathspace\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV EQNARRAY_STAR tableRowList ENDENV EQNARRAY_STAR {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnalign=\"right center left\" columnspacing=\"thickmathspace\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV GATHER tableRowList ENDENV GATHER {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV GATHER_STAR tableRowList ENDENV GATHER_STAR {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV MULTLINE tableRowList ENDENV MULTLINE {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV MULTLINE_STAR tableRowList ENDENV MULTLINE_STAR {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnwidth=\"100%\" width=\"85%\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV PMATRIX tableRowList ENDENV PMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>(</mo><mrow><mtable displaystyle=\"false\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>)</mo></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>)</mo></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV BMATRIX tableRowList ENDENV BMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>[</mo><mrow><mtable displaystyle=\"false\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>]</mo></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>]</mo></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV VMATRIX tableRowList ENDENV VMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>&VerticalBar;</mo><mrow><mtable displaystyle=\"false\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
      char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>&VerticalBar;</mo></mrow>");
      $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
      mtex2MML_free_string(t);
    }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>&VerticalBar;</mo></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV BBMATRIX tableRowList ENDENV BBMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>{</mo><mrow><mtable displaystyle=\"false\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>}</mo></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>}</mo></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV VVMATRIX tableRowList ENDENV VVMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>&DoubleVerticalBar;</mo><mrow><mtable displaystyle=\"false\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>&DoubleVerticalBar;</mo></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow><mo>&DoubleVerticalBar;</mo></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV SMALLMATRIX tableRowList ENDENV SMALLMATRIX {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"false\" columnspacing=\"0.333em\" ", row_data, ">");
  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV CASES tableRowList ENDENV CASES {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mo>{</mo><mrow><mtable displaystyle=\"false\" columnalign=\"left left\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNED ARRAYALIGN END tableRowList ENDENV ALIGNED {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" align=\"", $3, "\" columnspacing=\"0em 2em 0em 2em 0em 2em 0em 2em 0em 2em 0em\" columnalign=\"right left right left right left right left right left\" ");
  char * s2 = mtex2MML_copy3(s1, row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s2, $5, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s2, $5, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNED tableRowList ENDENV ALIGNED {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnspacing=\"0em 2em 0em 2em 0em 2em 0em 2em 0em 2em 0em\" columnalign=\"right left right left right left right left right left\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNENV tableRowList ENDENV ALIGNENV {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnspacing=\"0em 2em 0em 2em 0em 2em 0em 2em 0em 2em 0em\" columnalign=\"right left right left right left right left right left\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNENV_STAR tableRowList ENDENV ALIGNENV_STAR {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnspacing=\"0em 2em 0em 2em 0em 2em 0em 2em 0em 2em 0em\" columnalign=\"right left right left right left right left right left\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $3, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $3, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNAT ALIGNATVALUE END tableRowList ENDENV ALIGNAT {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnalign=\"right left\" columnspacing=\"0em\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $5, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $5, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNAT_STAR ALIGNATVALUE END tableRowList ENDENV ALIGNAT_STAR {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnalign=\"right left\" columnspacing=\"0em\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $5, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $5, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ALIGNEDAT ALIGNATVALUE END tableRowList ENDENV ALIGNEDAT {
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mrow><mtable displaystyle=\"true\" columnalign=\"right left right left right left right left right left\" columnspacing=\"0em\" ", row_data, ">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s1, $5, "</mtable></mrow>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s1, $5, "</mtable></mrow>");

  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string(s1);
  mtex2MML_free_string(row_data);
}
| BEGINENV ARRAY ARRAYALIGN ST columnAlignList END tableRowList ENDENV ARRAY {
  char *pipe_chars = mtex2MML_vertical_pipe_extract($5);
  char *column_align = mtex2MML_remove_excess_pipe_chars($5);
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mtable displaystyle=\"false\" align=\"", $3, "\" ");
  char * s2 = mtex2MML_copy3(s1, row_data, " columnalign=\"");
  char * s3 = mtex2MML_copy3(s2, column_align, "\" ");
  char * s4 = mtex2MML_copy3(s3, pipe_chars, "\">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s4, $7, "</mtable>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s4, $7, "</mtable>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
  mtex2MML_free_string($7);
  mtex2MML_free_string(pipe_chars);
  mtex2MML_free_string(column_align);
  mtex2MML_free_string(row_data);
}
| BEGINENV ARRAY ST columnAlignList END tableRowList ENDENV ARRAY {
  char *pipe_chars = mtex2MML_vertical_pipe_extract($4);
  char *column_align = mtex2MML_remove_excess_pipe_chars($4);
  char *row_data = mtex2MML_combine_row_data(&environment_data_stack);

  char * s1 = mtex2MML_copy3("<mtable displaystyle=\"false\" ", row_data, " columnalign=\"");
  char * s2 = mtex2MML_copy3(s1, column_align, "\" ");
  char * s3 = mtex2MML_copy3(s2, pipe_chars, "\">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s3, $6, "</mtable>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s3, $6, "</mtable>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
  mtex2MML_free_string(pipe_chars);
  mtex2MML_free_string(column_align);
  mtex2MML_free_string(row_data);
}
| BEGINENV SUBARRAY ST columnAlignList END tableRowList ENDENV SUBARRAY {
  char *pipe_chars = mtex2MML_vertical_pipe_extract($4);
  char *column_align = mtex2MML_remove_excess_pipe_chars($4);

  char * s1 = mtex2MML_copy3("<mtable displaystyle=\"false\" columnspacing=\"0em 0em 0em 0em\" rowspacing=\"0.1em\" columnalign=\"", column_align, "\" ");
  char * s2 = mtex2MML_copy3(s1, pipe_chars, "\">");

  if (encase == TOPENCLOSE) {
    char *t = mtex2MML_copy3(s2, $6, "</mtable>");
    $$ = mtex2MML_copy3("<menclose notation=\"top\">", t, "</menclose>");
    mtex2MML_free_string(t);
  }
  else
    $$ = mtex2MML_copy3(s2, $6, "</mtable>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
  mtex2MML_free_string(pipe_chars);
  mtex2MML_free_string(column_align);
}
| BEGINENV SVG XMLSTRING ENDSVG {
  $$ = mtex2MML_copy3("<semantics><annotation-xml encoding=\"SVG1.1\">", $3, "</annotation-xml></semantics>");
  mtex2MML_free_string($3);
}
| BEGINENV SVG ENDSVG {
  $$ = mtex2MML_copy_string(" ");
};

columnAlignList: columnAlignList COLUMNALIGN {
  $$ = mtex2MML_copy3($1, " ", $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
}
| COLUMNALIGN {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

substack: SUBSTACK MROWOPEN tableRowList MROWCLOSE {
  $$ = mtex2MML_copy3("<mrow><mtable columnalign=\"center\" rowspacing=\"0.5ex\">", $3, "</mtable></mrow>");
  mtex2MML_free_string($3);
};

sideset: SIDESET MROWOPEN SUB closedTerm SUP closedTerm MROWCLOSE MROWOPEN SUB closedTerm SUP closedTerm MROWCLOSE closedTerm {
  char *s1 = mtex2MML_copy2("<msubsup><mphantom>", "</mphantom><mrow>");
  char *s2 = mtex2MML_copy3(s1, $4, "</mrow>");
  char *s3 = mtex2MML_copy3(s2, $6, "</msubsup><mspace width=\"negativethinmathspace\" /><msubsup><mrow>");
  char *s4 = mtex2MML_copy3(s3, $14, "</mrow>");
  char *s5 = mtex2MML_copy3(s4, $10, $12);
  $$ = mtex2MML_copy2(s5, "</msubsup>");

  mtex2MML_free_string(s1);
  mtex2MML_free_string(s2);
  mtex2MML_free_string(s3);
  mtex2MML_free_string(s4);
  mtex2MML_free_string(s5);
  mtex2MML_free_string($4);
  mtex2MML_free_string($6);
  mtex2MML_free_string($10);
  mtex2MML_free_string($12);
  mtex2MML_free_string($14);
};

array: ARRAY MROWOPEN tableRowList MROWCLOSE {
  $$ = mtex2MML_copy3("<mrow><mtable>", $3, "</mtable></mrow>");
  mtex2MML_free_string($3);
}
| ARRAY MROWOPEN ARRAYOPTS MROWOPEN arrayopts MROWCLOSE tableRowList MROWCLOSE {
  char * s1 = mtex2MML_copy3("<mrow><mtable ", $5, ">");
  $$ = mtex2MML_copy3(s1, $7, "</mtable></mrow>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($5);
  mtex2MML_free_string($7);
};

cases: CASES MROWOPEN tableRowList MROWCLOSE {
  $$ = mtex2MML_copy3("<mrow><mo>{</mo><mrow><mtable displaystyle=\"false\" columnalign=\"left left\">", $3, "</mtable></mrow></mrow>");
  mtex2MML_free_string($3);
};

eqalign: EQALIGN MROWOPEN tableRowList MROWCLOSE {
  $$ = mtex2MML_copy3("<mtable columnalign=\"right left\" rowspacing=\".5em\" columnspacing=\"thickmathspace\" displaystyle=\"true\">", $3, "</mtable></mrow></mrow>");
  mtex2MML_free_string($3);
};

eqalignno: EQALIGNNO MROWOPEN tableRowList MROWCLOSE {
  $$ = mtex2MML_copy3("<mtable columnalign=\"right left right\" rowspacing=\".5em\" columnspacing=\"thickmathspace 3em\" displaystyle=\"true\">", $3, "</mtable></mrow></mrow>");
  mtex2MML_free_string($3);
};

arrayopts: anarrayopt {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| arrayopts anarrayopt {
  $$ = mtex2MML_copy3($1, " ", $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

anarrayopt: collayout {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| colalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| align {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| eqrows {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| eqcols {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowlines {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| collines {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| frame {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| padding {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

collayout: COLLAYOUT ATTRLIST {
  $$ = mtex2MML_copy2("columnalign=", $2);
  mtex2MML_free_string($2);
};

colalign: COLALIGN ATTRLIST {
  $$ = mtex2MML_copy2("columnalign=", $2);
  mtex2MML_free_string($2);
};

rowalign: ROWALIGN ATTRLIST {
  $$ = mtex2MML_copy2("rowalign=", $2);
  mtex2MML_free_string($2);
};

align: ALIGN ATTRLIST {
  $$ = mtex2MML_copy2("align=", $2);
  mtex2MML_free_string($2);
};

eqrows: EQROWS ATTRLIST {
  $$ = mtex2MML_copy2("equalrows=", $2);
  mtex2MML_free_string($2);
};

eqcols: EQCOLS ATTRLIST {
  $$ = mtex2MML_copy2("equalcolumns=", $2);
  mtex2MML_free_string($2);
};

rowlines: ROWLINES ATTRLIST {
  $$ = mtex2MML_copy2("rowlines=", $2);
  mtex2MML_free_string($2);
};

collines: COLLINES ATTRLIST {
  $$ = mtex2MML_copy2("columnlines=", $2);
  mtex2MML_free_string($2);
};

frame: FRAME ATTRLIST {
  $$ = mtex2MML_copy2("frame=", $2);
  mtex2MML_free_string($2);
};

padding: PADDING ATTRLIST {
  char * s1 = mtex2MML_copy3("rowspacing=", $2, " columnspacing=");
  $$ = mtex2MML_copy2(s1, $2);
  mtex2MML_free_string(s1);
  mtex2MML_free_string($2);
};

tableRowList: tableRow {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| tableRowList ROWSEP tableRow {
  $$ = mtex2MML_copy3($1, " ", $3);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

tableRow: simpleTableRow {
  int has_eqn_number = mtex2MML_fetch_eqn_number(&environment_data_stack);

  if (has_eqn_number && strcmp($1, "<mtd/>") != 0) {
    char * n = mtex2MML_global_label();

    char *s1 = mtex2MML_copy3("<mlabeledtr>", n, $1);
    $$ = mtex2MML_copy2(s1, "</mlabeledtr>");

    mtex2MML_free_string(n);
    mtex2MML_free_string(s1);
  }
  else {
    $$ = mtex2MML_copy3("<mtr>", $1, "</mtr>");
  }

  mtex2MML_free_string($1);
}
| optsTableRow {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

simpleTableRow: tableCell {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| simpleTableRow COLSEP tableCell {
  $$ = mtex2MML_copy3($1, " ", $3);
  mtex2MML_free_string($1);
  mtex2MML_free_string($3);
};

optsTableRow: ROWOPTS MROWOPEN rowopts MROWCLOSE simpleTableRow {
  char * s1 = mtex2MML_copy3("<mtr ", $3, ">");
  $$ = mtex2MML_copy3(s1, $5, "</mtr>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
};

rowopts: arowopt {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowopts arowopt {
  $$ = mtex2MML_copy3($1, " ", $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

arowopt: colalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

newline: NEWLINE {
  $$ = mtex2MML_copy_string("<mspace linebreak=\"newline\" />");
}

tableCell:   {
  $$ = mtex2MML_copy_string("<mtd></mtd>");
}
| compoundTermList {
  if (mtex2MML_current_env_type(&environment_data_stack) != ENV_MULTLINE && mtex2MML_current_env_type(&environment_data_stack) != ENV_MULTLINESTAR) {
    if (mtex2MML_current_env_type(&environment_data_stack) == ENV_ALIGNAT || mtex2MML_current_env_type(&environment_data_stack) == ENV_ALIGNATSTAR) {
      $$ = mtex2MML_copy3("<mtd><mi></mi>", $1, "</mtd>");
    }
    else {
      $$ = mtex2MML_copy3("<mtd>", $1, "</mtd>");
    }
  }
  else {
    int total_lines = mtex2MML_current_env_line_count(&environment_data_stack);
    if (line_counter == 1) {
      $$ = mtex2MML_copy3("<mtd columnalign=\"left\">", $1, "</mtd>");
      line_counter++;
    }
    else if (line_counter == total_lines) {
      $$ = mtex2MML_copy3("<mtd columnalign=\"right\">", $1, "</mtd>");
    }
    else {
      $$ = mtex2MML_copy3("<mtd columnalign=\"center\">", $1, "</mtd>");
      line_counter++;
    }
  }

  mtex2MML_free_string($1);
}
| CELLOPTS MROWOPEN cellopts MROWCLOSE compoundTermList {
  char * s1 = mtex2MML_copy3("<mtd ", $3, ">");
  $$ = mtex2MML_copy3(s1, $5, "</mtd>");
  mtex2MML_free_string(s1);
  mtex2MML_free_string($3);
  mtex2MML_free_string($5);
};

cellopts: acellopt {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| cellopts acellopt {
  $$ = mtex2MML_copy3($1, " ", $2);
  mtex2MML_free_string($1);
  mtex2MML_free_string($2);
};

acellopt: colalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowalign {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| rowspan {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
}
| colspan {
  $$ = mtex2MML_copy_string($1);
  mtex2MML_free_string($1);
};

rowspan: ROWSPAN ATTRLIST {
  $$ = mtex2MML_copy2("rowspan=", $2);
  mtex2MML_free_string($2);
};

colspan: COLSPAN ATTRLIST {
  $$ = mtex2MML_copy2("columnspan=", $2);
  mtex2MML_free_string($2);
};

%%

/* see http://git.io/vk8Sz */
void envdata_copy(void *_dst, const void *_src)
{
  envdata_t *dst = (envdata_t*)_dst, *src = (envdata_t*)_src;
  dst->rowspacing = src->rowspacing ? string_dup(src->rowspacing) : NULL;
  dst->rowlines = src->rowlines ? string_dup(src->rowlines) : NULL;
  dst->environment_type = src->environment_type;
  utarray_new(dst->eqn_numbers, &ut_int_icd);
  utarray_concat(dst->eqn_numbers, src->eqn_numbers);
  dst->line_count = src->line_count;
}

void envdata_dtor(void *_elt)
{
  envdata_t *elt = (envdata_t*)_elt;
  if (elt->rowspacing) { free(elt->rowspacing); }
  if (elt->rowlines) { free(elt->rowlines); }
  if (elt->eqn_numbers) { utarray_free(elt->eqn_numbers); }
}

UT_icd envdata_icd = {sizeof(envdata_t), NULL, envdata_copy, envdata_dtor};

void format_additions(const char *buffer, const int options)
{
  utarray_new(environment_data_stack, &envdata_icd);

  if (colors == NULL)
    mtex2MML_create_css_colors(&colors);

  if (options)
    delimiter_options = options;
  else
    delimiter_options = MTEX2MML_DELIMITER_DEFAULT;

  encaseType *encase_pointer = (encaseType *)NONE;
  line_counter = 1;
  mtex2MML_env_replacements(&environment_data_stack, &encase_pointer, buffer);
  encase = (encaseType) encase_pointer;
}

void free_additions()
{
  utarray_free(environment_data_stack);

  struct css_colors *c = NULL, *tmp;

  HASH_ITER(hh, colors, c, tmp) {
    HASH_DEL(colors, c);
    free(c);
  }
}

char * mtex2MML_global_parse (const char * buffer, unsigned long length, const int options, const int global_start)
{
  global_label = global_start;
  return mtex2MML_parse(buffer, length, options);
}

char * mtex2MML_parse (const char * buffer, unsigned long length, const int options)
{
  char * mathml = 0;

  int result;

  format_additions(buffer, options);

  mtex2MML_setup (buffer, length);
  mtex2MML_restart ();

  result = mtex2MML_yyparse (&mathml);

  free_additions();

  /* See Bison documentation/maliciousness test: http://bit.ly/1IbpOja
     1 if parse error, 2 if memory exhuastion, etc. */
  if (result > 0) {
    if (mathml) {
      mtex2MML_free_string (mathml);
      mathml = 0;
    }
    return NULL;
  }

  return mathml;
}

UT_array ** mtex2MML_get_environment_data_stack()
{
  return &environment_data_stack;
}

int mtex2MML_filter (const char * buffer, unsigned long length, const int options)
{
  return mtex2MML_do_filter (buffer, length, 0, 0, options);
}

int mtex2MML_text_filter (const char * buffer, unsigned long length, const int options)
{
  return mtex2MML_do_filter (buffer, length, 0, 1, options);
}

int mtex2MML_strict_filter (const char * buffer, unsigned long length, const int options)
{
  return mtex2MML_do_filter (buffer, length, 1, 1, options);
}

static char * mtex2MML_last_error = 0;

static void mtex2MML_keep_error (const char * msg)
{
  if (mtex2MML_last_error) {
    mtex2MML_free_string (mtex2MML_last_error);
    mtex2MML_last_error = 0;
  }
  mtex2MML_last_error = mtex2MML_copy_escaped (msg);
}

int mtex2MML_delimiter_type(const int type)
{
  if (delimiter_options == MTEX2MML_DELIMITER_DEFAULT) {
    return type & (MTEX2MML_DELIMITER_DOLLAR | MTEX2MML_DELIMITER_DOUBLE);
  } else {
    return delimiter_options & type;
  }
}

int mtex2MML_do_filter (const char * buffer, unsigned long length, const int forbid_markup, const int write, const int options)
{
  global_label = 1;

  format_additions(buffer, options);

  int result = 0;
  int type = 0;
  int skip = 0;
  int match = 0;

  const char * ptr1 = buffer;
  const char * ptr2 = 0;

  const char * end = buffer + length;

  char * mathml = 0;

  void (*save_error_fn) (const char * msg) = mtex2MML_error;

  mtex2MML_error = mtex2MML_keep_error;

_until_math:
  ptr2 = ptr1;

  /* Search for the first math part */
  while (ptr2 < end) {
    if (*ptr2 == '$' && (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOLLAR) || mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOUBLE))) { break; }
    if ((*ptr2 == '\\') && (ptr2 + 5 < end) && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_ENVIRONMENTS)) {
      if (*(ptr2+1) == 'b' && *(ptr2+2) == 'e' && *(ptr2+3) == 'g' && *(ptr2+4) == 'i' && *(ptr2+5) == 'n') {
        break;
      }
    }
    if ((*ptr2 == '\\') && (ptr2 + 1 < end)) {
      if (*(ptr2+1) == '[' && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_BRACKETS)) { break; }
      if (*(ptr2+1) == '(' && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_PARENS)) { break; }
      if (*(ptr2+1) == '$' && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOLLAR)) { ptr2++; }
    }
    ++ptr2;
  }
  if (mtex2MML_write && ptr2 > ptr1 && write) {
    (*mtex2MML_write) (ptr1, ptr2 - ptr1);
  }

  if (ptr2 == end) { goto _finish; }

  /*_until_html:*/
  ptr1 = ptr2;

  if (ptr2 + 5 < end && (*ptr2 == '\\') && (*(ptr2+1) == 'b') && (*(ptr2+2) == 'e') && (*(ptr2+3) == 'g') && (*(ptr2+4) == 'i') && (*(ptr2+5) == 'n') && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_ENVIRONMENTS)) {
    type = MTEX2MML_DELIMITER_ENVIRONMENTS;
    ptr2 += 6;
  }
  else if (ptr2 + 1 < end) {
    if ((*ptr2 == '\\') && (*(ptr2+1) == '[' && *(ptr2 - 1) != '\\') && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_BRACKETS)) {
      type = MTEX2MML_DELIMITER_BRACKETS;
      ptr2 += 2;
    } else if ((*ptr2 == '$') && (*(ptr2+1) == '$') && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOUBLE)) {
      type = MTEX2MML_DELIMITER_DOUBLE;
      ptr2 += 2;
    } else if ((*ptr2 == '\\') && (*(ptr2+1) == '(' && *(ptr2 - 1) != '\\') && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_PARENS)) {
      type = MTEX2MML_DELIMITER_PARENS;
      ptr2 += 2;
    } else if ((*ptr2 == '$') && !isspace(*(ptr2+1)) && mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOLLAR)) {
      type = MTEX2MML_DELIMITER_DOLLAR;
      ptr2++;
    }
  } else { goto _finish; }

  skip = 0;
  match = 0;

  while (ptr2 < end) {
    switch (*ptr2) {
    case '<':
    case '>':
      if (forbid_markup == 1) { skip = 1; }
      break;

    case '\\':
      if (ptr2 + 1 < end) {
        if (*(ptr2 + 1) == '[' && type == MTEX2MML_DELIMITER_BRACKETS) {
          skip = 1;
        } else if (*(ptr2 + 1) == ']') {
          if (type == MTEX2MML_DELIMITER_BRACKETS) {
            ptr2 += 2;
            match = 1;
          } else {
            skip = 1;
          }
        } else if (*(ptr2 + 1) == '(' && type == MTEX2MML_DELIMITER_PARENS) {
          skip = 1;
        } else if (*(ptr2 + 1) == ')') {
          if (type == MTEX2MML_DELIMITER_PARENS) {
            ptr2 += 2;
            match = 1;
          } else {
            skip = 1;
          }
        }
        else if (ptr2 + 3 < end && type == MTEX2MML_DELIMITER_ENVIRONMENTS) {
          if (*(ptr2 + 1) == 'e' && *(ptr2 + 2) == 'n' && *(ptr2 + 3) == 'd') {
            ptr2 += 3;
            /* {env} can be many things, so consider it closed when we find the first } after \end*/
            while (ptr2 < end && *ptr2 != '}') { ptr2++; }
            ptr2++;  
            match = 1;
          }
        } else {
          ptr2++;
        }
      }
      break;

    case '\n':
      /* we hit a newline in an unclosed inline equation -- skip this string */
      if (type == MTEX2MML_DELIMITER_DOLLAR || type == MTEX2MML_DELIMITER_PARENS) {
        skip = 1;
      }
      break;

    case '$':
      if (type == MTEX2MML_DELIMITER_BRACKETS || type == MTEX2MML_DELIMITER_PARENS || type == MTEX2MML_DELIMITER_ENVIRONMENTS) {
        /* no op */
      } else if (ptr2 + 1 < end) {
        if (*(ptr2 + 1) == '$') {
          if (type == MTEX2MML_DELIMITER_DOLLAR) {
            ptr2++;
            match = 1;
          } else {
            ptr2 += 2;
            match = 1;
          }
        } else {
          if (type == MTEX2MML_DELIMITER_DOLLAR) {
            if (isspace(*(ptr2 - 1))) {
              skip = 1;
            } else if (isdigit(*(ptr2 + 1))) {
              if (write) {
                (*mtex2MML_write) (ptr1, ptr2 - ptr1 + 2);
              }
              skip = 1;
              ptr2 += 2;
              ptr1 = ptr2;
            } else {
              ptr2++;
              match = 1;
            }
          } else {
            if (*(ptr2 - 1) != '\\') {
              skip = 1;
            }
          }
        }
      } else {
        if (type == MTEX2MML_DELIMITER_DOLLAR) {
          ptr2++;
          match = 1;
        } else {
          skip = 1;
        }
      }
      break;

    default:
      break;
    }
    if (skip || match) { break; }

    ++ptr2;
  }
  if (skip) {
    if (type == MTEX2MML_DELIMITER_DOLLAR) {
      if (mtex2MML_write && write) {
        (*mtex2MML_write) (ptr1, 1);
      }
      ptr1++;
    } else {
      if (mtex2MML_write && write) {
        (*mtex2MML_write) (ptr1, 2);
      }
      ptr1 += 2;
    }
    goto _until_math;
  }
  if (match) {
    mtex2MML_setup (ptr1, ptr2 - ptr1);
    mtex2MML_restart ();

    mtex2MML_yyparse (&mathml);

    if (mathml) {
      if (mtex2MML_write_mathml) {
        (*mtex2MML_write_mathml) (mathml);
      }
      mtex2MML_free_string (mathml);
      mathml = 0;
    } else {
      ++result;
      if (mtex2MML_write && write) {
        /* A problematic error? Just leave the text alone. */
        (*mtex2MML_write) (ptr1, ptr2 - ptr1);
      }
    }
    ptr1 = ptr2;

    goto _until_math;
  }
  if (mtex2MML_write) {
    (*mtex2MML_write) (ptr1, ptr2 - ptr1);
  }

_finish:
  if (mtex2MML_last_error) {
    mtex2MML_free_string (mtex2MML_last_error);
    mtex2MML_last_error = 0;
  }
  mtex2MML_error = save_error_fn;

  free_additions();
  return result;
}
