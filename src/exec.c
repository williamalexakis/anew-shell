#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "exec.h"
#include "builtins.h"
#include "utils.h"
#include "ast.h"

/* Apply output redirection in the child process */
static void apply_redirection(const AstCommand *command) {

    if (command->redir.type == REDIR_NONE || !command->redir.path) return;

    int flags = O_WRONLY | O_CREAT;

    if (command->redir.type == REDIR_TRUNC) {

        flags |= O_TRUNC;

    } else if (command->redir.type == REDIR_APPEND) {

        flags |= O_APPEND;

    }

    int fd = open(command->redir.path, flags, 0666);  // Get file descriptor

    if (fd < 0) {

        perror("[ash] Error");
        _exit(1);

    }

    if (dup2(fd, STDOUT_FILENO) < 0) {

        perror("[ash] Error");
        _exit(1);

    }

    close(fd);

}

/* Run a single command in a child process -- either external or builtin */
static void exec_child_command(const AstCommand *command) {

    if (command->argc == 0) _exit(0);

    builtin_func bf = builtin_lookup(command->argv[0]);  // Get the builtin command func

    if (bf) {

        int status = bf((int)command->argc, command->argv);

        _exit(status);

    }

    execvp(command->argv[0], command->argv);
    perror("[ash] Error");
    _exit(127);

}

/* Execute a pipeline */
static int exec_pipeline(const AstPipeline *pipeline, int top_level) {

    if (pipeline->count == 0) return 0;

    // If it's a single builtin, we run it in-process
    if (top_level && pipeline->count == 1) {

        const AstCommand *command = pipeline->commands[0];

        if (command->argc > 0 && command->redir.type == REDIR_NONE) {

            builtin_func bf = builtin_lookup(command->argv[0]);  // Get the builtin command func

            if (bf) {

                int status = bf((int)command->argc, command->argv);

                if (strcmp(command->argv[0], "exit") == 0) return SHELL_STATUS_EXIT;

                return status;

            }

        }

    }

    pid_t *pids = xmalloc(pipeline->count * sizeof(pid_t));  // Get process IDs
    int previous_fd = -1;  // Read from stdin for first command
    int status = 0;

    for (size_t i = 0; i < pipeline->count; ++i) {

        int pipe_fd[2] = {-1, -1};
        int is_last = {i == pipeline->count - 1};

        if (!is_last) {

            if (pipe(pipe_fd) < 0) {

                perror("[ash] Error");
                free(pids);

                return 1;

            }

        }

        pid_t pid = fork();  // Get the process ID

        if (pid < 0) {

            perror("[ash] Error");
            free(pids);

            return 1;

        }

        // Child process
        if (pid == 0) {

            if (previous_fd != -1) {

                if (dup2(previous_fd, STDIN_FILENO) < 0) {

                    perror("[ash] Error");
                    _exit(1);

                }

            }

            if (!is_last) {

                if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {

                    perror("[ash] Error");
                    _exit(1);

                }

            }

            if (previous_fd != -1) close(previous_fd);

            if (!is_last) {

                close(pipe_fd[0]);
                close(pipe_fd[1]);

            }

            apply_redirection(pipeline->commands[i]);
            exec_child_command(pipeline->commands[i]);

        // Parent process
        } else {

            pids[i] = pid;

            if (previous_fd != -1) close(previous_fd);

            if (!is_last) {

                close(pipe_fd[1]);

                previous_fd = pipe_fd[0];

            }

        }

    }

    if (previous_fd != -1) close(previous_fd);

    for (size_t i = 0; i < pipeline->count; ++i) {

        int wstatus;

        if (waitpid(pids[i], &wstatus, 0) < 0) {

            perror("[ash] Error");

        } else if (i == pipeline->count - 1) {

            if (WIFEXITED(wstatus)) {

                status = WEXITSTATUS(wstatus);

            } else if (WIFSIGNALED(wstatus)) {

                status = 128 + WTERMSIG(wstatus);

            }

        }

    }

    free(pids);

    return status;

}

int exec_sequence(const AstSequence *sequence) {

    int status = 0;

    for (size_t i = 0; i < sequence->count; ++i) {

        status = exec_pipeline(sequence->pipelines[i], 1);

        if (status == SHELL_STATUS_EXIT) return SHELL_STATUS_EXIT;

    }

    return status;

}
