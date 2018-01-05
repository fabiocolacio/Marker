#include "colors.h"

void mtex2MML_create_css_colors(struct css_colors **colors)
{
  const char **n, **h,
        *names[] = { "\"Apricot\"","\"Aquamarine\"","\"Bittersweet\"",
                     "\"Black\"","\"Blue\"","\"BlueGreen\"","\"BlueViolet\"","\"BrickRed\"",
                     "\"Brown\"","\"BurntOrange\"","\"CadetBlue\"","\"CarnationPink\"","\"Cerulean\"",
                     "\"CornflowerBlue\"","\"Cyan\"","\"Dandelion\"","\"DarkOrchid\"","\"Emerald\"",
                     "\"ForestGreen\"","\"Fuchsia\"","\"Goldenrod\"","\"Gray\"","\"Green\"",
                     "\"GreenYellow\"","\"JungleGreen\"","\"Lavender\"","\"LimeGreen\"","\"Magenta\"",
                     "\"Mahogany\"","\"Maroon\"","\"Melon\"","\"MidnightBlue\"","\"Mulberry\"",
                     "\"NavyBlue\"","\"OliveGreen\"","\"Orange\"","\"OrangeRed\"","\"Orchid\"",
                     "\"Peach\"","\"Periwinkle\"","\"PineGreen\"","\"Plum\"","\"ProcessBlue\"",
                     "\"Purple\"","\"RawSienna\"","\"Red\"","\"RedOrange\"","\"RedViolet\"",
                     "\"Rhodamine\"","\"RoyalBlue\"","\"RoyalPurple\"","\"RubineRed\"","\"Salmon\"",
                     "\"SeaGreen\"","\"Sepia\"","\"SkyBlue\"","\"SpringGreen\"","\"Tan\"","\"TealBlue\"",
                     "\"Thistle\"","\"Turquoise\"","\"Violet\"","\"VioletRed\"","\"White\"",
                     "\"WildStrawberry\"","\"Yellow\"","\"YellowGreen\"", "\"YellowOrange\"", NULL
                   },

                   *hex_code[] = { "\"#FBB982\"","\"#00B5BE\"","\"#C04F17\"","\"#221E1F\"","\"#2D2F92\"",
                                   "\"#00B3B8\"","\"#473992\"","\"#B6321C\"","\"#792500\"","\"#F7921D\"","\"#74729A\"",
                                   "\"#F282B4\"","\"#00A2E3\"","\"#41B0E4\"","\"#00AEEF\"","\"#FDBC42\"","\"#A4538A\"",
                                   "\"#00A99D\"","\"#009B55\"","\"#8C368C\"","\"#FFDF42\"","\"#949698\"","\"#00A64F\"",
                                   "\"#DFE674\"","\"#00A99A\"","\"#F49EC4\"","\"#8DC73E\"","\"#EC008C\"","\"#A9341F\"",
                                   "\"#AF3235\"","\"#F89E7B\"","\"#006795\"","\"#A93C93\"","\"#006EB8\"","\"#3C8031\"",
                                   "\"#F58137\"","\"#ED135A\"","\"#AF72B0\"","\"#F7965A\"","\"#7977B8\"","\"#008B72\"",
                                   "\"#92268F\"","\"#00B0F0\"","\"#99479B\"","\"#974006\"","\"#ED1B23\"","\"#F26035\"",
                                   "\"#A1246B\"","\"#EF559F\"","\"#0071BC\"","\"#613F99\"","\"#ED017D\"","\"#F69289\"",
                                   "\"#3FBC9D\"","\"#671800\"","\"#46C5DD\"","\"#C6DC67\"","\"#DA9D76\"","\"#00AEB3\"",
                                   "\"#D883B7\"","\"#00B4CE\"","\"#58429B\"","\"#EF58A0\"","\"#FFFFFF\"","\"#EE2967\"",
                                   "\"#FFF200\"","\"#98CC70\"","\"#FAA21A\"", NULL
                                 };

  struct css_colors *c = NULL;

  for (n = names, h = hex_code; *n != NULL; n++, h++) {
    c = malloc(sizeof(struct css_colors));
    strncpy(c->name, *n, 22);
    strncpy(c->color, *h, 10);
    HASH_ADD_STR(*colors, name, c );
  }
}
