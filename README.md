# rshell
cs 100 shell

## Building and Running
```bash
make
./bin/rshell
```

## Features
*. Supports up to 1000 chained commands in a single entry
*. Automatically trims tabs (at beginning and end of input) and excess spaces (at all parts) from commands
```bash
$               ls                                   -a             -a;
```
becomes
```bash
$ ls -a -a;
```
* Supports both single and double quotes
```bash
$ echo "This sentence will be output."
This sentence will be output.
$ echo "The ; && and || will be ignored while in these quotes."
The ; && and || will be ignored while in these quotes.
```

An error will be detected if quotes are left unclosed.

* If an empty command is entered, it will stop the entire chain of commands.
```bash
$ ls -l; echo testing &&     ; echo "will not appear"
```

## Dependencies
Uses the [`boost`](https://www.boost.org) library for trimming and tokenizing.

## Known Bugs
* Inline tabs will not be ignored in commands
```bash
$ ls -a \t\t\t -a
: invalid option -- ' '
Try 'ls --help' for more information.
```

* Cannot handle hostnames or login names that are longer than 256 characters.

* Does not currently handle input/output redirection or piping
