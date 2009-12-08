#include <string>
#include "pyconsole.h"

using namespace std;

namespace pyconsole {

class AutoDecRef {
    // use it on new references
public:
    AutoDecRef(PyObject * obj): _obj(obj) {
    }
    ~AutoDecRef() {
        Py_XDECREF(_obj);
    }
    PyObject * operator() () { 
        return _obj; 
    }
private:
    PyObject * _obj;
};

class PyConsoleImpl: public PyConsole {
public:
    PyConsoleImpl();
    virtual ~PyConsoleImpl();
    virtual bool start();
    virtual void stop();
    virtual bool restart();
    virtual void loop_once(const string & in, string & out, string & err);
    virtual bool alive() { return _alive; }
private:
    // helper functions

    PyObject * newStringIO();
    bool redirectOutAndErr();
    bool readStringIO(PyObject * io, string & ret);
    bool readStdout(string & ret);
    bool readStderr(string & ret);

private:
    // member
    PyObject * globals;
    bool _alive;
};


PyConsole * getConsole() {
    static PyConsoleImpl _console;
    return &_console;
}

PyConsoleImpl::PyConsoleImpl(): _alive(false) {
    start();
}

PyConsoleImpl::~PyConsoleImpl() {
    stop();
}

bool PyConsoleImpl::start() {
    if (_alive)
        return true;

    Py_Initialize();
    globals = PyDict_New();

    if (!globals)
        return false;

    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    if (!redirectOutAndErr())
        return false;

    _alive = true;
    return true;
}

void PyConsoleImpl::stop() {
    if (!_alive)
        return;

    if (globals)
        Py_XDECREF(globals);
    Py_Finalize();
    _alive = false;
}

bool PyConsoleImpl::restart() {
    stop();
    return start();
}

void PyConsoleImpl::loop_once(const string & in, string & out, string & err) {
    if (!_alive)
        return;

    out.clear();
    err.clear();
    // in should be stripped
    if (!in.size())
        return;
    
    // run it
    PyObject * ret = PyRun_String(in.c_str(), Py_single_input, globals, globals);
    if (PyErr_Occurred()) {
        PyErr_Print();
    }

    // get output
    if (!readStdout(out))
        out = "Can't read from stdout(sys.stdout maybe changed)\n";
    if (!readStderr(err))
        err = "Can't read from stderr(sys.stderr maybe changed)\n";
    Py_XDECREF(ret);
}


// new ref
PyObject * PyConsoleImpl::newStringIO() {
    AutoDecRef cStringIO(PyImport_ImportModule("cStringIO"));
    if (!cStringIO())
        return 0;
    AutoDecRef StringIO(PyObject_GetAttrString(cStringIO(), "StringIO"));
    if (!StringIO()) 
        return 0;
    return PyObject_CallObject(StringIO(), NULL);
}

bool PyConsoleImpl::redirectOutAndErr() {
    AutoDecRef sys(PyImport_ImportModule("sys"));
    if (!sys())
        return false;

    AutoDecRef new_stdout(newStringIO());
    if (!new_stdout())
        return false;
    if (PyObject_SetAttrString(sys(), "stdout", new_stdout()) < 0)
        return false;

    AutoDecRef new_stderr(newStringIO());
    if (!new_stderr())
        return false;
    if (PyObject_SetAttrString(sys(), "stderr", new_stderr()) < 0)
        return false;

    return true;
}

bool PyConsoleImpl::readStringIO(PyObject * io, string & ret) {
    if (!io)
        return false;
    // get all methods needed
    AutoDecRef getvalue(PyObject_GetAttrString(io, "getvalue"));
    AutoDecRef reset(PyObject_GetAttrString(io, "reset"));
    AutoDecRef truncate(PyObject_GetAttrString(io, "truncate"));
    if (!getvalue() || !reset() || !truncate())
        return false;

    AutoDecRef s(PyObject_CallObject(getvalue(), NULL));
    if (!s())
        return false;
    ret = PyString_AsString(s());

    PyObject * tmp = PyObject_CallObject(reset(), NULL);
    if (!tmp)
        return false;
    Py_XDECREF(tmp);
    tmp = PyObject_CallObject(truncate(), NULL);
    if (!tmp)
        return false;
    Py_XDECREF(tmp);
    return true;
}

bool PyConsoleImpl::readStdout(string & ret) {
    AutoDecRef sys(PyImport_ImportModule("sys"));
    if (!sys())
        return 0;
    AutoDecRef so(PyObject_GetAttrString(sys(), "stdout"));
    return readStringIO(so(), ret);
}

bool PyConsoleImpl::readStderr(string & ret) {
    AutoDecRef sys(PyImport_ImportModule("sys"));
    if (!sys())
        return 0;
    AutoDecRef se(PyObject_GetAttrString(sys(), "stderr"));
    return readStringIO(se(), ret);
}

}
