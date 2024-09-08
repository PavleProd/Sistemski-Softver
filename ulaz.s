.global a

.section sekcija1
  ld b, %r7
  a: halt
  b: .word b
.end
