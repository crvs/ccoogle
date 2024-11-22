C approximation of coogle
=====

To build should be enough to do:

```bash
$ make
```
Example of fuzzy matching on signature:

```bash
$ xargs ccoogle <list files> | fzf
```

or from vim do

```vim
:!ccoogle /usr/X11R6/include/*.h /usr/X11R6/include/**/*.h > X11R6_signatures.lst
:cfile X11R6_signatures.lst
```

This populates the quickfix window with a list of signatures from the `X11R6` library, so that pressing enter on a line transports you to the corresponding signature.
