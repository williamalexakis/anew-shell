# Ash

## Overview

Ash, or Anew Shell, is a simple, custom Unix-style shell focusing on speed, minimalism, and clarity.

## Features

- **Minimal builtin set** to preserve the Unix model and prevent overlap with existing utilities.
- **Recursive-descent parser** for clarity, control, and extensibility.
- **POSIX-based execution model** for accurate Unix-style behaviour.
- **Builtin debug modes** for convenience, such as displaying token streams and AST structure.
- **Modular architecture** designed to easily accommodate new features and extensions.

## Examples

### Commands

Standard Unix commands work as usual:
```sh
ash> pwd
/your/current/path
ash> echo hello world
hello world
ash> echo hello; echo world
hi
world
ash> echo hello world | tr a-z A-Z
HELLO WORLD
```

### Debug Builtins

Debug commands require the input to be passed as a single quoted string.

For example, using `tokens`:
```sh
ash> tokens "echo hello world"
4 | WORD 'echo'
10 | WORD 'hello'
16 | WORD 'world'
16 | EOF
```

And using `ast`:
```sh
ash> ast "echo hello world"
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
    git clone https://github.com/williamalexakis/ash.git
    cd ash
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
