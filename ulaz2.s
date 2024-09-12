.global a
.extern b

.section sekcija1
  ld b, %r4
.section sekcija2
  a:
  .word 0x1122
.end