*This program does macroexpand for Kent Beck machine's assembly code.*

`Input: asm_files/input.asm`

`Output: asm_files/output.asm`

Macrodefinition:
```
Macroname MACRO &argument1, &argument2 ... &argumentN
...
MEND
```

Where:

`Macroname` — name of macrodefintion;

`MACRO` — begin of macrodefinition;

`MEND` — end of macrodefinition;

`&argument` — macrodefinition's argument.
