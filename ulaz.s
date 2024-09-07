.global a, b
.extern c

.section sekcija1
  .word 0x1122
  d: .skip 4
  .word 0x3344
  .word d
.section sekcija2

.end
