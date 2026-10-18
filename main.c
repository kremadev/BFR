
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int ac, char **av) {
	char *outf;
	char *inf;
	for (int i = 0; i < ac; i++) {
		if (strstr(av[i], "if=") != NULL) {
			inf = av[i] + 3;
		} else if (strstr(av[i], "of=") != NULL) {
			outf = av[i] + 3 ;
		}
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

	fprintf(out, "default rel\n\nsection .bss\ncells resb 30000\nsection .text\nglobal _start\n_start:\nmov r12, 0\n");

	int loop = 0;
	int loops[1024];
	int next = 1;
	
	for (int i = 0; i < size; i++) {
		switch (buffer[i]) {
			case '+': fprintf(out, "inc [cells+r12]\n"); break;
			case '-': fprintf(out, "dec [cells+r12]\n"); break;
			
			case '>': fprintf(out, "inc r12\n"); break;
			case '<': fprintf(out, "dec r12\n"); break;
			
			case '@': fprintf(out, "mov rax, 60\nmov rdi, [cells+r12]\nsyscall\n"); break;

			case '.': fprintf(out, "mov rax, 1\nmov rdi, 1\nlea rsi, [cells+r12]\nmov rdx, 1\nsyscall\n"); break;
			case ',': fprintf(out, "mov rax, 0\nmov rdi, 0\nlea rsi, [cells+r12]\nmov rdx, 1\nsyscall\n"); break;
			case ':': fprintf(out, "mov rax, 1\nmov rdi, 2\nlea rsi, [cells+r12]\nmov rdx, 1\nsyscall\n"); break;

			case '[': fprintf(out, "loop%d:\ncmp byte [cells+r12], 0\nje end%d\n", next, next); loops[loop] = next; loop++; next++; break;
			case ']': loop--; fprintf(out, "cmp byte [cells+r12], 0\njne loop%d\nend%d:\n", loops[loop], loops[loop]); break;
		}
	}
	fclose(out);
	
	char sysbuf[1024];
	snprintf(sysbuf, 1024, "nasm -felf64 a.asm -o a.o && ld a.o -o %s && rm a.o ; rm a.asm\0", outf);
	system(sysbuf);

	free(buffer);
	return 0;
}

