%{
%}
%option noyywrap
%x MATH ATTRIBUTELIST BRACKETLIST PMBSTRING BBOLD FRAKTUR CALLIGRAPHIC SCRIPT ROMAN VERBTEXT PLAINTEXT PLAINTEXT_CURLY NUM INT1 INT2 INT3 MATHENV SVGENV SVGEND OPTARG ARRAYOPTIONS ENVOPTIONS ALIGNATOPTS COLOPTS ENCLOSEOPTS ENCLOSEATTRPARSE ENCLOSETEXTPARSE SPACEWIDTH SPACEHEIGHT SPACEDEPTH HSPACEARG TEXTOPTARG TEXTOPTARG1 RAISEBOX1 RAISEBOX2
%{
#include <string.h>

#define YYSTYPE char *

#include "parser.h"
#include "mtex2MML.h"
#include "environment.h"
#include "string_extras.h"

#define YY_NEVER_INTERACTIVE 1
#define YY_INPUT(buffer,result,max_size) result = s_mtex2MML_read (buffer, max_size);

#if defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__ || defined __DragonFly__
#undef fileno
#endif

extern int fileno(FILE *); /* avoids gcc warning */

static int s_mtex2MML_read (char * buffer, int max_size);

#define yylval mtex2MML_yylval
#define mtex2MML_optarg_depth 100

extern char * yylval;

int mtex2MML_lineno = 1;
int mtex2MML_inoptarg[mtex2MML_optarg_depth];
int mtex2MML_optarg_ind = 0;
int mtex2MML_env_start = 0;
int mtex2MML_variant_start = 0;

#define YY_USER_ACTION mtex2MML_capture_eqn_src();
#define mtex2MML_eqn_bufsize 131072
char mtex2MML_eqn_src[mtex2MML_eqn_bufsize];
char * mtex2MML_eqn_end = mtex2MML_eqn_src;
char mtex2MML_verbdelim;
char * mtex2MML_str;

int mtex2MML_rowposn = 0;

typedef enum {NO_PARSE = 0, OPEN_PARSE, CLOSE_PARSE} parsingEnv;
int parsing_environment = NO_PARSE;

envType environment_type;

/* TODO: I'm not sure what the next option does. My addition
   of `display_style` is to fix issue776. Probably worth merging
   these in the future? */
int mtex2MML_displaymode = 0;
int display_style = 0;

%}
%%
%{ if (mtex2MML_rowposn==1) mtex2MML_rowposn=0;
   if (mtex2MML_rowposn==2) mtex2MML_rowposn=1;
%}

 /* "Basic heuristic is: opening $ can't be followed by whitespace, closing $ can't be followed by
     a digit or preceded by whitespace." -- @jgm, http://bit.ly/1HVtemx */

<INITIAL>"$$" if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOUBLE)) {BEGIN(MATH); mtex2MML_inoptarg[0] = 0; mtex2MML_rowposn=2; mtex2MML_displaymode=1; return STARTDMATH; }

<INITIAL>"$"[\t\r\n ]? if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOLLAR)) {mtex2MML_inoptarg[0] = 0; mtex2MML_rowposn=2; if (yyleng==2) { BEGIN(ENDMATH); return INITIAL;}; BEGIN(MATH);  mtex2MML_displaymode=0; return STARTMATH; }

<INITIAL>"\\(" if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_PARENS)) {BEGIN(MATH); mtex2MML_inoptarg[0] = 0; mtex2MML_rowposn=2; mtex2MML_displaymode=0; return STARTMATH; }

<INITIAL>"\\[" if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_BRACKETS)) {BEGIN(MATH); mtex2MML_inoptarg[0] = 0; mtex2MML_rowposn=2; mtex2MML_displaymode=1; return STARTDMATH; }

<INITIAL>"\\begin" if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_ENVIRONMENTS)) {BEGIN(MATHENV); parsing_environment = OPEN_PARSE; mtex2MML_env_start = 1; mtex2MML_inoptarg[0] = 0; mtex2MML_rowposn=2; mtex2MML_displaymode=0; return BEGINENV; }

<INITIAL>EOF    return 0;
<INITIAL>"\n" {mtex2MML_lineno++; yylval=mtex2MML_copy_string(yytext); return CHAR;}
<INITIAL>.     {yylval=mtex2MML_copy_string(yytext); return CHAR;}


<MATH>{

"$"{1,2} if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOLLAR) || mtex2MML_delimiter_type(MTEX2MML_DELIMITER_DOUBLE)) { mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src-yyleng]=0; mtex2MML_eqn_end=mtex2MML_eqn_src; yylval=mtex2MML_copy_escaped(mtex2MML_eqn_src); BEGIN(INITIAL); return ENDMATH;} else {yylval=mtex2MML_copy_string("$"); return MI;}

"\\)"    if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_PARENS))   {mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src-yyleng]=0; mtex2MML_eqn_end=mtex2MML_eqn_src; yylval=mtex2MML_copy_escaped(mtex2MML_eqn_src); BEGIN(INITIAL); return ENDMATH;}

"\\]"    if (mtex2MML_delimiter_type(MTEX2MML_DELIMITER_BRACKETS)) {mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src-yyleng]=0; mtex2MML_eqn_end=mtex2MML_eqn_src; yylval=mtex2MML_copy_escaped(mtex2MML_eqn_src); BEGIN(INITIAL); return ENDMATH;}

"\\limits"    return LIMITS;
"\\nolimits"    return NOLIMITS;
"^"            return SUP;
"_"            return SUB;
"{"[\t\r\n ]*"}"   return EMPTYMROW;

"\\hline" |
"\\hdashline"      { /* Ignore */ ;}

[^\\]?%.+$               { /* Ignore */ ;}

"{"            {mtex2MML_rowposn = 2; if(mtex2MML_inoptarg[mtex2MML_optarg_ind] != 0) mtex2MML_inoptarg[mtex2MML_optarg_ind]++; return MROWOPEN;}
"}"            {if(mtex2MML_inoptarg[mtex2MML_optarg_ind] != 0) mtex2MML_inoptarg[mtex2MML_optarg_ind]--; return MROWCLOSE;}

"\\left"       mtex2MML_rowposn=2; return LEFT;
"\\right"      return RIGHT;

"\\big"   |
"\\bigr"  {return BIG;}
"\\Big"   |
"\\Bigr"  {return BBIG;}
"\\bigg"  |
"\\biggr" {return BIGG;}
"\\Bigg"  |
"\\Biggr" {return BBIGG;}

"\\bigl"  {return BIGL;}
"\\Bigl"  {return BBIGL;}
"\\biggl" {return BIGGL;}
"\\Biggl" {return BBIGGL;}

"\\bigm"  {return BIGM;}
"\\Bigm"  {return BBIGM;}
"\\biggm" {return BIGGM;}
"\\Biggm" {return BBIGGM;}

"\\mathrlap" |
"\\rlap"         {return RLAP;}
"\\mathllap" |
"\\llap"         {return LLAP;}
"\\mathclap" |
"\\clap"         {return CLAP;}
"\\ulap" |
"\\dlap"               {/* Ignore */ ;}


"&amp;" |
"&"         {mtex2MML_rowposn=2; return COLSEP;}

"\\&"           {yylval=mtex2MML_copy_string("&amp;"); return MI;}

"\\$"           {yylval=mtex2MML_copy_string("$"); return MI;}
"\\%"           {yylval=mtex2MML_copy_string("%"); return MI;}

"\\\\"[\n\r\t ]*\[[0-9\.]+(em|ex|px|in|cm|mm|mu|pt|pc|%)\] |
"\\newline"[\n\r\t ]* |
"\\cr"[\n\r\t ]*      |
"\\\\"[\n\r\t ]*          { mtex2MML_rowposn=2; if (mtex2MML_current_env_type(mtex2MML_get_environment_data_stack()) == OTHER) return NEWLINE; return ROWSEP;}

[0-9\.,]+(em|ex|px|in|cm|mm|mu|pt|pc|%) {char * normalized_px = mtex2MML_str_replace(yytext, ",", "."); yylval=mtex2MML_copy_string(normalized_px); mtex2MML_free_string(normalized_px); return PXSTRING; }

[a-zA-Z]       {yylval=mtex2MML_copy_string(yytext); return MI;}

[0-9]{1,3}(,[0-9]{3,3})*(\.[0-9]+)? |
[0-9]*[\.][0-9]+                   |
[0-9]+           {yylval=mtex2MML_copy_string(yytext); return MN;}
"\\infty"   |
"\\infinity"     {yylval=mtex2MML_copy_string("&infin;"); return MN;}

"("   |
"["             {yylval=mtex2MML_copy_string(yytext); return LEFTDELIM;}

