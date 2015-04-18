# rshell
cs 100 shell

## Building and Running
```bash
make
./bin/rshell
```

## Features
* Supports up to 1000 chained commands in a single entry
* Includes login and hostname in prompt
```bash
kkita004@hammer.cs.ucr.edu$
```
* Automatically trims tabs (at beginning and end of input) and excess spaces (at all parts) from commands
```bash
kkita004@hammer.cs.ucr.edu$               ls                                   -a             -a;
```
becomes
```bash
kkita004@hammer.cs.ucr.edu$ ls -a -a;
```
* Supports both single and double quotes
```bash
kkita004@hammer.cs.ucr.edu$ echo "This sentence will be output."
This sentence will be output.
kkita004@hammer.cs.ucr.edu$ echo "The ; && and || will be ignored while in these quotes."
The ; && and || will be ignored while in these quotes.
```
An error will be detected if quotes are left unclosed.

* Empty input is ignored

* If an empty command is entered, it will stop the remaining chain of commands from working
```bash
kkita004@hammer.cs.ucr.edu$ echo testing &&     ; echo "will not appear"
testing
```

## Dependencies
Uses the [`boost`](https://www.boost.org) library for trimming and tokenizing.
Uses `C++11` features.

## Known Bugs and Limitations
* Inline tabs will not be ignored in commands
```bash
kkita004@hammer.cs.ucr.edu$ ls -a \t\t\t -a
: invalid option -- ' '
Try 'ls --help' for more information.
```

* Cannot handle hostnames or login names that are longer than 256 characters.

* Does not currently handle input/output redirection or piping

* Does not support escaped quotes
```bash
echo 'You're it!';
error: no closing single quotation found
```

