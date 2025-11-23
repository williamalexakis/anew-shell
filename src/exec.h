#ifndef EXEC_H
#define EXEC_H

#include "ast.h"

#define SHELL_STATUS_EXIT (-1)  // Shell should exit when receiving this value

int exec_sequence(const AstSequence *sequence);

#endif
