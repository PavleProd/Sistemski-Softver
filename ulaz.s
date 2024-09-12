.extern a
.global b

.section sekcija1
  beq %r1, %r2, a
  halt
.section sekcija2
  b: .word b
  c: .skip 4
.end
