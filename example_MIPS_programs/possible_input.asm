# .data
#
# .asciiz "don\'t"
# .asciiz ""
# .asciiz "\n"
# .asciiz "hello\n"
# .asciiz "stuff \"inner quote\" stuff"
# .asciiz "stuff \'right quote\'"
# .asciiz "heyy\theyy"
# .asciiz "\thowdy"
# .asciiz "\\\\\"what?\\'\\"
# .asciiz "contraction\'t"


# escape_seqs: .asciiz ""
# endLine: .asciiz "stuff \q stuff"
# c: .ascii "c:"
# a: .asciiz "a:"
# A:	.word 3, 4, 5
# d: .half 4

.word 69000, -420
.half -1, 0
.byte 33, 34, 35
.space 4
.byte 255, 254, 253, 252

# "[This is a \\\"test \\\ comment \"with chars like # and  :]
raw_str: .ascii "raw_str"
c_str: .asciiz "c_string"

.word 7, 8, 9
test1: .asciiz "This is a test string with chars like # and \" :" # This is a comment with chars like # and \" :
.asciiz "[This is a \\\"test \\\ string \"with chars like # and  :]\\\\" #[ This is a comment with chars like # and \" :]

.text # this is the start of the data segment
main: # this is "main"

	li $s0,1 # a
	li $s1,4 # b
	add $s0,$s0,$s1 # a = a + b

	la $s3,A # address
	lw $t2,8($s3) # A[2]

	sub $t3,$s0,$s1
	add $t2,$t2,$t3

	sw $t2,4($s3)

while:	beq $s0,1,endWhile# while loop

		add $s2,$s0,$s1# comment
		sub $s0,1

		li $v0,4
		la $a0,c
		syscall

		li $v0,1
		move $a0,$s2
		syscall

		li $v0,4
		la $a0,endLine
		syscall

		li $v0,4
		la $a0,a
		syscall

		li $v0,1
		move $a0,$s0
		syscall
# sup
		li $v0,4
		la $a0,endLine
		syscall

		j while
endWhile:#boop



	li $v0, 10
	syscall# syscall




# down here