")"   {yylval=mtex2MML_copy_string(yytext); return RIGHTDELIM;}
"]"   {if(mtex2MML_inoptarg[mtex2MML_optarg_ind]==1 && mtex2MML_optarg_ind > 0) {mtex2MML_inoptarg[mtex2MML_optarg_ind]=0; mtex2MML_optarg_ind--; return OPTARGCLOSE;} else {yylval=mtex2MML_copy_string(yytext); return RIGHTDELIM;}}

"\\"            {yylval=mtex2MML_copy_string("&#xA0;"); return MTEXT;}

"\\_"             {yylval=mtex2MML_copy_string("_"); return MI;}

"\\lbrace" |
"\\{"           {yylval=mtex2MML_copy_string("{"); return LEFTDELIM;}
"\\rbrace" |
"\\}"           {yylval=mtex2MML_copy_string("}"); return RIGHTDELIM;}

"\\vert" |
"|"             {yylval=mtex2MML_copy_string("&vert;"); return OTHERDELIM;}
"\\Vert" |
"\\|"           {yylval=mtex2MML_copy_string("&Vert;"); return OTHERDELIM;}

"\\lbrack"      {yylval=mtex2MML_copy_string("&#91;"); return LEFTDELIM;}
"\\rbrack"      {yylval=mtex2MML_copy_string("&#93;"); return RIGHTDELIM;}

"\\lVert"      {yylval=mtex2MML_copy_string("&#8741;"); return LEFTDELIM;}
"\\rVert"      {yylval=mtex2MML_copy_string("&#8741;"); return RIGHTDELIM;}

"\\lvert"      {yylval=mtex2MML_copy_string("&#8739;"); return LEFTDELIM;}
"\\rvert"      {yylval=mtex2MML_copy_string("&#8739;"); return RIGHTDELIM;}

"\\bracevert"   {yylval=mtex2MML_copy_string("&#9130;"); return OTHERDELIM;}

"\\setminus"   {yylval=mtex2MML_copy_string("&setminus;"); return MO;}
"\\backslash"   {yylval=mtex2MML_copy_string("&bsol;"); return MO;}
"\\smallsetminus"   {yylval=mtex2MML_copy_string("&#xFE68;"); return MOF;}

"\\sslash" {yylval=mtex2MML_copy_string("&parsl;"); return MO;}

"\\lfloor" |
"\\lceil"  |
"\\lang"   |
"\\langle"      {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return LEFTDELIM;}
"\\llangle"     {yylval=mtex2MML_copy_string("&Lang;"); return LEFTDELIM;}
"\\lgroup"      {yylval=mtex2MML_copy_string("&#10222;"); return LEFTDELIM;}

"\\rfloor" |
"\\rceil"  |
"\\rmoustache" |
"\\rang"   |
"\\rangle"      {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return RIGHTDELIM;}
"\\rrangle"     {yylval=mtex2MML_copy_string("&Rang;"); return RIGHTDELIM;}
"\\rgroup"      {yylval=mtex2MML_copy_string("&#10223;"); return RIGHTDELIM;}

"/"             {yylval=mtex2MML_copy_string(yytext); return OTHERDELIM;}

"\\uparrow"   |
"\\downarrow" |
"\\updownarrow" {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return OTHERDELIM;}

"."             {yylval=mtex2MML_copy_string(yytext); return PERIODDELIM;}

"-"    {if (mtex2MML_rowposn==1) return UNARYMINUS; else {yylval=mtex2MML_copy_string("&minus;"); return MO;}}

"+"    {if (mtex2MML_rowposn==1) return UNARYPLUS; else {yylval=mtex2MML_copy_string("+"); return MO;}}

"\\#" {yylval=mtex2MML_copy_string("#"); return MOL;}

","   {yylval=mtex2MML_copy_string(yytext); return COMMADELIM;}

":"   |
"!"   |
"="   |
"~"   |
";"   |
"?"   |
"#"   |
"`"   |
"*"        {yylval=mtex2MML_copy_string(yytext); return MOL;}

"\\prime"  {yylval=mtex2MML_copy_string("&prime;"); return MOLL;}
"'"        {yylval=mtex2MML_copy_string("&prime;"); return MOL;}
"''"     {yylval=mtex2MML_copy_string("&Prime;"); return MOL;}
"'''"  {yylval=mtex2MML_copy_string("&#8244;"); return MOL;}
"''''"  {yylval=mtex2MML_copy_string("&#8279;"); return MOL;}

"\\alpha"   |
"\\beta" |
"\\gamma" |
"\\delta" |
"\\zeta" |
"\\eta"  |
"\\theta" |
"\\iota"  |
"\\kappa" |
"\\lambda" |
"\\mu"     |
"\\nu"    |
"\\xi"   |
"\\pi"     |
"\\rho"   |
"\\sigma"  |
"\\tau"   |
"\\upsilon" |
"\\chi"  |
"\\psi"  |
"\\omega" |
"\\backepsilon"   |
"\\varkappa" |
"\\varpi" |
"\\varrho" |
"\\varsigma" |
"\\vartheta"      {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return MI;}

"\\varepsilon" {yylval=mtex2MML_copy_string("&#x03B5;"); return MI;}
"\\phi"  {yylval=mtex2MML_copy_string("&#x03D5;"); return MI;}
"\\varphi"  {yylval=mtex2MML_copy_string("&#x03C6;"); return MI;}

"\\varDelta" {yylval=mtex2MML_copy_string("&#x394;"); return MI;}
"\\varGamma" {yylval=mtex2MML_copy_string("&#x393;"); return MI;}
"\\varLambda" {yylval=mtex2MML_copy_string("&#x39B;"); return MI;}
"\\varOmega" {yylval=mtex2MML_copy_string("&#x3A9;"); return MI;}
"\\varPhi" {yylval=mtex2MML_copy_string("&#x3A6;"); return MI;}
"\\varPi" {yylval=mtex2MML_copy_string("&#x3A0;"); return MI;}
"\\varPsi" {yylval=mtex2MML_copy_string("&#x3A8;"); return MI;}
"\\varSigma" {yylval=mtex2MML_copy_string("&#x3A3;"); return MI;}
"\\varTheta" {yylval=mtex2MML_copy_string("&#x398;"); return MI;}
"\\varUpsilon" {yylval=mtex2MML_copy_string("&#x3A5;"); return MI;}
"\\varXi" {yylval=mtex2MML_copy_string("&#x39E;"); return MI;}

"\\arccos"  |
"\\arcsin"  |
"\\arctan"  |
"\\arg"     |
"\\cos"     |
"\\cosh"    |
"\\cot"     |
"\\coth"    |
"\\csc"     |
"\\deg"     |
"\\dim"     |
"\\exp"     |
"\\hom"     |
"\\ker"     |
"\\lg"      |
"\\ln"      |
"\\log"     |
"\\sec"     |
"\\sin"     |
"\\sinh"    |
"\\tan"     |
"\\tanh"    {yylval=mtex2MML_copy_string(yytext+1); return MI;}

"\\injlim"  {yylval=mtex2MML_copy_string("inj lim"); return MO;}
"\\projlim" {yylval=mtex2MML_copy_string("proj lim"); return MO;}

"\\varinjlim"   return VARINJLIM;
"\\varliminf"   return VARLIMINF;
"\\varlimsup"   return VARLIMSUP;
"\\varprojlim"  return VARPROJLIM;

"\\det"     |
"\\gcd"     |
"\\inf"     |
"\\lim"     |
"\\liminf"  |
"\\limsup"  |
"\\max"     |
"\\min"     |
"\\Pr"      |
"\\sup"    {yylval=mtex2MML_copy_string(yytext+1); return MIB;}

"\\omicron"  {yylval=mtex2MML_copy_string("&oscr;"); return MI;}

"\\epsilon"  {yylval=mtex2MML_copy_string("&epsi;"); return MI;}

"\\cdot"   {yylval=mtex2MML_copy_string("&sdot;"); return MO;}
"\\cdotp"   {yylval=mtex2MML_copy_string("&middot;"); return MO;}
"\\centerdot"   {yylval=mtex2MML_copy_string("&centerdot;"); return MO;}

"\\dotsb"   |
"\\dotsi"   |
"\\dotsm"     {yylval=mtex2MML_copy_string("&#x22EF;"); return MI;}

"\\ldotp"   |
"\\dotsc"   |
"\\dotso"     {yylval=mtex2MML_copy_string("&#x2026;"); return MI;}

"\\Alpha"   |
"\\Beta" |
"\\Delta"  |
"\\Gamma"  |
"\\digamma"  |
"\\Lambda" |
"\\Pi"     |
"\\Phi"    |
"\\Psi"    |
"\\Sigma"  |
"\\Theta"  |
"\\Xi"     |
"\\Zeta" |
"\\Eta"  |
"\\Iota"  |
"\\Kappa" |
"\\Mu"     |
"\\Nu"    |
"\\Rho"   |
"\\Tau"  |
"\\mho"  |
"\\Omega"     {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return MI;}

