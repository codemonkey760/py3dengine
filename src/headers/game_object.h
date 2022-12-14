#ifndef PY3DENGINE_GAME_OBJECT_H
#define PY3DENGINE_GAME_OBJECT_H

#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct ComponentListNode;
struct TransformComponent;
struct ChildListNode;
struct BaseComponent;
struct String;
struct RenderingContext;
struct ResourceManager;
struct PyGameObject;

struct GameObject {
    struct ComponentListNode *components;
    struct ChildListNode *children;
    struct GameObject *parent;
    struct String *name;
    struct TransformComponent *transform;
    struct PyGameObject *pyGameObject;
};

extern bool PyInit_Py3dGameObject(PyObject *module);
extern bool findPyGameObjectCtor(PyObject *module);
extern void finalizePyGameObjectCtor();

extern void allocGameObject(struct GameObject **gameObjectPtr);
extern void deleteGameObject(struct GameObject **gameObjectPtr);

extern void updateGameObject(struct GameObject *gameObject, float dt);
extern void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext);
extern void resizeGameObject(struct GameObject *gameObject, int newWidth, int newHeight);

extern void attachChild(struct GameObject *parent, struct GameObject *newChild);
extern void removeChild(struct GameObject *gameObject, struct GameObject *target);
extern void removeChildByName(struct GameObject *gameObject, const char* name);

extern struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name);

extern void attachComponent(struct GameObject *gameObject, struct BaseComponent *newComponent);
extern void attachPyComponent(struct GameObject *gameObject, PyObject *newPyComponent);
extern struct BaseComponent *getGameObjectComponentByType(struct GameObject *gameObject, const char *typeName);

extern struct String *getGameObjectName(struct GameObject *gameObject);
extern void setGameObjectName(struct GameObject *gameObject, const char *newName);

extern struct TransformComponent *getGameObjectTransform(struct GameObject *gameObject);

#endif
