#ifndef PY3DENGINE_BASE_RESOURCE_H
#define PY3DENGINE_BASE_RESOURCE_H

#include <stdbool.h>

struct String;

struct BaseResource {
    unsigned int _type;
    struct String *_typeName;
    struct String *_name;
    void (*delete)(struct BaseResource **);
};

extern void initializeBaseResource(struct BaseResource *resource);
extern void finalizeBaseResource(struct BaseResource *resource);
extern void deleteResource(struct BaseResource **resource);

extern bool resourceTypesEqual(struct BaseResource *r1, struct BaseResource *r2);
extern bool resourceNamesEqual(struct BaseResource *r1, struct BaseResource *r2);

extern struct String *getResourceName(struct BaseResource *resource);
extern void setResourceName(struct BaseResource *resource, const char *newName);

extern struct String *getResourceTypeName(struct BaseResource *resource);

#endif