"\\Upsilon" |
"\\Upsi" {yylval=mtex2MML_copy_string("&Upsi;"); return MI;}

"\\iff" |
"\\Longleftrightarrow"  {yylval=mtex2MML_copy_string("&iff;"); return MOL;}

"\\longleftrightarrow"  {yylval=mtex2MML_copy_string("&#10231;"); return MOL;}

"\\Leftrightarrow"  {yylval=mtex2MML_copy_string("&Leftrightarrow;"); return MOL;}

"\\impliedby" |
"\\Leftarrow"   {yylval=mtex2MML_copy_string("&Leftarrow;"); return MOL;}

"\\implies" |
"\\Rightarrow"   {yylval=mtex2MML_copy_string("&Rightarrow;"); return MOL;}

"\\hookleftarrow" {yylval=mtex2MML_copy_string("&hookleftarrow;"); return MOL;}

"\\embedsin" |
"\\hookrightarrow"  {yylval=mtex2MML_copy_string("&hookrightarrow;"); return MOL;}

"\\longleftarrow"   {yylval=mtex2MML_copy_string("&longleftarrow;"); return MOL;}

"\\Longleftarrow"   {yylval=mtex2MML_copy_string("&#10232;"); return MOL;}

"\\longrightarrow"  {yylval=mtex2MML_copy_string("&longrightarrow;"); return MOL;}

"\\Longrightarrow"  {yylval=mtex2MML_copy_string("&#10233;"); return MOL;}

"\\leftarrow"   {yylval=mtex2MML_copy_string("&leftarrow;"); return MOL;}

"\\to"    |
"\\rightarrow"  {yylval=mtex2MML_copy_string("&rightarrow;"); return MOL;}

"\\leftrightarrow"  {yylval=mtex2MML_copy_string("&leftrightarrow;"); return MOL;}

"\\mapsto"  |
"\\map"       {yylval=mtex2MML_copy_string("&map;"); return MOL;}

"\\nearrow" |
"\\nearr"    {yylval=mtex2MML_copy_string("&nearr;"); return MOL;}

"\\nwarrow" |
"\\nwarr"     {yylval=mtex2MML_copy_string("&nwarr;"); return MOL;}

"\\searrow" |
"\\searr"     {yylval=mtex2MML_copy_string("&searr;"); return MOL;}

"\\swarrow" |
"\\swarr"     {yylval=mtex2MML_copy_string("&swarr;"); return MOL;}

"\\neArrow" |
"\\neArr"    {yylval=mtex2MML_copy_string("&neArr;"); return MOL;}

"\\nwArrow" |
"\\nwArr"     {yylval=mtex2MML_copy_string("&nwArr;"); return MOL;}

"\\seArrow" |
"\\seArr"     {yylval=mtex2MML_copy_string("&seArr;"); return MOL;}

"\\swArrow" |
"\\swArr"     {yylval=mtex2MML_copy_string("&swArr;"); return MOL;}

"\\darr"     {yylval=mtex2MML_copy_string("&darr;"); return MOL;}

"\\Downarrow" {yylval=mtex2MML_copy_string("&dArr;"); return MOL;}

"\\Arrowvert" {yylval=mtex2MML_copy_string("&#8214;"); return MOL;}

"\\arrowvert" {yylval=mtex2MML_copy_string("&#9168;"); return MOL;}

"\\uparr"    {yylval=mtex2MML_copy_string("&uarr;"); return MOL;}

"\\Uparrow" {yylval=mtex2MML_copy_string("&uArr;"); return MOL;}

"\\downuparrow" |
"\\duparr" |
"\\updarr"  {yylval=mtex2MML_copy_string("&updownarrow;"); return MOL;}

"\\Updownarrow"  {yylval=mtex2MML_copy_string("&Updownarrow;"); return MOL;}

"\\leftsquigarrow"       {yylval=mtex2MML_copy_string("&#x21DC;"); return MOL;}
"\\rightsquigarrow"      {yylval=mtex2MML_copy_string("&zigrarr;"); return MOL;}
"\\dashleftarrow"        {yylval=mtex2MML_copy_string("&#x290E;"); return MOL;}
"\\dashrightarrow"       {yylval=mtex2MML_copy_string("&#x290F;"); return MOL;}
"\\curvearrowbotright"   {yylval=mtex2MML_copy_string("&#x293B;"); return MOL;}
"\\righttoleftarrow"     {yylval=mtex2MML_copy_string("&#x27F2;"); return MOL;}
"\\lefttorightarrow"     {yylval=mtex2MML_copy_string("&#x27F3;"); return MOL;}
"\\leftrightsquigarrow" |
"\\upuparrows"          |
"\\rightleftarrows"     |
"\\rightrightarrows"    |
"\\curvearrowleft"      |
"\\curvearrowright"     |
"\\downdownarrows"      |
"\\leftarrowtail"       |
"\\rightarrowtail"      |
"\\leftleftarrows"      |
"\\leftrightarrows"     |
"\\Lleftarrow"          |
"\\Rrightarrow"         |
"\\looparrowleft"       |
"\\looparrowright"      |
"\\Lsh"                 |
"\\Rsh"                 |
"\\circlearrowleft"     |
"\\circlearrowright"    |
"\\twoheadleftarrow"    |
"\\twoheadrightarrow"   |
"\\nLeftarrow"          |
"\\nleftarrow"          |
"\\nLeftrightarrow"     |
"\\nleftrightarrow"     |
"\\nRightarrow"         |
"\\nrightarrow"         |
"\\rightharpoonup"      |
"\\rightharpoondown"    |
"\\leftharpoonup"       |
"\\leftharpoondown"     |
"\\downharpoonleft"     |
"\\downharpoonright"    |
"\\leftrightharpoons"   |
"\\rightleftharpoons"   |
"\\upharpoonleft"       |
"\\upharpoonright"   {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return MOL;}

"\\xrightarrow"        |
"\\xleftarrow"         |
"\\xleftrightarrow"    |
"\\xLeftarrow"         |
"\\xRightarrow"        |
"\\xLeftrightarrow"    |
"\\xleftrightharpoons" |
"\\xrightleftharpoons" |
"\\xhookleftarrow"     |
"\\xhookrightarrow"    |
"\\xtwoheadrightarrow"    |
"\\xtwoheadleftarrow"    |
"\\xmapsto" {BEGIN(OPTARG); yylval = mtex2MML_copy3("&", yytext+2, ";"); return XARROW;}

"\\xlongequal" {BEGIN(OPTARG); yylval=mtex2MML_copy_string("&#61;"); return XARROW;}

"\\xtofrom"    {BEGIN(OPTARG); yylval=mtex2MML_copy_string("&#8644;"); return XARROW;}

"\\dots" |
"\\ldots"  {yylval=mtex2MML_copy_string("&hellip;"); return MI;}
"\\cdots"   {yylval=mtex2MML_copy_string("&ctdot;"); return MI;}
"\\ddots"   {yylval=mtex2MML_copy_string("&dtdot;"); return MI;}
"\\udots"   {yylval=mtex2MML_copy_string("&utdot;"); return MI;}
"\\vdots"   {yylval=mtex2MML_copy_string("&vellip;"); return MI;}
"\\colon"   {yylval=mtex2MML_copy_string("&colon;"); return MOR;}

"\\cup"  |
"\&union;" |
"\\union"          {yylval=mtex2MML_copy_string("&cup;"); return MOL;}

"\\bigcup" |
"\\Union"  |
"\&Union;"      {yylval=mtex2MML_copy_string("&Union;"); return MOB;}

"\\cap"  |
"\\intersection"          {yylval=mtex2MML_copy_string("&cap;"); return MOL;}

"\\bigcap" |
"\\Intersection"       {yylval=mtex2MML_copy_string("&Intersection;"); return MOB;}

"\\in"           {yylval=mtex2MML_copy_string("&Element;"); return MOL;}

 /* For some reason, Firefox does not put in the right spacing around some of these operators
  * unless we ask for it, explicitly.
  */
"\\coloneqq"     {yylval=mtex2MML_copy_string("&colone;"); return MOL;}
"\\Coloneqq"     {yylval=mtex2MML_copy_string("&Colone;"); return MOS;}
"\\coloneq"      {yylval=mtex2MML_copy_string("&#x2236;&#x2212;"); return MOS;}
"\\Coloneq"      {yylval=mtex2MML_copy_string("&#x2237;&#x2212;"); return MOS;}
"\\eqqcolon"     {yylval=mtex2MML_copy_string("&ecolon;"); return MOS;}
"\\Eqqcolon"     {yylval=mtex2MML_copy_string("&#x003d;&#x2237;"); return MOS;}
"\\eqcolon"      {yylval=mtex2MML_copy_string("&#x2239;"); return MOS;}
"\\Eqcolon"      {yylval=mtex2MML_copy_string("&#x2212;&#x2237;"); return MOS;}
"\\colonapprox"  {yylval=mtex2MML_copy_string("&#x2236;&#x2248;"); return MOS;}
"\\Colonapprox"  {yylval=mtex2MML_copy_string("&#x2237;&#x2248;"); return MOS;}
"\\colonsim"     {yylval=mtex2MML_copy_string("&#x2236;&#x223C;"); return MOS;}
"\\Colonsim"     {yylval=mtex2MML_copy_string("&#x2237;&#x223C;"); return MOS;}
"\\dblcolon"     {yylval=mtex2MML_copy_string("&Colon;"); return MOL;}

