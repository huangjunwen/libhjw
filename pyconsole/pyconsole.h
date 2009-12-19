#ifndef _PYCONSOLE_H_
#define _PYCONSOLE_H_

#include <string>
#include <Python.h>

namespace pyconsole {

class PyConsole {
public:
    virtual bool restart() = 0;
    virtual bool alive() = 0;
    virtual void loop_once(const std::string & in, std::string & out, std::string & err) = 0;
public:
};

PyConsole * getConsole();

}

#endif
