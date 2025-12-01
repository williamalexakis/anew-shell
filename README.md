# Void Shell

## Overview

Void Shell, or Vsh, is a custom, simple Unix-style shell focusing on speed, minimalism, and clarity.

## Features

- **Minimal Builtin Set** – Unix model is preserved and overlap with existing utilities is prevented.
- **Recursive-Descent Parser** – Semantics are clear, controllable, and extensible.
- **Builtin Debug Modes** – Display token streams and AST structure for convenience.
- **Modular Architecture** – New features and extensions are easily accommodated.

## Examples

### Commands

Standard Unix commands work as usual:

```bash
vsh> pwd
/your/current/path
vsh> echo hello world
hello world
vsh> echo hello; echo world
hello
world
vsh> echo hello world | tr a-z A-Z
HELLO WORLD
```

### Debug Builtins

Debug commands require the input to be passed as a single quoted string.

For example, using `tokens`:

```bash
vsh> tokens "echo hello world"
4 | WORD 'echo'
10 | WORD 'hello'
16 | WORD 'world'
16 | EOF
```

And using `ast`:

```bash
vsh> ast "echo hello world"
SEQUENCE
    ╰ PIPELINE
        ╰ COMMAND (ECHO) ["hello", "world"]
```

## Installation

### Prerequisites

- POSIX-compatible system
- C compiler supporting C17
- CMake 3.20+

### Building

1. Clone the repo:
    ```sh
    git clone https://github.com/williamalexakis/void-shell.git
    cd void-shell
    ```
    
2. Build the project:
    ```sh
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

### Usage

To get started, run the executable and use `help` to see all builtin commands.
