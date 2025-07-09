# MiniShell

A lightweight Unix-like shell built in C that supports command execution, piping, I/O redirection, background processes, built-in commands, and in-memory command history.

## 🚀 Features

-  Command execution (e.g. `ls`, `cat`, `grep`)
-  Output redirection: `command > file.txt`
-  Input redirection: `command < file.txt`
-  Piping: `ls -l | grep .c`
-  Background process support: `sleep 5 &`
- Built-in commands:
  - `cd <dir>` – change working directory
  - `exit` – quit shell
  - `history` – view command history
-  Basic signal handling (e.g. `Ctrl+C`)

## Example Usage
```
sh
Copy
Edit
MiniShell$ ls -l
MiniShell$ cat input.txt | grep keyword > result.txt
MiniShell$ cd /tmp
MiniShell$ history
MiniShell$ sleep 10 &
MiniShell$ exit
```