"\\ast"  |
"\\Cap"  |
"\\Cup"  |
"\\circledast"  |
"\\circledcirc" |
"\\curlyvee"    |
"\\curlywedge"  |
"\\divideontimes" |
"\\dotplus" |
"\\leftthreetimes" |
"\\rightthreetimes" |
"\\veebar" |
"\\gt"   |
"\\lt"   |
"\\approxeq"   |
"\\backsim"   |
"\\backsimeq"   |
"\\barwedge"    |
"\\doublebarwedge" |
"\\subset"  |
"\\subseteq" |
"\\subseteqq" |
"\\subsetneq" |
"\\subsetneqq" |
"\\varsubsetneq" |
"\\varsubsetneqq" |
"\\prec"     |
"\\parallel"  |
"\\nparallel" |
"\\shortparallel" |
"\\nshortparallel" |
"\\perp"  |
"\\eqslantgtr" |
"\\eqslantless" |
"\\gg" |
"\\ggg" |
"\\geq" |
"\\geqq" |
"\\geqslant" |
"\\gneq" |
"\\gneqq" |
"\\gnapprox" |
"\\gnsim" |
"\\gtrapprox" |
"\\ge"  |
"\\le"  |
"\\leq" |
"\\leqq" |
"\\leqslant" |
"\\lessapprox" |
"\\lessdot" |
"\\lesseqgtr" |
"\\lesseqqgtr" |
"\\lessgtr" |
"\\lneq" |
"\\lneqq" |
"\\lnsim" |
"\\lvertneqq" |
"\\gtrsim" |
"\\gtrdot" |
"\\gtreqless" |
"\\gtreqqless" |
"\\gtrless" |
"\\gvertneqq" |
"\\lesssim" |
"\\lnapprox" |
"\\nsubset" |
"\\nsubseteq" |
"\\nsubseteqq" |
"\\notin" |
"\\ni" |
"\\notni" |
"\\nmid" |
"\\nshortmid" |
"\\preceq" |
"\\npreceq" |
"\\ll" |
"\\ngeq" |
"\\ngeqq" |
"\\ngeqslant" |
"\\nleq" |
"\\nleqq" |
"\\nleqslant" |
"\\nless" |
"\\supset" |
"\\supseteq" |
"\\supseteqq" |
"\\supsetneq" |
"\\supsetneqq" |
"\\varsupsetneq" |
"\\varsupsetneqq" |
"\\approx" |
"\\asymp" |
"\\bowtie" |
"\\dashv" |
"\\Vdash" |
"\\vDash" |
"\\VDash" |
"\\vdash" |
"\\Vvdash" |
"\\models" |
"\\sim" |
"\\simeq" |
"\\nsim" |
"\\smile" |
"\\triangle" |
"\\triangledown" |
"\\triangleleft" |
"\\cong" |
"\\succ" |
"\\nsucc" |
"\\ngtr" |
"\\nsupset" |
"\\nsupseteq" |
"\\propto" |
"\\equiv" |
"\\nequiv" |
"\\frown" |
"\\triangleright" |
"\\ncong" |
"\\succeq" |
"\\succapprox" |
"\\succnapprox" |
"\\succcurlyeq" |
"\\succsim" |
"\\succnsim" |
"\\nsucceq" |
"\\nvDash" |
"\\nvdash" |
"\\nVDash" |
"\\amalg" |
"\\pm" |
"\\mp" |
"\\bigcirc" |
"\\wr" |
"\\odot" |
"\\uplus" |
"\\clubsuit" |
"\\spadesuit" |
"\\Diamond" |
"\\diamond" |
"\\sqcup" |
"\\sqcap" |
"\\sqsubset" |
"\\sqsubseteq" |
"\\sqsupset" |
"\\sqsupseteq" |
"\\Subset" |
"\\Supset" |
"\\ltimes" |
"\\div" |
"\\rtimes" |
"\\bot" |
"\\therefore" |
"\\thickapprox" |
"\\thicksim" |
"\\varpropto" |
"\\varnothing" |
"\\flat" |
"\\vee" |
"\\because" |
"\\between" |
"\\Bumpeq" |
"\\bumpeq" |
"\\circeq" |
"\\curlyeqprec" |
"\\curlyeqsucc" |
"\\doteq" |
"\\doteqdot" |
"\\eqcirc" |
"\\fallingdotseq" |
"\\multimap" |
"\\pitchfork" |
"\\precapprox" |
"\\precnapprox" |
"\\preccurlyeq" |
"\\precsim" |
"\\precnsim" |
"\\risingdotseq" |
"\\sharp" |
"\\bullet" |
"\\nexists" |
"\\dagger" |
"\\ddagger" |
"\\top" |
"\\natural" |
"\\angle" |
"\\measuredangle" |
"\\backprime" |
"\\bigstar" |
"\\blacklozenge" |
"\\lozenge" |
"\\blacksquare" |
"\\blacktriangle" |
"\\blacktriangleleft" |
"\\blacktriangleright" |
"\\blacktriangledown" |
"\\ntriangleleft" |
"\\ntriangleright" |
"\\ntrianglelefteq" |
"\\ntrianglerighteq" |
"\\trianglelefteq" |
"\\trianglerighteq" |
"\\triangleq" |
"\\vartriangleleft" |
"\\vartriangleright" |
"\\forall" |
"\\bigtriangleup" |
"\\bigtriangledown" |
"\\nprec"  {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return MOL;}

"\\textgreater" |
" > "               {yylval=mtex2MML_copy_string("&gt;"); return MOL;}
"\\textless"    |
" < "               {yylval=mtex2MML_copy_string("&lt;"); return MOL;}
"\\land"  {yylval=mtex2MML_copy_string("&and;"); return MOL;}
"\\lor"   {yylval=mtex2MML_copy_string("&or;"); return MOL;}
"\\surd"  {yylval=mtex2MML_copy_string("&radic;"); return MOL;}
"\\And"   {yylval=mtex2MML_copy_string("&amp;"); return MOL;}
"\\checkmark" {yylval=mtex2MML_copy_string("&#10003;"); return MOL;}
"\\circledR" {yylval=mtex2MML_copy_string("&reg;"); return MOL;}
"\\circledS" {yylval=mtex2MML_copy_string("&#9416;"); return MOL;}
"\\complement" {yylval=mtex2MML_copy_string("&#8705;"); return MOL;}
"\\diagdown" {yylval=mtex2MML_copy_string("&#9586;"); return MOL;}
"\\diagup" {yylval=mtex2MML_copy_string("&#9585;"); return MOL;}
"\\doublecap" {yylval=mtex2MML_copy_string("&#8914;"); return MOL;}
"\\doublecup" {yylval=mtex2MML_copy_string("&#8915;"); return MOL;}
"\\eqsim" {yylval=mtex2MML_copy_string("&#8770;"); return MOL;}
"\\Finv" {yylval=mtex2MML_copy_string("&#8498;"); return MOL;}
"\\Game" {yylval=mtex2MML_copy_string("&#8513;"); return MOL;}
"\\gggtr" {yylval=mtex2MML_copy_string("&#8921;"); return MOL;}
"\\gets" {yylval=mtex2MML_copy_string("&#x2190;"); return MOL;}
"\\hslash" {yylval=mtex2MML_copy_string("&#x210F;"); return MOL;}
"\\intop" {yylval=mtex2MML_copy_string("&#x222B;"); return MOL;}
"\\Join" {yylval=mtex2MML_copy_string("&#x22C8;"); return MOL;}
"\\leadsto" {yylval=mtex2MML_copy_string("&#x21DD;"); return MOL;}
"\\llcorner"  {yylval=mtex2MML_copy_string("&#x231E;"); return MOL;}
"\\lrcorner"  {yylval=mtex2MML_copy_string("&#x231F;"); return MOL;}
"\\llless"  {yylval=mtex2MML_copy_string("&#x22D8;"); return MOL;}
"\\lmoustache"  {yylval=mtex2MML_copy_string("&#x23B0;"); return MOL;}
"\\lnot"  {yylval=mtex2MML_copy_string("&#xAC;"); return MOL;}
"\\longmapsto"  {yylval=mtex2MML_copy_string("&#x27FC;"); return MOL;}
"\\maltese" {yylval=mtex2MML_copy_string("&#x2720;"); return MOL;}
"\\nsupseteqq" {yylval=mtex2MML_copy_string("&#x2289;"); return MOL;}
"\\nVdash" {yylval=mtex2MML_copy_string("&#x22AE;"); return MOL;}
"\\owns" {yylval=mtex2MML_copy_string("&#x220B;"); return MOL;}
"\\precneqq" {yylval=mtex2MML_copy_string("&#x2AB5;"); return MOL;}
"\\restriction" {yylval=mtex2MML_copy_string("&#x21BE;"); return MOL;}
"\\smallint" {yylval=mtex2MML_copy_string("&#x222B;"); return MOL;}
"\\succneqq" {yylval=mtex2MML_copy_string("&#x2AB6;"); return MOL;}
"\\ulcorner" {yylval=mtex2MML_copy_string("&#x231C;"); return MOL;}
"\\urcorner" {yylval=mtex2MML_copy_string("&#x231D;"); return MOL;}
"\\vartriangle" {yylval=mtex2MML_copy_string("&#x25B3;"); return MOL;}
"\\yen" {yylval=mtex2MML_copy_string("&#xA5;"); return MOL;}

