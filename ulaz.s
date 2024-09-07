.global a, b
.extern c

.section sekcija1
  d: .word 0x1122
  .skip 4
  .word 0x3344
  .word d
.section sekcija2

.end
