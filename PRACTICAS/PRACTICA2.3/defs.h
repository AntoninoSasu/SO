#ifndef DEFS_H
#define DEFS_H

#define LABEL_MAX_LEN 15 // caracteres sin contar el \0

typedef struct {
	int record_id; 
	double value;	
	char* label[LABEL_MAX_LEN + 1];
} SimpleRecord;


#endif