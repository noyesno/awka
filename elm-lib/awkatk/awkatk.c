#include <awkatk.h>
#include <tcl.h>
#include <tk.h>

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

extern char **tcl_init;
extern char **tk_init;

extern int matherr();
int *tclDummyMathPtr = (int *) matherr;

Tcl_Interp *interp = NULL;

/*
 *----------------------------------------------------------------------
 *
 * _awkatk_execfn --
 *
 *        This is a middleman function between a Tcl procedure callback
 *        and the user-defined AWK functions.  It populates an a_VARARG
 *        structure from the objv[] array, then calls the function 
 *        referenced by clientdata.
 *
 * Returns:
 *        TCL_OK on successful execution, TCL_ERROR if there was a problem.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

int
_awkatk_execfn( ClientData clientdata, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
  static a_VARARG va;
  static int va_used = 0;
  a_VAR * (*fn)( a_VARARG *);
  int i;
  
  if (!clientdata)
    return TCL_ERROR;

  if (!va_used)
  {
    va_used = 1;
    va.used = 0;
    memset(va.var, 0, sizeof(a_VAR *) * 256);
  }
  if (objc > 256) objc = 256;
  
  for (i=1; i<objc; i++)
  {
    if (!va.var[i-1])
      awka_varinit(va.var[i-1]);
    
    awka_strcpy(va.var[i-1], Tcl_GetStringFromObj(objv[i], NULL));
  }
  va.used = objc-1;
  
  fn = clientdata;
  fn(&va);

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * _awkatk_mapfn --
 *
 *        This creates a new Tcl command for each user-defined function
 *        in the original AWK script.  This allows procedure callbacks
 *        from Tk widgets to invoke the AWK functions.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

void
_awkatk_mapfn()
{
  struct awka_fn_struct *fn = _awkafn;

  while (fn->name)
  {
    Tcl_CreateObjCommand(interp, fn->name, _awkatk_execfn, (ClientData) fn->fn, NULL);
    fn++;
  }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *        This procedure performs application-specific initialization.
 *        Most applications, especially those that incorporate additional
 *        packages, will have their own version of this procedure.
 *
 * Results:
 *        Returns a standard Tcl completion code, and leaves an error
 *        message in interp->result if an error occurs.
 *
 * Side effects:
 *        Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;                /* Interpreter for application. */
{
    char *cmd;
    int result;

    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

#ifdef AWKATK_USE_TK
    Tk_Window main;

    if (Tk_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

    main = Tk_MainWindow(interp);
#endif
    
#if 0
    if ((result = Tcl_EvalFile(interp, INIT_TCL_FILE)) != TCL_OK)
      awka_error("Failed to read %s: %s\n",INIT_TCL_FILE,interp->result);
        
    if ((result = Tcl_EvalFile(interp, TK_TCL_FILE)) != TCL_OK)
      awka_error("Failed to read %s: %s\n",TK_TCL_FILE,interp->result);
#endif
        
    Tcl_SetVar(interp, "tcl_rcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);
    
    _awkatk_mapfn();
    
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * _tk_init --
 *
 *        Called only once, this ensures that an interpreter has been
 *        created, and that Tcl_AppInit has been called.
 *
 * Returns:
 *        Nothing
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

void
_tk_init()
{
  interp = Tcl_CreateInterp();
  Tcl_AppInit(interp);
}

/*
 *----------------------------------------------------------------------
 *
 * tk_fn --
 *
 *        This API function basically provides an interface to Tcl_Eval,
 *        and uses only one string argument, which is the Tcl script
 *        to be evaluated.
 *
 * Returns:
 *        The string value in interp->result that is output from the
 *        call to Tcl_Eval, or the numeric error flag if a problem has
 *        occurred.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

a_VAR *
tk_fn( a_VARARG *va )
{
  a_VAR *ret = awka_getstringvar(FALSE);
  
  if (!interp)
    _tk_init();
  
  if (!va->used)
  {
    awka_setd(ret) = AWKATK_TOOFEWARGS;
    return ret;
  }
  if (va->var[0]->type == a_VARNUL || va->var[0]->type == a_VARDBL)
  {
    awka_setd(ret) = AWKATK_ARGNOTSTRING;
    return ret;
  }

  if (Tcl_Eval(interp, awka_gets(va->var[0])) != TCL_OK)
    awka_error("Call to tk(%s) failed\n",va->var[0]->ptr);
  
  awka_strcpy(ret, Tcl_GetStringResult(interp));
  return ret;
}

/*
 *----------------------------------------------------------------------
 * tcl_fn --
 *----------------------------------------------------------------------
 */
a_VAR *
tcl_fn( a_VARARG *va )
{
  a_VAR *ret = awka_getstringvar(FALSE);
  
  if (!interp)
    _tk_init();
  
  if (!va->used)
  {
    awka_setd(ret) = AWKATK_TOOFEWARGS;
    return ret;
  }
  if (va->var[0]->type == a_VARNUL || va->var[0]->type == a_VARDBL)
  {
    awka_setd(ret) = AWKATK_ARGNOTSTRING;
    return ret;
  }

  if (Tcl_Eval(interp, awka_gets(va->var[0])) != TCL_OK)
    awka_error("Call to tk(%s) failed\n",va->var[0]->ptr);
  
  awka_strcpy(ret, Tcl_GetStringResult(interp));
  return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * tk_setvar_fn --
 *
 *        This sets a TCL variable, identified by the first argument in
 *        va, to the value contained in the second argument of va.
 *
 * Returns:
 *        AWKATK_OK on success, otherwise a flag indicating which type
 *        of error has occured.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

a_VAR *
tk_setvar_fn( a_VARARG *va )
{
  char *result;
  a_VAR *ret = awka_getdoublevar(FALSE);
  ret->dval = AWKATK_OK;
  
  if (va->used < 2)
  {
    ret->dval = AWKATK_TOOFEWARGS;
    return ret;
  }
  if (va->var[0]->type == a_VARNUL || va->var[0]->type == a_VARDBL)
  {
    ret->dval = AWKATK_ARGNOTSTRING;
    return ret;
  }

  result = Tcl_SetVar( interp, awka_gets(va->var[0]), awka_gets(va->var[1]), TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG );
  
  if (result == NULL)
    awka_strcpy(ret, Tcl_GetStringResult(interp));
  
  return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * tk_getvar_fn --
 *
 *        This retrieves the value of the TCL variable identified by the
 *        first argument of va and sets a temporary a_VAR to this value. 
 *
 * Returns:
 *        The temporary a_VAR * containing the retrieved value.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

a_VAR *
tk_getvar_fn( a_VARARG *va )
{
  char *str;
  a_VAR *ret = awka_getstringvar(FALSE);
  
  if (va->used < 1)
  {
    ret->dval = AWKATK_TOOFEWARGS;
    return ret;
  }
  if (va->var[0]->type == a_VARNUL || va->var[0]->type == a_VARDBL)
  {
    ret->dval = AWKATK_ARGNOTSTRING;
    return ret;
  }

  str = Tcl_GetVar(interp, awka_gets(va->var[0]), TCL_GLOBAL_ONLY);
  if (!str)
    awka_strcpy(ret, "");
  else
    awka_strcpy(ret, str);

  return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * tk_mainloop_fn --
 *
 *        Ensures tk has been inited, then calls Tk_MainLoop(), which
 *        will continue checking for user events until the tk window
 *        has been closed.
 *
 * Returns:
 *        An a_VAR * set to AWKATK_OK.
 *
 * Side effects:
 *        none.
 *
 *----------------------------------------------------------------------
 */

a_VAR *
tk_mainloop_fn( a_VARARG *va )
{
  a_VAR *ret = awka_getdoublevar(FALSE);
  ret->dval = AWKATK_OK;

  if (!interp)
    _tk_init();

#ifdef AWKATK_USE_TK
  Tk_MainLoop();
#endif
  return ret;
}

