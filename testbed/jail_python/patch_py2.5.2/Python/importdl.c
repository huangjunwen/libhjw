
/* Support for dynamic loading of extension modules */

#include "Python.h"

/* ./configure sets HAVE_DYNAMIC_LOADING if dynamic loading of modules is
   supported on this platform. configure will then compile and link in one
   of the dynload_*.c files, as appropriate. We will call a function in
   those modules to get a function pointer to the module's init function.
*/
#ifdef HAVE_DYNAMIC_LOADING

#include "importdl.h"

PyObject *
_PyImport_LoadDynamicModule(char *name, char *pathname, FILE *fp)
{
    PyErr_SetString(PyExc_RuntimeError, 
            "permission denied: import dso extension");
    return NULL;
}

#endif /* HAVE_DYNAMIC_LOADING */
