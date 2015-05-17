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
* Now supports piping and redirection with < > and |

* Can also append by using >>

* Can chain any number of pipes and redirection at the same time
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

* Does not support single quotes within quotes
```bash
echo 'You're it!';
error: no closing single quotation found
```
<!--- ' -->

# ls
cs 100 ls

## Building and Running
```bash
make
bin/ls
```

## Features
* Accepts multiple directory inputs
``` bash
bin/ls dir1, dir2 dir3
```
* Adjusts formatting to accomodate file sizes
* Accepts 3 flags, -a, -l, and -R in any combination
```bash
$ bin/ls -laR
$ bin/ls -Ral
$ bin/ls -R -a -l

```
Will all result in the same output
* Outputs almost exactly like standard ls

## Dependencies
Uses the [`boost`](https://www.boost.org) library for trimming and tokenizing.
Uses `C++11` features.
Requries a Unix machine.

## Known Bugs and Limitations
* Does not accept files as input, only directories
```bash
$ touch foo
$ bin/ls foo
```
* Cannot bring back the dead
* Redirection must have the program name come first
``` bash
$ < in cat > out
Will result in error
```
* Redirection does not support output redirection before a pipe
``` bash
$ cat < in > out | wc
Will result in error
```
* Redirection does not support file descriptors (e.g. 2>)
* Redirection does support string redirection (e.g. <<<)
