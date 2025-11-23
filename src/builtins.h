#ifndef BUILTINS_H
#define BUILTINS_H

typedef int (*builtin_func)(int argc, char **argv);

builtin_func builtin_lookup(const char *name);

typedef struct {

    const char  *name;
    builtin_func func;

} BuiltinEntry;

#endif
