*This program does assembler code compilation.*

`Input: asm_files/input.asm`

`Output: output/out`


Defined directives: `begin, end, db, dw, rb, rw`.

Compiled code is written in Intel HEX format. Every line has next definition (size in bytes): 

`{:} {(1)byte count} {(2)memory adress} {(1)data type} {data bytes in LittleEndian} {(1)checksum} {CR} {LF}`

Data types:

`00` — informational line;

`01` — EOF (last line);

`02` — memory segment (first line).

Checksum is replaced by `##`.
