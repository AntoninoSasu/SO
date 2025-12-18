#ifndef SIMPLE_RECORD_H
#define SIMPLE_RECORD_H

#define LABEL_MAX_LEN 15

typedef struct {
    int id;
    double value;
    char label[LABEL_MAX_LEN + 1];
} SimpleRecord;

#endif

