#include <stdio.h>
#include <stdlib.h>
#include <err.h>

int main(int argc, char* argv[]) {
	FILE* file=NULL;

	int  c;

    int ret;

	char buffer;

	if (argc!=2) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);

	/* Read file byte by byte 
	while ((c = getc(file)) != EOF) {
		/* Print byte to stdout 
		ret=putc((unsigned char) c, stdout);

		if (ret==EOF){
			fclose(file);
			err(3,"putc() failed!!");
		}
	}
	*/

    // Leo con fread en el buffer, c indica el tamaño de lo leido, y va hasta que sea mayor a 0
    // aunque ya para por si solo
	while (c = fread(&buffer, sizeof(char), sizeof(buffer), file) > 0){
        fwrite(&buffer, sizeof(char), sizeof(buffer), stdout);
    }


	fclose(file);
	return 0;
}