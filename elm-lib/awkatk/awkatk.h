#ifndef _AWKATK_H
#define _AWKATK_H

#include <libawka.h>
#define AWKATK_TOOFEWARGS   -1
#define AWKATK_ARGNOTSTRING -2
#define AWKATK_ERROR        0
#define AWKATK_OK           1

a_VAR *tk_fn( a_VARARG *va );
a_VAR *tk_setvar_fn( a_VARARG *va );
a_VAR *tk_getvar_fn( a_VARARG *va );
a_VAR *tk_mainloop_fn( a_VARARG *va );

#endif
