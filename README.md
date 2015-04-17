# rshell
cs 100 shell

## How to build and run
```bash
make
./bin/rshell
```

## Features
1. Supports up to 1000 chained commands in a single entry
2. Automatically trims tabs and spaces from commands
```bash
$ ls                                   -a             -a;
```
becomes
```bash
$ ls -a -a;
```
3. Supports both single and double quotes
```bash
$ echo "This sentence will be output."
This sentence will be output.
$ echo "The ; && and || will be ignored while in these quotes."
The ; && and || will be ignored while in these quotes.
```

## Known Bugs
1. Inline tabs will not be ignored in commands
```bash
$ ls -a \t\t\t -a
: invalid option -- ' '
Try 'ls --help' for more information.
```
