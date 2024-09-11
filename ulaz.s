.global a

.section sekcija1
  st %r7, b
  a: 
  halt
  .skip 10
  b: .word b
.end
