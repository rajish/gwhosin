#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <libintl.h>

// we will use this to produce proper utf8 strings from gettext output
#define GT(x) Glib::locale_to_utf8(gettext(x))

// interpolation type, will later be read from config
#define INTERPTYPE Gdk::INTERP_BILINEAR

#endif // _DEFINES_H_