"\\TeX"   {return TEXSYMBOL;}
"\\LaTeX" {return LATEXSYMBOL;}

"\\aleph" |
"\\beth" |
"\\gimel" |
"\\daleth" |
"\\eth" |
"\\ell" |
"\\hbar" |
"\\Im" |
"\\imath" |
"\\jmath" |
"\\wp" |
"\\Re" {yylval=mtex2MML_copy2(yytext, ";"); if (yylval != mtex2MML_empty_string) yylval[0]='&'; return MI;}

"\\Perp" |
"\\Vbar" {yylval=mtex2MML_copy_string("&Vbar;"); return MOL;}
"\\boxdot" {yylval=mtex2MML_copy_string("&sdotb;"); return MOL;}
"\\Doteq" {yylval=mtex2MML_copy_string("&#8785;"); return MOL;}
"\\Box" |
"\\square" {yylval=mtex2MML_copy_string("&square;"); return MOL;}
"\\emptyset" |
"\\empty" {yylval=mtex2MML_copy_string("&emptyv;"); return MI;}
"\\exists" {yylval=mtex2MML_copy_string("&Exists;"); return MOL;}
"\\circ" {yylval=mtex2MML_copy_string("&SmallCircle;"); return MOL;}
"\\rhd" {yylval=mtex2MML_copy_string("&RightTriangle;"); return MOL;}
"\\lhd" {yylval=mtex2MML_copy_string("&LeftTriangle;"); return MOL;}
"\\lll" {yylval=mtex2MML_copy_string("&Ll;"); return MOL;}
"\\unrhd" {yylval=mtex2MML_copy_string("&RightTriangleEqual;"); return MOL;}
"\\unlhd" {yylval=mtex2MML_copy_string("&LeftTriangleEqual;"); return MOL;}
"\\Del" |
"\\nabla" {yylval=mtex2MML_copy_string("&Del;"); return MOL;}
"\\sphericalangle" {yylval=mtex2MML_copy_string("&angsph;"); return MOL;}

"\\heartsuit" {yylval=mtex2MML_copy_string("&#x2661;"); return MOL;}
"\\diamondsuit" {yylval=mtex2MML_copy_string("&#x2662;"); return MOL;}

"\\partial" {yylval=mtex2MML_copy_string("&PartialD;"); return MOL;}

"\\qed" {yylval=mtex2MML_copy_string("&squf;"); return MO;}

"\\mod" {yylval=mtex2MML_copy_string("mod"); return MOS;}
"\\pmod" {return PMOD;}
"\\bmod" {return BMOD;}
"\\pod"  {return POD;}

"\\bottom" {yylval=mtex2MML_copy_string("&perp;"); return MOL;}

"\\neg" {yylval=mtex2MML_copy_string("&not;"); return MOL;}

"\\neq" |
"\\ne"   {yylval=mtex2MML_copy_string("&ne;"); return MOL;}

"\\shortmid" |
"\\mid"  {yylval=mtex2MML_copy_string("&VerticalBar;"); return MOS;}

"\\int"    |
"\\integral"     {yylval=mtex2MML_copy_string("&Integral;"); return MOL;}

"\\iint"   |
"\\doubleintegral" {yylval=mtex2MML_copy_string("&Int;"); return MOL;}

"\\iiint"   |
"\\tripleintegral" {yylval=mtex2MML_copy_string("&tint;"); return MOL;}

"\\iiiint"   |
"\\quadrupleintegral" {yylval=mtex2MML_copy_string("&qint;"); return MOL;}

"\\idotsint" {yylval=mtex2MML_copy_string("&#x222B; &#x22EF; &#x222B;"); return MOL;}

"\\oint"    |
"\\conint"  |
"\\contourintegral"   {yylval=mtex2MML_copy_string("&conint;"); return MOL;}

"\\times"   {yylval=mtex2MML_copy_string("&times;"); return MOL;}
"\\star"   {yylval=mtex2MML_copy_string("&sstarf;"); return MOL;}
"\\circleddash" |
"\\odash"   {yylval=mtex2MML_copy_string("&odash;"); return MOL;}

"\\intercal"   {yylval=mtex2MML_copy_string("&intcal;"); return MOL;}
"\\smallfrown"   {yylval=mtex2MML_copy_string("&sfrown;"); return MOL;}
"\\smallsmile"   {yylval=mtex2MML_copy_string("&ssmile;"); return MOL;}

"\\boxminus" |
"\\minusb"   {yylval=mtex2MML_copy_string("&boxminus;"); return MOL;}
"\\boxplus" |
"\\plusb"   {yylval=mtex2MML_copy_string("&boxplus;"); return MOL;}
"\\boxtimes" |
"\\timesb"   {yylval=mtex2MML_copy_string("&boxtimes;"); return MOL;}

"\\sideset"  {return SIDESET;}

"\\sum"          {yylval=mtex2MML_copy_string("&Sum;"); return MOB;}

"\\prod" |
"\\product"      {yylval=mtex2MML_copy_string("&prod;"); return MOB;}

"\\coprod"  |
"\\coproduct"    {yylval=mtex2MML_copy_string("&coprod;"); return MOB;}

"\\otimes"       {yylval=mtex2MML_copy_string("&otimes;"); return MOL;}
"\\Otimes"       {yylval=mtex2MML_copy_string("&Otimes;"); return MOL;}
"\\bigotimes"     {yylval=mtex2MML_copy_string("&bigotimes;"); return MOL;}

"\\ominus"       {yylval=mtex2MML_copy_string("&ominus;"); return MOL;}
"\\oslash"       {yylval=mtex2MML_copy_string("&osol;"); return MOL;}

"\\oplus"       {yylval=mtex2MML_copy_string("&oplus;"); return MOL;}
"\\Oplus"   |
"\\bigoplus"    {yylval=mtex2MML_copy_string("&bigoplus;"); return MOB;}

"\\bigodot"    {yylval=mtex2MML_copy_string("&bigodot;"); return MOB;}
"\\bigsqcup"    {yylval=mtex2MML_copy_string("&bigsqcup;"); return MOB;}
"\\bigsqcap"    {yylval=mtex2MML_copy_string("&#x2A05;"); return MOB;}
"\\biginterleave" {yylval=mtex2MML_copy_string("&#x2AFC;"); return MOB;}
"\\biguplus"    {yylval=mtex2MML_copy_string("&biguplus;"); return MOB;}

"\\wedge"       {yylval=mtex2MML_copy_string("&wedge;"); return MOL;}
"\\Wedge"  |
"\\bigwedge"       {yylval=mtex2MML_copy_string("&xwedge;"); return MOB;}

"\\Vee"  |
"\\bigvee"       {yylval=mtex2MML_copy_string("&bigvee;"); return MOB;}

"\\invamp"  |
"\\parr"       {yylval=mtex2MML_copy_string("&#8523;"); return MOL;}

"\\frac"         {return FRAC;}
"\\tfrac"        {return TFRAC;}
"\\dfrac"        {return DFRAC;}
"\\cfrac"        {return CFRAC;}
"\\genfrac"      {return GENFRAC;}

"\\binom"        {return BINOM;}
"\\tbinom"        {return TBINOM;}
"\\brace"        {return BRACE;}
"\\brack"        {return BRACK;}
"\\choose"        {return CHOOSE;}
"\\dbinom"        {return DBINOM;}

"\\tensor"       {return TENSOR;}

"\\multiscripts" {return MULTI;}

"\\overbrace"     {return OVERBRACE;}

"\\overbracket"     {return OVERBRACKET;}

