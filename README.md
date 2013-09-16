grepscan
========

grep a substring in a file


----

compile only configuration at the moment.

1. edit grepscan.c
2. change KEY to the string you want to find
3. change KEYMATCH to the minimum characters of KEY you want matched
4. compile with:
```sh
gcc -Wall -O3 -o grepscan grepscan.c
```

5. run it as:
```sh
./grepscan /path/*
```
to search all files in /path.
