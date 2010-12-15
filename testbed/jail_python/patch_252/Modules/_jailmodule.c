#include "Python.h"
#include "_pyjail.h"

static int _in_jail = 0;

const int _jail_enabled() {
    return _in_jail;
}

// !!! please compile python without threads with this
static PyObject * _jail_enable(PyObject * unused1, PyObject * unused2) {
    if (_in_jail) {
        PyErr_SetString(PyExc_RuntimeError, "already in jail");
        return NULL;
    }

    PyObject * v;
#define SET_SYS_KV(key, value) \
    v = value; \
    if (v != NULL) \
        PySys_SetObject(key, v); \
    Py_XDECREF(v);

    // set sys.argv to ['',]
    SET_SYS_KV("argv", Py_BuildValue("[s]", ""));
    // set sys.executable to ''
    SET_SYS_KV("executable", PyString_FromString(""));
#undef SET_SYS_KV
    
    // set the flag
    _in_jail = 1;
    Py_RETURN_NONE;
}

static struct PyMethodDef _jail_methods[] = {
	{"enable", _jail_enable, METH_NOARGS, ""},
	{NULL,	 NULL}		/* sentinel */
};

PyMODINIT_FUNC
init_jail(void) {
	(void)Py_InitModule("_jail", _jail_methods);
}