"\\underbrace"   {return UNDERBRACE;}

"\\underbracket"   {return UNDERBRACKET;}

"\\underline"   {return UNDERLINE;}

"\\underleftarrow" { return UNDERLEFTARROW;}

"\\underleftrightarrow" { return UNDERLEFTRIGHTARROW;}

"\\underrightarrow" { return UNDERRIGHTARROW;}

"\\bar"          {return BAR;}
"\\overline" |
"\\closure" |
"\\widebar"      {return WIDEBAR;}

"\\skew"         {return SKEW; }

"\\acute"        {return ACUTE;}

"\\grave"        {return GRAVE;}

"\\breve"        {return BREVE;}

"\\mathring"     {return MATHRING;}

"\\vec"       {return VEC;}
"\\widevec"   {return WIDEVEC;}

"\\dot"       {return DOT;}
"\\ddot"      {return DDOT;}
"\\dddot"      {return DDDOT;}
"\\ddddot"      {return DDDDOT;}

"\\tilde"       {return TILDE;}
"\\widetilde"   {return WIDETILDE;}


"\\check"       {return CHECK;}
"\\widecheck"   {return WIDECHECK;}


"\\hat"       {return HAT;}
"\\widehat"   {return WIDEHAT;}

"\\underset"     {return UNDER;}

"\\buildrel"   {return BUILDREL;}

"\\stackrel" |
"\\overset"      {return OVER;}

"\\over"         {return TEXOVER;}
"\\overwithdelims"         {return TEXOVERWITHDELIMS;}
"\\atop"         {return TEXATOP;}
"\\atopwithdelims"         {return TEXATOPWITHDELIMS;}
"\\above"        {return TEXABOVE;}
"\\abovewithdelims"        {return TEXABOVEWITHDELIMS;}

"\\overleftarrow" {return OVERLEFTARROW;}

"\\overleftrightarrow" {return OVERLEFTRIGHTARROW;}

"\\overrightarrow" {return OVERRIGHTARROW;}

"\\underoverset"  {return UNDEROVER;}

"\\sqrt"         {BEGIN(OPTARG); return SQRT;}

"\\root"         {return ROOT;}
"\\leftroot"         {BEGIN(NUM); return LEFTROOT;}
"\\uproot"         {BEGIN(NUM); return UPROOT;}

"\\of"           {return OF;}

"\\text"          {BEGIN(PLAINTEXT); return TEXTBOX;}
"\\verb"[^ ]      {mtex2MML_verbdelim = yytext[yyleng-1]; mtex2MML_str = mtex2MML_copy_string(""); BEGIN(VERBTEXT); return VERBBOX;}

"\\statusline"    {BEGIN(PLAINTEXT); return STATLINE;}
"\\tooltip"       {BEGIN(PLAINTEXT); return TOOLTIP;}

"\\toggle"        {return TOGGLE;}
"\\begintoggle"   {return TOGGLESTART;}
"\\endtoggle"     {return TOGGLEEND;}

"\\mathraisebox"  {BEGIN(RAISEBOX1); return RAISEBOX;}

"\\fghilight" |
"\\fghighlight"   {BEGIN(ATTRIBUTELIST); return FGHIGHLIGHT;}

"\\bghilight" |
"\\bghighlight"   {BEGIN(ATTRIBUTELIST); return BGHIGHLIGHT;}

"\\colorbox"     {BEGIN(ATTRIBUTELIST); return COLORBOX;}

"\\color"        {BEGIN(ATTRIBUTELIST); return COLOR;}
"\\bgcolor"        {BEGIN(ATTRIBUTELIST); return BGCOLOR;}

"\\label"         {BEGIN(ATTRIBUTELIST); return LABEL;}
"\\tag"          {BEGIN(ATTRIBUTELIST); return TAG;}

"\\bbox"         {BEGIN(BRACKETLIST); return BBOX;}

"\\displaystyle"  {mtex2MML_displaymode=1; mtex2MML_rowposn=2; display_style = 1; return DISPLAY;}

"\\textstyle"     {mtex2MML_displaymode=0; mtex2MML_rowposn=2; display_style = 0; return TEXTSTY;}

"\\textsize"     {display_style = 0; return TEXTSIZE;}

"\\scriptstyle"  {display_style = 0; return SCSTY;}

"\\scriptsize"     {return SCSIZE;}

"\\scriptscriptstyle" |
"\\scriptscriptsize"     {return SCSCSIZE;}

"\\tiny" { return TINY; }
"\\Tiny" { return TTINY; }
"\\small" { return SMALL; }
"\\normalsize" { return NORMALSIZE; }
"\\large" { return LARGE; }
"\\Large" { return LLARGE; }
"\\LARGE" { return LLLARGE; }
"\\huge" { return HUGE; }
"\\Huge" { return HHUGE; }

"\\oldstyle"     {return OLDSTYLE;}

"\\moveleft"     {return MOVELEFT;}

"\\moveright"     {return MOVERIGHT;}

"\\raise"     { return RAISE;}

"\\lower"     {return LOWER;}

"\\it"         |
"\\textit"     |
"\\mathit"       {return ITALICS;}

"\\sf"         |
"\\textsf"     |
"\\mathsf"       {return SANS;}

"\\tt"         |
"\\texttt"     |
"\\mathtt"       {return TT;}

"\\boldsymbol" |
"\\bf"         |
"\\textbf"     |
"\\mathbf"       {return BOLD;}

"\\rm"         |
"\\textrm"     |
"\\mathrm"       {BEGIN(ROMAN); return RM;}

"\\scr"        |
"\\mathscr"      {BEGIN(SCRIPT); return SCR;}

"\\Bbb"        |
"\\mathbb"       {BEGIN(BBOLD); return BB;}

"\\frak"       |
"\\mathfrak"   |
"\\mathfr"       {BEGIN(FRAKTUR); return FRAK;}

"\\pmb"         {BEGIN(PMBSTRING); return PMB;}

"\\not"         {return NOT;}

"\\cancel"   |
"\\slash"       {return SLASHED;}

"\\bcancel"       {return BCANCELED;}

"\\xcancel"       {return XCANCELED;}

"\\cancelto"       {return CANCELEDTO;}

"\\boxed"       {return BOXED;}

"\\fbox"        {return FBOX;}
"\\hbox"        {return HBOX;}
"\\mbox"        {return MBOX;}

"\\mathopen"    {return MATHOPEN;}
"\\mathclose"   {return MATHCLOSE;}
"\\mathord"     {return MATHORD;}
"\\mathpunct"   {return MATHPUNCT;}

"\\vcenter"     {return VCENTER;}

"\\enclose"     {BEGIN(ENCLOSEOPTS); return ENCLOSE;}

"\\cal"         |
"\\mathcal"     {BEGIN(CALLIGRAPHIC); return CAL;}

"\\begin"        {mtex2MML_env_start = 1; BEGIN(MATHENV); return BEGINENV;}
"\\end"          { if (parsing_environment == OPEN_PARSE) parsing_environment = CLOSE_PARSE; environment_type = OTHER; BEGIN(MATHENV); return ENDENV;}

"\\notag" |
"\\nonumber"     { /* ignore, but address in .y */}

"\\substack"     {return SUBSTACK;}

"\\cases"        {return CASES;}

"\\eqalign"      {return EQALIGN;}

"\\eqalignno"      {return EQALIGNNO;}

"\\array"        {return ARRAY;}

"\\arrayopts"    {return ARRAYOPTS;}

"\\colalign"     {BEGIN(ATTRIBUTELIST); return COLALIGN;}

"\\collayout"   {BEGIN(ATTRIBUTELIST); return COLLAYOUT;}

"\\rowalign"     {BEGIN(ATTRIBUTELIST); return ROWALIGN;}

"\\align"     {BEGIN(ATTRIBUTELIST); return ALIGN;}

"\\equalrows"     {BEGIN(ATTRIBUTELIST); return EQROWS;}

"\\equalcols"     {BEGIN(ATTRIBUTELIST); return EQCOLS;}

"\\rowlines"     {BEGIN(ATTRIBUTELIST); return ROWLINES;}

"\\collines"     {BEGIN(ATTRIBUTELIST); return COLLINES;}

"\\frame"     {BEGIN(ATTRIBUTELIST); return FRAME;}

"\\padding"     {BEGIN(ATTRIBUTELIST); return PADDING;}

"\\rowopts"    {return ROWOPTS;}

"\\cellopts"   {return CELLOPTS;}

"\\rowspan"     {BEGIN(ATTRIBUTELIST); return ROWSPAN;}

"\\colspan"     {BEGIN(ATTRIBUTELIST); return COLSPAN;}


"\\," |
"\\thinspace"   {return THINSPACE;}

"\\:" |
"\\>" |
"\\medspace"    {return MEDSPACE;}

