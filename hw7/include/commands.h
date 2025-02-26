#ifndef COMMANDS_H
#define COMMANDS_H

#include "vfs.h"

void ls_command(vnode_t *vn, char **args);
void cat_command(vnode_t *vn, char **args);
void write_command(vnode_t *vn, char **args);
void read_command(vnode_t *vn, char **args);

#endif // COMMANDS_H
