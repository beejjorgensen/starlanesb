#ifndef COMPANY_H
#define COMPANY_H

typedef struct {
    char name[100];
    int price;
    int size;
} COMPANY;

void show_coinfo(void);
void more_coinfo(void);
int co_avail(void);

#endif