"\\;" |
"\\thickspace"  {return THICKSPACE;}

"\\quad"        {return QUAD;}
"\\qquad"       {return QQUAD;}

"\\!" |
"\\negspace" |
"\\negthinspace"    {return NEGSPACE;}

"\\negmedspace"   {return NEGMEDSPACE;}

"\\negthickspace" {return NEGTHICKSPACE;}

"\\enspace"     { return ENSPACE; }

"\\hskip"  |
"\\hspace" |
"\\mspace" |
"\\kern"   |
"\\mkern"  |
"\\mskip"       { return HSPACE; }

"\\space" |
"\\nobreakspace" { yylval=mtex2MML_copy_string("&#xA0;"); return MTEXT; }

"\\strut"   {return STRUT;}
"\\mathstrut"   {return MATHSTRUT;}

"\\phantom"     {return PHANTOM;}
"\\vphantom"    {return VPHANTOM;}
"\\hphantom"    {return HPHANTOM;}

"\\smash"       {return SMASH;}

"\\href"        {BEGIN(PLAINTEXT); return HREF;}

"\\operatorname"  {return OPERATORNAME;}
"\\mathop"        {return MATHOP;}

"\\mathbin"       {BEGIN(PLAINTEXT); return MATHBIN;}
"\\mathinner"     {BEGIN(PLAINTEXT); return MATHINNER;}
"\\mathrel"       {BEGIN(PLAINTEXT); return MATHREL;}
"\\mtexnum"       {BEGIN(PLAINTEXT); return MTEXNUM;}

"\\Space"      { BEGIN(SPACEWIDTH); return SPACECUBE; }

"\\includegraphics"(\[[^\]]*\])?\{[^}]*\} ;

"\\"[a-zA-Z]+    {yylval=mtex2MML_copy_string(yytext+1); return MOP;}

"\&"[a-zA-Z_][a-zA-Z0-9_]*";" |
"\&#"[0-9]+";" |
"\&#x"[a-fA-F0-9]+";"  {yylval=mtex2MML_copy_string(yytext); return MI;}

"\n"             mtex2MML_lineno++;
[\r\t ]+           ;


.              {return UNKNOWNCHAR;}
}

<ATTRIBUTELIST>"{"[ \t\n\r #a-zA-Z0-9.:\-]*"}"  {BEGIN(MATH); yylval=mtex2MML_copy_string(yytext); if (yylval != mtex2MML_empty_string) { yylval[0]='"'; yylval[yyleng-1]='"'; } return ATTRLIST;}

<BRACKETLIST>{
"\["  {return ST;}
"\]"  {BEGIN(MATH); return END;}
[0-9\.,]+(em|ex|px|in|cm|mm|mu|pt|pc|%) {char * normalized_px = mtex2MML_str_replace(yytext, ",", "."); if (normalized_px[0] == '.') mtex2MML_remove_first_char(normalized_px); yylval=mtex2MML_copy_string(normalized_px); mtex2MML_free_string(normalized_px); return PXSTRING; }
[0-9a-zA-Z ]+":"[0-9a-zA-Z ]+   {yylval=mtex2MML_copy_string(yytext); return STYLESTRING;}
#?[0-9a-zA-Z]+            {yylval=mtex2MML_copy_string(yytext); return COLORSTRING;}
[\t\n\r, ]+       ;
.           {return yytext[0];}
}

<OPTARG>{
"[" {if(mtex2MML_optarg_ind < mtex2MML_optarg_depth - 1 && mtex2MML_inoptarg[mtex2MML_optarg_ind] !=1 ){mtex2MML_optarg_ind++; mtex2MML_inoptarg[mtex2MML_optarg_ind]=1;}; BEGIN(MATH); return OPTARGOPEN;}
[\n\r\t ]+    ;
.   {mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src]=0; --mtex2MML_eqn_end; BEGIN(MATH); unput(yytext[0]);}
}

<RAISEBOX1>{
\{[\n\r\t ]*- {BEGIN(RAISEBOX2); return NEG;}
\{[\n\r\t ]* {BEGIN(RAISEBOX2);}
}

<RAISEBOX2>{
[^\}]*?\}  {yylval=mtex2MML_copy_string(yytext); if ((yyleng > 1) && (yylval != mtex2MML_empty_string)) yylval[yyleng-1]='\0'; BEGIN(TEXTOPTARG1); return TEXTSTRING;}
}


<TEXTOPTARG1>{
\[[^\]]*?\] {yylval=mtex2MML_copy_string(yytext+1); if ((yyleng > 1) && (yylval != mtex2MML_empty_string)) yylval[yyleng-2]='\0'; BEGIN(TEXTOPTARG); return TEXTSTRING;}
. {mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src]=0; --mtex2MML_eqn_end; BEGIN(MATH); unput(yytext[0]);}
}

<TEXTOPTARG>{
\[[^\]]*?\] {yylval=mtex2MML_copy_string(yytext+1); if ((yyleng > 1) && (yylval != mtex2MML_empty_string)) yylval[yyleng-2]='\0'; BEGIN(MATH); return TEXTSTRING;}
. {mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src]=0; --mtex2MML_eqn_end; BEGIN(MATH); unput(yytext[0]);}
}

<MATHENV>{
"\{equation\}" { if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return EQUATION;}
"\{equation*\}" { if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return EQUATION_STAR;}
"\{matrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return MATRIX;}
"\{pmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return PMATRIX;}
"\{bmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return BMATRIX;}
"\{Bmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return BBMATRIX;}
"\{vmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return VMATRIX;}
"\{Vmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return VVMATRIX;}
"\{smallmatrix\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); }; return SMALLMATRIX;}
"\{cases\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return CASES;}
"\{aligned\}[" {BEGIN(ENVOPTIONS); return ALIGNED;}
"\{aligned\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNED;}
"\{align\}" { if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNENV;}
"\{align*\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNENV_STAR;}
"\{alignat\}" {environment_type = ENV_ALIGNAT; if (mtex2MML_env_start == 1) BEGIN(ALIGNATOPTS); else if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNAT;}
"\{alignat*\}" {environment_type = ENV_ALIGNATSTAR; if (mtex2MML_env_start == 1) BEGIN(ALIGNATOPTS); else if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNAT_STAR;}
"\{alignedat\}" {if (mtex2MML_env_start == 1) BEGIN(ALIGNATOPTS); else if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNEDAT;}
"\{eqnarray*\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return EQNARRAY_STAR;}
"\{eqnarray\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return EQNARRAY;}
"\{gather\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return GATHER;}
"\{gather*\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return GATHER_STAR;}
"\{gathered\}[" {BEGIN(ENVOPTIONS); return GATHERED;}
"\{gathered\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return GATHERED;}
"\{multline*\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return MULTLINE_STAR;}
"\{multline\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return MULTLINE;}
"\{split\}" {if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ALIGNED;}
"\{array\}" {if (mtex2MML_env_start == 1) BEGIN(ARRAYOPTIONS); else if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return ARRAY;}
"\{subarray\}" {if (mtex2MML_env_start == 1) BEGIN(ARRAYOPTIONS); else if (parsing_environment == CLOSE_PARSE) { mtex2MML_reset_parsing_environment(); } else { BEGIN(MATH); } return SUBARRAY;}
"\{svg\}" {mtex2MML_env_start = 0; BEGIN(SVGENV); return SVG;}
}

<ARRAYOPTIONS>{
"[t]" {yylval=mtex2MML_copy_string("axis 1"); return ARRAYALIGN;}
"[c]" {yylval=mtex2MML_copy_string("center"); return ARRAYALIGN;}
"[b]" {yylval=mtex2MML_copy_string("axis -1"); return ARRAYALIGN;}
"\{"  {BEGIN(COLOPTS); return ST;}
[\t\n\r ]+       ;
}

<ALIGNATOPTS>{
"\{"     { /* Ignore */ }
[0-9]+   {yylval=mtex2MML_copy_string(yytext); return ALIGNATVALUE;}
"\}"     {mtex2MML_env_start = 0; BEGIN(MATH); return END;}
[\t\n\r ]+       ;
}

<ENVOPTIONS>{
"t" {yylval=mtex2MML_copy_string("baseline 1"); return ARRAYALIGN;}
"c" {yylval=mtex2MML_copy_string("center"); return ARRAYALIGN;}
"b" {yylval=mtex2MML_copy_string("baseline -1"); return ARRAYALIGN;}
"]"  {mtex2MML_env_start = 0; BEGIN(MATH); return END; }
[\t\n\r ]+       ;
}

