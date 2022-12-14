#ifndef PY3DENGINE_PYTHON_COMPONENT_H
#define PY3DENGINE_PYTHON_COMPONENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "components/base_component.h"

#define COMPONENT_TYPE_NAME_PYTHON "PythonComponent"

struct PythonComponent {
    struct BaseComponent base;

    PyObject *pyComponent;
};

extern void allocPythonComponent(struct PythonComponent **componentPtr);
extern void deletePythonComponent(struct PythonComponent **componentPtr);

extern void initPythonComponent(struct PythonComponent *component, PyObject *componentType);
extern PyObject *getPythonComponentAttr(struct PythonComponent *component, const char *attrName);
extern void setPythonComponentAttr(struct PythonComponent *component, const char *attrName, PyObject *newAttr);

#endif
