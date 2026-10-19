
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int ac, char **av) {
	char *outf = NULL;
	char *inf = NULL;
	if (ac < 2 || ac > 3) {
		printf("too many or few arguments.\n");
		return 1;
	}
	for (int i = 0; i < ac; i++) {
		if (strncmp(av[i], "if=", 3) == 0) {
			inf = av[i] + 3;
		} else if (strncmp(av[i], "of=", 3) == 0) {
			outf = av[i] + 3 ;
		} else if (av[i][0] == '-') {
			switch (av[i][1]) {
				case 'h': printf("Usage:\n\t%s if=[FILE] of=[FILE]\n\t\t-h # prints this menu\n\t\t-v # prints version\n\t\t-g # prints guide\n", av[0]); return 0;
				case 'v': printf("Version v1.0\n"); return 0;
				case 'g': printf("Guide:\t[] :: loops until the current cell is 0\n\t:  :: prints the current cell into stderr\n\t.  :: prints the current cell into stdout\n\t,  :: gets 1 character from stdin\n\t@  :: exits with the current cell as the value\n\t<  :: switches to the cell on the left\n\t>  :: switches to the cell on the right\n\t#  :: puts random into the current cell (really old CPUs are not supported)\n\t\\  :: clears the screen\n\t!  :: returns to cell one\n"); return 0;
				default: printf("Invalid argument: %s\n", av[i]); return 1;
			}
		}
	}
	if (inf == NULL) {
		printf("No input file given. Please give an input file with if=[FILE]\n");
		return 1;
	}

	FILE *src = fopen(inf, "r");
	if (src == NULL) {
		printf("Input file \"%s\" not found.\n", inf);
		return 2;
	}
	fseek(src, 0, SEEK_END);
	long size = ftell(src);
	rewind(src);

	char *buffer = malloc(size);
	fread(buffer, 1, size, src);
	fclose(src);
	
	if (outf == NULL) outf = "a.out";
	FILE *out = fopen("a.asm", "w");

	fprintf(out, "\n.data\ncls: .ascii \"\\033[2J\\033[H\"\nlen = . - cls\n.bss\ncells: .skip 30000\n.text\n.globl _start\n");
	fprintf(out, "clear:\nmov $1, %%rax\nmov $1, %%rdi\nlea cls(%%rip), %%rsi\nmov $len, %%rdx\nsyscall\nret\n\n_start:\nxor %%rdi, %%rdi\n");

	int exit = 0;
	int loop = 0;
	int loops[1024];
	int next = 1;
	
	for (int i = 0; i < size; i++) {
		switch (buffer[i]) {
			case '+': fprintf(out, "incb cells(%%r12)\n"); break;
			case '-': fprintf(out, "decb cells(%%r12)\n"); break;
			
			case '>': fprintf(out, "inc %%r12\n"); break;
			case '<': fprintf(out, "dec %%r12\n"); break;
			
			case '@': fprintf(out, "mov $60, %%rax\nmovzbl cells(%%r12), %%edi\nsyscall\n"); exit = 1; break;
			case '\\': fprintf(out, "call clear\n"); break;

			case '#': fprintf(out, "rdrand %%ax\nmov %%al, cells(%%r12)\n"); break;
			case '!': fprintf(out, "mov $0, %%r12\n"); break;

			case '.': fprintf(out, "mov $1, %%rax\nmov $1, %%rdi\nlea cells(%%r12), %%rsi\nmov $1, %%rdx\nsyscall\n"); break;
			case ',': fprintf(out, "mov $0, %%rax\nmov $0, %%rdi\nlea cells(%%r12), %%rsi\nmov $1, %%rdx\nsyscall\n"); break;
			case ':': fprintf(out, "mov $1, %%rax\nmov $2, %%rdi\nlea cells(%%r12), %%rsi\nmov $1, %%rdx\nsyscall\n"); break;

			case '[': fprintf(out, "loop%d:\ncmpb $0, cells(%%r12)\nje end%d\n", next, next); loops[loop] = next; loop++; next++; break;
			case ']': loop--; fprintf(out, "cmpb $0, cells(%%r12)\njne loop%d\nend%d:\n", loops[loop], loops[loop]); break;
		}
	}

	if (exit == 0) {
		fprintf(out, "\n\nmov $60, %%rax\nmov $0, %%rdi\nsyscall\n");
	}

	fprintf(out, "\n\n.section .note.GNU-stack, \"\", @progbits\n");
	fclose(out);
	
	char sysbuf[1024];
	snprintf(sysbuf, 1024, "as a.asm -o a.o ; ld a.o -o %s ; rm a.o #; rm a.asm", outf);
	system(sysbuf);

	free(buffer);
	return 0;
}

