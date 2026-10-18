
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int ac, char **av) {
	char *outf;
	char *inf;
	if (ac < 2 || ac > 3) {
		printf("too many or few arguments.\n");
		return 1;
	}
	for (int i = 0; i < ac; i++) {
		if (strstr(av[i], "if=") != NULL) {
			inf = av[i] + 3;
		} else if (strstr(av[i], "of=") != NULL) {
			outf = av[i] + 3 ;
		} else if (av[i][0] == '-') {
			switch (av[i][1]) {
				case 'h': printf("Usage:\n\t%s if=[FILE] of=[FILE]\n\t\t-h # prints this menu\n\t\t-v # prints version\n\t\t-g # prints guide\n", av[0]); return 0;
				case 'v': printf("Version v1.0\n"); return 0;
				case 'g': printf("Guide:\t[] :: loops until the current cell is 0\n\t:  :: prints the current cell into stderr\n\t.  :: prints the current cell into stdout\n\t,  :: gets 1 character from stdin\n\t@  :: exits with the current cell as the value\n\t>  :: switches to the cell on the left\n\t<  :: switches to the cell on the right\n"); return 0;
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

