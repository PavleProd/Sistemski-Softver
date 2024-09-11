.global a

.section sekcija1
  bne %r1, %r2, b
  a: 
  halt
  b: .word b
.end
