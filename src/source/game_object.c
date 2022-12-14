#include <stdlib.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "custom_string.h"
#include "logger.h"
#include "game_object.h"
#include "components/base_component.h"
#include "components/transform_component.h"
#include "python/python_util.h"

struct ComponentListNode {
    struct BaseComponent *component;
    PyObject *pyComponent;
    struct ComponentListNode *next;
};

struct ChildListNode {
    struct GameObject *child;
    struct ChildListNode *next;
};

struct Py3dGameObject {
    PyObject_HEAD
    struct GameObject *gameObject;
};

static PyObject *py3dGameObjectCtor = NULL;

static void py3d_game_object_dealloc(struct Py3dGameObject *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *py3d_game_object_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct Py3dGameObject *self = (struct Py3dGameObject *) type->tp_alloc(type, 0);
    self->gameObject = NULL;

    return (PyObject *) self;
}

static int py3d_game_object_init(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return 0;
}

static PyObject *py3d_game_object_get_name(struct Py3dGameObject *self, PyObject * Py_UNUSED(ignored)) {
    if (self == NULL || self->gameObject == NULL) Py_RETURN_NONE;

    return PyUnicode_FromString(getChars(getGameObjectName(self->gameObject)));
}

PyMemberDef py3d_game_object_members[] = {
    {NULL}
};

PyMethodDef py3d_game_object_methods[] = {
    {"get_name", (PyCFunction) py3d_game_object_get_name, METH_NOARGS, "Get Game Object's name"},
    {NULL}
};

PyTypeObject Py3dGameObjectType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.GameObject",
    .tp_basicsize = sizeof(struct Py3dGameObject),
    .tp_dealloc = (destructor) py3d_game_object_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Class for manipulating Game Objects",
    .tp_methods = py3d_game_object_methods,
    .tp_members = py3d_game_object_members,
    .tp_init = (initproc) py3d_game_object_init,
    .tp_new = py3d_game_object_new
};

static PyObject *createPy3dGameObject() {
    if (py3dGameObjectCtor == NULL) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return NULL;
    }

    PyObject *py3dGameObject = PyObject_Call(py3dGameObjectCtor, PyTuple_New(0), NULL);
    if (py3dGameObject == NULL) {
        critical_log("%s", "[Python]: Failed to allocate GameObject in python interpreter");
        handleException();

        return NULL;
    }

    return py3dGameObject;
}

static void allocComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ComponentListNode *newNode = calloc(1, sizeof(struct ComponentListNode));
    if (newNode == NULL) return;

    newNode->component = NULL;
    newNode->pyComponent = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ComponentListNode *node = (*listNodePtr);
    deleteComponent(&node->component);
    Py_CLEAR(node->pyComponent);

    deleteComponentListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static bool componentNamesAreEqual(struct ComponentListNode *listNode, const char *componentName) {
    if (listNode == NULL || componentName == NULL) return false;

    if (
        (listNode->component == NULL && listNode->pyComponent == NULL) ||
        (listNode->component != NULL && listNode->pyComponent != NULL)
    ) {
        critical_log("%s", "[GameObject]: Component list node failed sanity check and is not well formed.");
        return false;
    }

    if (listNode->component != NULL) {
        return stringEqualsCStr(getComponentName(listNode->component), componentName);
    }

    // TODO: this is wrong ... even though the component was checked when it was attached there's no
    // certainty that these fields weren't deleted afterwards
    // always check before trying to use python objects ... they're very flexible
    PyObject *get_name = PyObject_GetAttrString(listNode->pyComponent, "get_name");
    PyObject *name = PyObject_CallNoArgs(get_name);

    return strcmp(PyUnicode_AsUTF8(name), componentName) == 0;
}

static void allocChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ChildListNode *newNode = calloc(1, sizeof(struct ChildListNode));
    if (newNode == NULL) return;

    newNode->child = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ChildListNode *node = (*listNodePtr);
    deleteGameObject(&node->child);

    deleteChildListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static void initializePyGameObject(struct GameObject *gameObject) {
    if (gameObject == NULL || gameObject->pyGameObject != NULL) return;

    struct PyGameObject *newPyGameObject = (struct PyGameObject *) createPy3dGameObject();
    if (newPyGameObject == NULL) {
        critical_log("%s", "[GameObject]: Could not instantiate new PyGameObject");

        return;
    }
    Py_INCREF(newPyGameObject);

    gameObject->pyGameObject = newPyGameObject;
}

bool PyInit_Py3dGameObject(PyObject *module) {
    if (PyType_Ready(&Py3dGameObjectType) == -1) return false;

    if (PyModule_AddObject(module, "GameObject", (PyObject *) &Py3dGameObjectType) == -1) return false;

    Py_INCREF(&Py3dGameObjectType);

    return true;
}

bool findPyGameObjectCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "GameObject") == 0) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return false;
    }

    py3dGameObjectCtor = PyObject_GetAttrString(module, "GameObject");

    return true;
}

void finalizePyGameObjectCtor() {
    Py_CLEAR(py3dGameObjectCtor);
}

void allocGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) != NULL) return;

    struct GameObject *gameObject = NULL;
    gameObject = calloc(1, sizeof(struct GameObject));
    if (gameObject == NULL) return;

    gameObject->components = NULL;
    gameObject->children = NULL;
    gameObject->parent = NULL;
    gameObject->name = NULL;
    gameObject->transform = NULL;
    allocTransformComponent(&gameObject->transform);
    gameObject->pyGameObject = NULL;
    initializePyGameObject(gameObject);

    (*gameObjectPtr) = gameObject;
    gameObject = NULL;
}

void deleteGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) == NULL) return;

    struct GameObject *gameObject = (*gameObjectPtr);
    deleteComponentListNode(&gameObject->components);
    deleteChildListNode(&gameObject->children);
    gameObject->parent = NULL;

    deleteString(&gameObject->name);
    deleteTransformComponent(&gameObject->transform);
    Py_CLEAR(gameObject->pyGameObject);

    free(gameObject);
    gameObject = NULL;
    (*gameObjectPtr) = NULL;
}

void updateGameObject(struct GameObject *gameObject, float dt) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->update != NULL) {
            curComponent->update(curComponent, dt);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        updateGameObject(curChild->child, dt);

        curChild = curChild->next;
    }
}

void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext) {
    if (gameObject == NULL || renderingContext == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->render != NULL) {
            curComponent->render(curComponent, renderingContext);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        renderGameObject(curChild->child, renderingContext);

        curChild = curChild->next;
    }
}

void resizeGameObject(struct GameObject *gameObject, int newWidth, int newHeight) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->resize != NULL) {
            curComponent->resize(curComponent, newWidth, newHeight);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        resizeGameObject(curChild->child, newWidth, newHeight);

        curChild = curChild->next;
    }
}

void attachChild(struct GameObject *parent, struct GameObject *newChild) {
    if (parent == NULL || newChild == NULL) return;

    struct String *newChildName = getGameObjectName(newChild);
    if (newChildName == NULL) {
        error_log("%s", "Cannot attach child game object unless it has a name");
        return;
    }

    struct ChildListNode *prevNode = NULL, *curNode = parent->children;
    while (curNode != NULL) {
        if (stringEquals(getGameObjectName(curNode->child), newChildName)) {
            error_log("%s", "Cannot attach child game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ChildListNode *newNode = NULL;
    allocChildListNode(&newNode);
    if (newNode == NULL) return;
    newNode->child = newChild;

    if (prevNode == NULL) {
        parent->children = newNode;
    } else {
        prevNode->next = newNode;
    }
    newChild->parent = parent;
}

void removeChild(struct GameObject *gameObject, struct GameObject *target) {
    critical_log("%s", "GameObject::removeChild is not yet implemented");

    // TODO: implement GameObject::removeChild
}

void removeChildByName(struct GameObject *gameObject, const char* name) {
    critical_log("%s", "GameObject::removeChildByName is not yet implemented");

    // TODO: implement GameObject::removeChildByName
}

struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name) {
    if (gameObject == NULL || name == NULL) return NULL;

    if (stringEqualsCStr(gameObject->name, name)) {
        return gameObject;
    }

    struct ChildListNode *curNode = gameObject->children;
    struct GameObject *ret = NULL;
    while (curNode != NULL) {
        ret = findGameObjectByName(curNode->child, name);
        if (ret != NULL) {
            return ret;
        }

        curNode = curNode->next;
    }

    return ret;
}

void attachComponent(struct GameObject *gameObject, struct BaseComponent *newComponent) {
    if (gameObject == NULL || newComponent == NULL) return;

    struct String *newComponentName = getComponentName(newComponent);
    if (newComponentName == NULL) {
        error_log("%s", "Cannot attach component to game object unless it has a name");
        return;
    }

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEquals(getComponentName(curNode->component), newComponentName)) {
            error_log("%s", "Cannot attach component to game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ComponentListNode *newNode = NULL;
    allocComponentListNode(&newNode);
    if (newNode == NULL) return;

    newNode->component = newComponent;

    if (prevNode == NULL) {
        gameObject->components = newNode;
    } else {
        prevNode->next = newNode;
    }
    newComponent->_owner = gameObject;
}

void attachPyComponent(struct GameObject *gameObject, PyObject *newPyComponent) {
    if (gameObject == NULL || newPyComponent == NULL) return;

    if (PyObject_HasAttrString(newPyComponent, "get_name") < 0) {
        error_log("%s", "[GameObject]: Cannot attach component to game object unless it has a get_name property");
        return;
    }
    PyObject *get_name = PyObject_GetAttrString(newPyComponent, "get_name");
    if (PyCallable_Check(get_name) < 0) {
        error_log("%s", "[GameObject]: Cannot attach component to game object unless its get_name property is callable");
        return;
    }

    PyObject *pyComponentName = PyObject_CallNoArgs(get_name);
    if (pyComponentName == NULL) {
        error_log("%s", "[GameObject]: An error occurred trying to get the name of a python component");
        handleException();
        return;
    }
    if (Py_IsNone(pyComponentName)) {
        error_log("%s", "[GameObject]: Cannot attach component to game object unless it has a name");
        return;
    }
    char *pyComponentNameAsCStr = PyUnicode_AsUTF8(pyComponentName);

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (componentNamesAreEqual(curNode, pyComponentNameAsCStr)) {
            error_log("[GameObject]: Cannot attach python component named \"%s\". Component names must be unique");
            return;
        }

        // TODO: finish attaching component

        prevNode = curNode;
        curNode = curNode->next;
    }
}

struct BaseComponent *getGameObjectComponentByType(struct GameObject *gameObject, const char *typeName) {
    if (gameObject == NULL || typeName == NULL) return NULL;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEqualsCStr(getComponentTypeName(curNode->component), typeName)) {
            return curNode->component;
        }

        curNode = curNode->next;
    }

    return NULL;
}

struct String *getGameObjectName(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->name;
}

void setGameObjectName(struct GameObject *gameObject, const char *newName) {
    if (gameObject == NULL) return;

    if (gameObject->name == NULL) {
        allocString(&(gameObject->name), newName);
    } else {
        setChars(gameObject->name, newName);
    }
}

struct TransformComponent *getGameObjectTransform(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->transform;
}
