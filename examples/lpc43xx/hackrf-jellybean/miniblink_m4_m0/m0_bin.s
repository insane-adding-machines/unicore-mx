
	.section .rodata

	.global	m0_bin
	.align  4
m0_bin:
	.incbin "../miniblink_m0_for_m4/miniblink_m0.bin"

	.global m0_bin_size
	.align  4
m0_bin_size:
	.int	m0_bin_size - m0_bin
