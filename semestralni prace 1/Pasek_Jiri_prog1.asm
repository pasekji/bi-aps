.data
.align    2

.text

lw   $a0, 0xC
lw   $a1, 0x8
jal begin
end:
 beq $zero, 0, end

begin:
addi $t1, $zero, 1
addi $t8, $zero, 1
outer_cycle:
inner_cycle:
  lw  $t2, 0x0($a0)
  lw  $t3, 0x4($a0)
  slt $t4, $t2, $t3
  beq $t4, 1, step
  sw  $t3, 0x0($a0)
  sw  $t2, 0x4($a0)
step:
  addi $a0, $a0, 0x4
  addi $t1, $t1, 0x1
  slt  $t5, $t1, $a1
  beq  $t5, 1, inner_cycle
  addi $a1, $a1, -1
  lw   $a0, 0xC
  addi $t1, $zero, 1
  slt  $t6, $t8, $a1
  beq $t6, 1, outer_cycle
  jr $ra
