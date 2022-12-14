#ifndef PY3DENGINE_COMPONENT_FACTORY_H
#define PY3DENGINE_COMPONENT_FACTORY_H

#include <json-c/json.h>

struct BaseComponent;

extern void componentFactoryCreateComponentFromJson(const char *typeName, struct BaseComponent **componentPtr);

#endif
