#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

#include "logger.h"
#include "python/python_util.h"
#include "python/py3denginemodule.h"
#include "game_object.h"

static struct PyModuleDef py3dengineModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dengine",
    .m_doc = "Top level module containing interface to engine.",
    .m_size = -1,
};

static PyObject *module = NULL;

PyMODINIT_FUNC
PyInit_py3dEngine(void) {
    PyObject *newModule = PyModule_Create(&py3dengineModuleDef);
    if (newModule == NULL) {
        critical_log("%s", "[Python]: Failed to create \"py3dengine\" module");

        return NULL;
    }

    if (!PyInit_Py3dGameObject(newModule)) {
        critical_log("%s", "[Python]: Failed to attach Game Object to py3dengine module");

        Py_CLEAR(newModule);
    }

    return newModule;
}

bool appendPy3dEngineModule() {
    if (PyImport_AppendInittab("py3dengine", PyInit_py3dEngine) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with py3dengine module");
        return false;
    }

    return true;
}

bool importPy3dEngineModule() {
    module = PyImport_ImportModule("py3dengine");
    if (module == NULL) {
        critical_log("%s, [Python]: Could not import py3dengine");
        handleException();
        return false;
    }

    Py_INCREF(module);

    return true;
}

bool initPy3dEngineObjects() {
    return findPyGameObjectCtor(module);
}

void finalizePy3dEngineModule() {
    finalizePyGameObjectCtor();
}