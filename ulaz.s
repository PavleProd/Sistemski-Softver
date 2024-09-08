.global a, b

.section sekcija1
  a: ld $0x11223344, %r7
  b: .word 0x9999, a
  add %r1, %r2
  halt
.end