<COLOPTS>{
"\}"  {mtex2MML_env_start = 0; BEGIN(MATH); return END;}
"c"   {yylval=mtex2MML_copy_string("center"); return COLUMNALIGN;}
"l"   {yylval=mtex2MML_copy_string("left"); return COLUMNALIGN;}
"r"   {yylval=mtex2MML_copy_string("right"); return COLUMNALIGN;}
"|"   {yylval=mtex2MML_copy_string("s"); return COLUMNALIGN;}
":"   {yylval=mtex2MML_copy_string("d"); return COLUMNALIGN;}
[\t\n\r ]+       ;
}

<SVGENV>{
"\\end\{svg" {mtex2MML_do_capture_eqn(); yylval=mtex2MML_copy_string(yytext); yylval[yyleng-8]=0; BEGIN(SVGEND); return XMLSTRING;}
(.|\n) {yymore();}
}

<SVGEND>{
"\}" {BEGIN(MATH); return ENDSVG;}
}

<ROMAN>{
"\{"     {mtex2MML_variant_start = 1; return ST;}
"\}"     {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
"\\ "    {yylval=mtex2MML_copy_string(" "); return RMCHAR;}
[^ ]     {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return RMCHAR;}
[\t\n\r ]+       ;
}

<SCRIPT>{
"\{"     {mtex2MML_variant_start = 1; return ST;}
"\}"     {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
"\\ "    {yylval=mtex2MML_copy_string(" "); return SCRCHAR;}
[^ ]     {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return SCRCHAR;}
[\t\n\r ]+       ;
}

<PMBSTRING>{
"\{"     {mtex2MML_variant_start = 1; return ST;}
"\}"     {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
"\\ "    {yylval=mtex2MML_copy_string(" "); return PMBCHAR;}
[^ ]     {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return PMBCHAR;}
[\t\n\r ]+       ;
}

<BBOLD>{
"\{"  {mtex2MML_variant_start = 1; return ST;}
"\}"  {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
[a-z]+ {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return BBLOWERCHAR;}
[A-Z]+   {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return BBUPPERCHAR;}
[0-9]   {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return BBDIGIT;}
[\t\n\r ]+       ;
.           {return yytext[0];}
}

<CALLIGRAPHIC>{
"\{"     {mtex2MML_variant_start = 1; return ST;}
"\}"     {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
"\\ "    {yylval=mtex2MML_copy_string(" "); return CALCHAR;}
[^ ]     {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return CALCHAR;}
[\t\n\r ]+       ;
}

<FRAKTUR>{
"\{"     {mtex2MML_variant_start = 1; return ST;}
"\}"     {mtex2MML_variant_start = 0; BEGIN(MATH); return END;}
"\\ "    {yylval=mtex2MML_copy_string(" "); return FRAKCHAR;}
[^ ]     {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return FRAKCHAR;}
[\t\n\r ]+       ;
}

<PLAINTEXT>{
"\{"          {yylval=mtex2MML_copy_string_extra("", 1); BEGIN(PLAINTEXT_CURLY);}
.             {yylval=mtex2MML_copy_string(yytext); if (mtex2MML_variant_start == 0) BEGIN(MATH); return TEXTSTRING;}
[\t\n\r ]+       ;
}

<PLAINTEXT_CURLY>{
\\.          {char *s = mtex2MML_copy_string(yylval); mtex2MML_free_string(yylval); yylval = mtex2MML_copy2(s, yytext+1); mtex2MML_free_string(s); }
"\}"         {BEGIN(MATH); return TEXTSTRING;}
.            {char *s = mtex2MML_copy_string(yylval); mtex2MML_free_string(yylval); yylval=mtex2MML_copy2(s, yytext);  mtex2MML_free_string(s);}
}

<VERBTEXT>{
.         { if (yytext[0] != mtex2MML_verbdelim) { char *s = mtex2MML_copy_string(mtex2MML_str); mtex2MML_free_string(mtex2MML_str); mtex2MML_str=mtex2MML_copy2(s, yytext); mtex2MML_free_string(s); } else { yylval=mtex2MML_copy_string(mtex2MML_str); mtex2MML_free_string(mtex2MML_str); BEGIN(MATH); return VERBSTRING; } }
}

<NUM>{
"\{"      {return ST;}
"\}"      {BEGIN(MATH); return END;}
-?[.0-9]+    {yylval=mtex2MML_copy_string(yytext); return NUMBER;}
[\t\n\r ]+   ;
}

<INT1>{
"\{"      {return ST;}
"\}"      {BEGIN(INT2); return END;}
[0-9]+    {yylval=mtex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != mtex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTONE;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}

<INT2>{
"\{"      {return ST;}
"\}"      {BEGIN(INT3); return END;}
[0-9]+    {yylval=mtex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != mtex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTTWO;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}

<INT3>{
"\{"      {return ST;}
"\}"      {BEGIN(MATH); return END;}
[0-9]+    {yylval=mtex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != mtex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTTHREE;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}

<ENCLOSEOPTS>{
"\}\{"      {BEGIN(ENCLOSETEXTPARSE); return ST;}
"\}\["      {BEGIN(ENCLOSEATTRPARSE); return ST;}
"\{"       {return ST;}
[a-zA-Z,]+  {yylval=mtex2MML_copy_string(yytext); return ENCLOSENOTATION;}
[\t\n\r ]+       ;
}

<ENCLOSEATTRPARSE>{
"\]\{"       {BEGIN(ENCLOSETEXTPARSE); return ST;}
[a-zA-Z,=0-9]+  {yylval=mtex2MML_copy_string(yytext); return ENCLOSEATTR;}
[\t\n\r ]+       ;
}

<ENCLOSETEXTPARSE>{
"\}"      {BEGIN(MATH); return ST;}
[a-zA-Z]+  {yylval=mtex2MML_copy_string(yytext); return ENCLOSETEXT;}
[\t\n\r ]+       ;
}

<SPACEWIDTH>{
"\{"      { return ST;}
[0-9\.,]+(em|ex|px|in|cm|mm|mu|pt|pc|%) {char * normalized_px = mtex2MML_str_replace(yytext, ",", "."); yylval=mtex2MML_copy_string(normalized_px); mtex2MML_free_string(normalized_px); return PXSTRING; }
"\}"      {BEGIN(SPACEHEIGHT); return ST;}
[\t\n\r ]+       ;
}

<SPACEHEIGHT>{
"\{"      { return ST;}
[0-9\.,]+(em|ex|px|in|cm|mm|mu|pt|pc|%) {char * normalized_px = mtex2MML_str_replace(yytext, ",", "."); yylval=mtex2MML_copy_string(normalized_px); mtex2MML_free_string(normalized_px); return PXSTRING; }
"\}"      {BEGIN(SPACEDEPTH); return ST;}
[\t\n\r ]+       ;
}

<SPACEDEPTH>{
"\{"      { return ST;}
[0-9\.,]+(em|ex|px|in|cm|mm|mu|pt|pc|%) {char * normalized_px = mtex2MML_str_replace(yytext, ",", "."); yylval=mtex2MML_copy_string(normalized_px); mtex2MML_free_string(normalized_px); return PXSTRING; }
"\}"      {BEGIN(MATH); return ST;}
[\t\n\r ]+       ;
}

%%

void mtex2MML_restart ()
{
  BEGIN(INITIAL);
}

void mtex2MML_reset_parsing_environment()
{
  parsing_environment = NO_PARSE;
  mtex2MML_eqn_src[mtex2MML_eqn_end-mtex2MML_eqn_src-yyleng]=0;
  mtex2MML_eqn_end=mtex2MML_eqn_src;

  BEGIN(INITIAL);
}

static const char *  s_mtex2MML_buffer = 0;
static unsigned long s_mtex2MML_length = 0;

void mtex2MML_setup (const char * buffer, unsigned long length)
{
  yy_flush_buffer( YY_CURRENT_BUFFER );

  s_mtex2MML_buffer = buffer;
  s_mtex2MML_length = length;
}

int mtex2MML_capture_eqn_src()
{
  if (YY_START != INITIAL && YY_START != SVGENV)
    return mtex2MML_do_capture_eqn();

  return -1;
}

int mtex2MML_do_capture_eqn()
{
  if (mtex2MML_eqn_bufsize - (mtex2MML_eqn_end-mtex2MML_eqn_src+yyleng) > 1)
  {
    strcpy(mtex2MML_eqn_end, yytext);
    mtex2MML_eqn_end += yyleng;
    return 0;
  }

  return -1;
}

static int s_mtex2MML_read (char * buffer, int max_size)
{
  int result = 0;

  if ((unsigned long) max_size < s_mtex2MML_length)
  {
    result = max_size;

    memcpy (buffer, s_mtex2MML_buffer, max_size);

    s_mtex2MML_buffer += max_size;
    s_mtex2MML_length -= max_size;
  }
  else
  {
    result = s_mtex2MML_length;

    memcpy (buffer, s_mtex2MML_buffer, s_mtex2MML_length);

    s_mtex2MML_buffer = 0;
    s_mtex2MML_length = 0;
  }
  return result;
}
