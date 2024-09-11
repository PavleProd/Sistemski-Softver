.global a

.section sekcija1
  pop %r4
  a: 
  halt
  .skip 10
  b: .word b
.end
