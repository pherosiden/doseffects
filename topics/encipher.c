#include <stdio.h>
#include <conio.h>

void main()
{
   FILE *fp_in, *fp_out;
	int c, key = 98;

	fp_in = fopen("sysinfor.sys","rb");
	fp_out = fopen("sysinfor.txt","wb");
	if(!fp_in || !fp_out) {
		clrscr();
		printf("File does not exist");
		return;
	}
	while((c = fgetc(fp_in)) != EOF) {
		c = c - ~key;    // Giai ma/Ma hoa -/+
		fputc(c, fp_out);
	}
	fclose(fp_in);
	fclose(fp_out);
}