#ifndef __GPL_stuff_h__
#define __GPL_stuff_h__

#define GPL_base_text \
"Yacas is Free Software--Free as in Freedom--so you can redistribute Yacas or\n" \
"modify it under certain conditions. Yacas comes with ABSOLUTELY NO WARRANTY.\n" \
"See the GNU General Public License (GPL) for the full conditions.\n"

#define Yacas_Web_info \
"See http://yacas.sf.net for more information and documentation on Yacas.\n"

#define Yacas_help_info \
"Type ?license or ?licence to see the GPL; type ?warranty for warranty info.\n"

// This is the full text for systems where the online help (?blah) is available
#define GPL_blurb GPL_base_text Yacas_help_info Yacas_Web_info "\n"

// This is for systems where online help (?blah) is normally not available
#define GPL_blurb_nohelp GPL_base_text Yacas_Web_info "\n"



#endif
