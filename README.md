# ptbm
Parentheses Tree Based Multicast header processor

## Usage:

```ptbm [options]```

**Options:**

* **--brackets BRACKETS**
Brackets in the header (default: none).
For example: ()(())
* **--numbers NUM1,NUM2,NUM3..**
Numbers in the header (default: none)
For example: 3,0,8
* **--virtual NUM1,NUM2,NUM3..**
Virtual ports (default: none).
* **--generate**
Generate header (binary)
* **--input**
Input header (binary)
* **--print**
Print header
* **--help**
Print usage

### Scenerio 1

You have the multicast tree in its textual form (brackets and numbers) and the router has one virtual port (number: 1). You want simulate the processing of this header by the router.

Command to execute:
```./ptbm --brackets "()((()())()())((()()())())(())((()(()()()))())" --numbers 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 --virtual 1```

Result:
```
2 *
3 (()())()() 4,5,6,7,8
9 (()()())() 0,0,1,2,10
11 () 12
32 ()(()()()) 2,3,4,5,6
33 *
```

### Scenerio 2

Converting the multicast tree from its textual form (brackets and numbers) to its binary form.

Command to execute:
```--brackets "()((()())()())((()()())())(())((()(()()()))())" --numbers 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 --generate```
Result:
```0000000000000000000000000000000000000000000000000000000000000000000000000000000000010110010101000011001000000001110010111010001000010000000010011000011101100101010000110010000000000000000000000000000000000000000010001010110111001100100101011100101001011101```

### Scenerio 3

Converting the multicast tree from its its binary form to its textual form (brackets and numbers).

Command to execute:
```--input --virtual 1 --print```
Note: you have to enter binary from the console or redirect STDIN

Result:
```()((()())()())((()()())())(())((()(()()()))()) 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1```

### Scenerio 4

Simulate processing of the header in its binary form.

```--input --virtual 1 --print```
Note: you have to enter binary from the console or redirect STDIN

Result:
```
2 *
3 (()())()() 4,5,6,7,8
9 (()()())() 0,0,1,2,10
11 () 12
32 ()(()()()) 2,3,4,5,6
33 *
```

## Build
Program should build on any UNIX like or Windows operation system with a standard C++11 compiler, qmake and make utility.

To compile:
```
qmake ptbm.pro
make
```

To run:
```
./ptbm --help
```
To clean:
```
make clean
```

## Author

Written by Andras Majdan.

License: GNU General Public License Version 3

Report bugs to <majdan.andras@gmail.com>
