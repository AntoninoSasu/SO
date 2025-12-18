#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

int main(int argc, char* argv[])
{
	if(argc < 3){
		perror("Usage: ./write_strings <output_file> <string1> <string2> ... <stringN>\n");
		exit(1);
	}
    FILE* file=NULL;
	file = fopen(argv[1], "w");
	if (file == NULL){
		err(2, "The input file %s could not be opened", argv[1]);
	}

	for(int i = 2; i < argc; i++){
		ssize_t longitud = strlen(argv[i]) + 1;
		size_t escrito = fwrite(argv[i], sizeof(char), longitud, file);
		if (escrito != longitud){
			err(3, "Error writing string %s to file %s", argv[i], argv[1]);
		}
	}

	fclose(file);	
	return 0;
}
