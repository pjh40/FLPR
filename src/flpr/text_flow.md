# Text Flow Design Notes #

## Features ##

* "Expandable" lines: `INCLUDE` and `#include`
* "Expandable" statements: `USE`
* Block alternatives: `#if` 
* Groups that intermingle program and comment lines, with easy
  identification of the former.
* No absolute numbering of lines or of main text lines/cols. 
  * Identify text and whitespace *segments*.  A Fortran token may be
    made up of multiple text segments if it is split across lines with
    a continuation.
  * There are a variety of breaks to track:
    1. Intra-token continuation new line
	2. Inter-token continuation new line
	3. End-of-statement new line
	4. Non-advancing end-of-statement `;`.
	
## Preprocessor Considerations ##
When considering a source file that contains standard C preprocessor
directives or Fortran `INCLUDE` directives, it is tempting to view
them as somehow part of the program text itself. However, a more
accurate view is that they produce a tree of possible text file
inputs.  A compiler has the advantage of knowning the definitions of
all preprocessor symbols as it compiles the code, allowing it to
discard ignored text from the stream and follow one path in the tree
of files. FLPR, on the other hand, may need to do transformations
based on any possible set of preprocessor symbols. 

### Impact of conditionals ###
For ease of analysis, assume that any conditional that is simply an
"if/end" pair can be replaced with "if/else/end", with an empty else
clause.  For example: 
```c
#ifdef FOO
  /* controlled text */
#endif
```
becomes
```c
#ifdef FOO
  /* controlled text */
#else
  /* empty */
#endif
```

L1
L2
#if blah
L4
#else
#endif
L7
L8
=>
L1
L2 /L4
  * 
   \<empty?
L7
L8

Each `#elif` adds another child to the branch.  Each child can have
its own branches.

While you could show this as a tree, it requires less duplication if
it is a dag, with a node for each block of possible text, and a
possible input is one path from the root to terminal node.

### Impact of file includes ###


### Impact of function-like macros ###




  
