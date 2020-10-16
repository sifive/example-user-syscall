/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdlib.h>
#include <stdio.h>

#include <metal/cpu.h>
#include <metal/pmp.h>
#include <metal/privilege.h>

#define ECODE_SYSCALL_FROM_U_MODE	8

/* Create a stack for user-mode execution */
uint8_t my_stack[768] __attribute__((aligned(16)));

/* Create the register file for user mode execution */
struct metal_register_file my_regfile = {
	.sp = (riscv_xlen_t) (my_stack + sizeof(my_stack)),
};

void metal_exception_ecall_from_u_mode_handler(struct metal_cpu cpu, int ecode)
{
	if(ecode == ECODE_SYSCALL_FROM_U_MODE) {
		printf("Handling syscall from User mode\n");
		exit(0);
	} else {
		exit(7);
	}
}

void user_mode_entry_point()
{
	/* Perform a syscall from user mode */
	__asm__ volatile("ecall");

	/* If we didn't trap or control flow returns here, fail the test */
	exit(8);
}

int main()
{
	metal_pmp_init();

	/* Configure PMP 0 to allow access to all memory */
	struct metal_pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_TOR,
		.X = 1,
		.W = 1,
		.R = 1,
	};
	int rc = metal_pmp_set_region(0, config, UINTPTR_MAX);
	if(rc != 0) {
		return 5;
	}

	printf("Dropping privilege to User mode\n");

	/* Drop to user mode */
	metal_privilege_drop_to_mode(METAL_PRIVILEGE_USER, my_regfile, user_mode_entry_point);

	/* Execution should never return here */
	return 6;
}

