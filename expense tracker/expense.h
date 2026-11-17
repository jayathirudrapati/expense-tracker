#ifndef EXPENSE_H
#define EXPENSE_H

#include <stdio.h>

#define MAX_EXP  1000
#define NUM_CAT  6

extern const char *CATEGORIES[NUM_CAT];

typedef struct {
    int   id;
    char  date[11];      /* YYYY-MM-DD */
    char  category[20];
    char  desc[50];
    float amount;
} Expense;

/* File handling */
int   load_expenses(const char *filename, Expense list[]);
int   save_expenses(const char *filename, const Expense list[], int count);
float load_budget(const char *filename);
void  save_budget(const char *filename, float budget);

/* Core operations */
int  add_expense(Expense list[], int *count);
void view_expenses(const Expense list[], int count);
int  delete_expense(Expense list[], int *count);

/* Spending pattern analysis (output goes to any stream: screen or file) */
void category_summary(FILE *out, const Expense list[], int count);
void monthly_summary(FILE *out, const Expense list[], int count);
void analyze_patterns(FILE *out, const Expense list[], int count, float budget);
int  export_report(const char *filename, const Expense list[], int count, float budget);

#endif