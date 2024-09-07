.global a, b
.extern c

.section sekcija1
  .word 0x1122
  a: not %r1
  b: and %r2, %r3
  .word a, b, 0x3344
.end
