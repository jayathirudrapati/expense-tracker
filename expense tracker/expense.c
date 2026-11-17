#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "expense.h"

const char *CATEGORIES[NUM_CAT] = {
    "Food", "Transport", "Shopping", "Bills", "Entertainment", "Other"
};

/* ---------- helpers ---------- */

static void read_line(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}

static int valid_date(const char *d)
{
    int i, month, day;
    if (strlen(d) != 10) return 0;
    for (i = 0; i < 10; i++) {
        if (i == 4 || i == 7) { if (d[i] != '-') return 0; }
        else if (!isdigit((unsigned char)d[i])) return 0;
    }
    month = atoi(d + 5);
    day   = atoi(d + 8);
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

static int category_index(const char *name)
{
    int i;
    for (i = 0; i < NUM_CAT; i++)
        if (strcmp(CATEGORIES[i], name) == 0) return i;
    return NUM_CAT - 1;   /* unknown -> Other */
}

/* ---------- file handling ---------- */

int load_expenses(const char *filename, Expense list[])
{
    FILE *fp = fopen(filename, "r");
    char line[200];
    int count = 0;
    if (!fp) return 0;                       /* no file yet: start empty */

    while (count < MAX_EXP && fgets(line, sizeof line, fp)) {
        Expense e;
        if (sscanf(line, "%10[^|]|%19[^|]|%49[^|]|%f",
                   e.date, e.category, e.desc, &e.amount) == 4) {
            e.id = count + 1;
            list[count++] = e;
        }
    }
    fclose(fp);
    return count;
}

int save_expenses(const char *filename, const Expense list[], int count)
{
    int i;
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    for (i = 0; i < count; i++)
        fprintf(fp, "%s|%s|%s|%.2f\n",
                list[i].date, list[i].category, list[i].desc, list[i].amount);
    fclose(fp);
    return 1;
}

float load_budget(const char *filename)
{
    float b = 0;
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    if (fscanf(fp, "%f", &b) != 1) b = 0;
    fclose(fp);
    return b;
}

void save_budget(const char *filename, float budget)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "%.2f\n", budget);
    fclose(fp);
}

/* ---------- core operations ---------- */

int add_expense(Expense list[], int *count)
{
    Expense e;
    char buf[100], *end;
    int i, choice;

    if (*count >= MAX_EXP) { printf("Expense list is full!\n"); return 0; }

    do {
        read_line("Date (YYYY-MM-DD): ", buf, sizeof buf);
        if (!valid_date(buf)) printf("  Invalid date, try again.\n");
    } while (!valid_date(buf));
    strcpy(e.date, buf);

    printf("Category:\n");
    for (i = 0; i < NUM_CAT; i++) printf("  %d. %s\n", i + 1, CATEGORIES[i]);
    do {
        read_line("Choose (1-6): ", buf, sizeof buf);
        choice = atoi(buf);
    } while (choice < 1 || choice > NUM_CAT);
    strcpy(e.category, CATEGORIES[choice - 1]);

    do {
        read_line("Description: ", e.desc, sizeof e.desc);
    } while (e.desc[0] == '\0');
    for (i = 0; e.desc[i]; i++) if (e.desc[i] == '|') e.desc[i] = '-';

    for (;;) {
        read_line("Amount (Rs.): ", buf, sizeof buf);
        e.amount = strtof(buf, &end);
        if (end != buf && *end == '\0' && e.amount > 0) break;
        printf("  Enter a positive number.\n");
    }

    e.id = *count + 1;
    list[(*count)++] = e;
    printf("Expense added.\n");
    return 1;
}

void view_expenses(const Expense list[], int count)
{
    int i;
    if (count == 0) { printf("No expenses recorded yet.\n"); return; }
    printf("\n%-4s %-11s %-14s %-24s %10s\n", "ID", "Date", "Category", "Description", "Amount");
    printf("--------------------------------------------------------------------\n");
    for (i = 0; i < count; i++)
        printf("%-4d %-11s %-14s %-24s %10.2f\n",
               list[i].id, list[i].date, list[i].category, list[i].desc, list[i].amount);
}

int delete_expense(Expense list[], int *count)
{
    char buf[20];
    int id, i;
    view_expenses(list, *count);
    if (*count == 0) return 0;
    read_line("Enter ID to delete: ", buf, sizeof buf);
    id = atoi(buf);
    if (id < 1 || id > *count) { printf("Invalid ID.\n"); return 0; }
    for (i = id - 1; i < *count - 1; i++) {
        list[i] = list[i + 1];
        list[i].id = i + 1;
    }
    (*count)--;
    printf("Expense deleted.\n");
    return 1;
}

/* ---------- spending pattern analysis ---------- */

void category_summary(FILE *out, const Expense list[], int count)
{
    float total[NUM_CAT] = {0}, grand = 0;
    int i, j, bar;
    for (i = 0; i < count; i++) {
        total[category_index(list[i].category)] += list[i].amount;
        grand += list[i].amount;
    }
    fprintf(out, "\n=== Spending by Category ===\n");
    if (grand == 0) { fprintf(out, "No data.\n"); return; }
    for (i = 0; i < NUM_CAT; i++) {
        float pct = total[i] * 100 / grand;
        fprintf(out, "%-14s Rs.%9.2f  %5.1f%%  ", CATEGORIES[i], total[i], pct);
        bar = (int)(pct / 5);
        for (j = 0; j < bar; j++) fputc('#', out);
        fputc('\n', out);
    }
    fprintf(out, "%-14s Rs.%9.2f\n", "TOTAL", grand);
}

void monthly_summary(FILE *out, const Expense list[], int count)
{
    char months[60][8];
    float total[60] = {0};
    int n = 0, i, j, found;

    for (i = 0; i < count; i++) {
        found = -1;
        for (j = 0; j < n; j++)
            if (strncmp(months[j], list[i].date, 7) == 0) { found = j; break; }
        if (found < 0 && n < 60) {
            strncpy(months[n], list[i].date, 7);
            months[n][7] = '\0';
            found = n++;
        }
        if (found >= 0) total[found] += list[i].amount;
    }

    fprintf(out, "\n=== Monthly Totals ===\n");
    if (n == 0) { fprintf(out, "No data.\n"); return; }
    for (j = 0; j < n; j++) {
        fprintf(out, "%s  Rs.%9.2f", months[j], total[j]);
        if (j > 0) {
            float diff = total[j] - total[j - 1];
            fprintf(out, "  (%s Rs.%.2f vs previous month)",
                    diff >= 0 ? "up" : "down", diff >= 0 ? diff : -diff);
        }
        fputc('\n', out);
    }
}

void analyze_patterns(FILE *out, const Expense list[], int count, float budget)
{
    float cat_total[NUM_CAT] = {0}, grand = 0, month_total = 0;
    int   cat_count[NUM_CAT] = {0};
    int   i, top = 0, max_i = 0, max_day = 1, flagged = 0;
    char  latest[8] = "";

    fprintf(out, "\n=== Smart Spending Analysis ===\n");
    if (count == 0) { fprintf(out, "Add some expenses first.\n"); return; }

    for (i = 0; i < count; i++) {
        int c = category_index(list[i].category);
        cat_total[c] += list[i].amount;
        cat_count[c]++;
        grand += list[i].amount;
        if (list[i].amount > list[max_i].amount) max_i = i;
        if (strncmp(list[i].date, latest, 7) > 0) strncpy(latest, list[i].date, 7);
    }
    latest[7] = '\0';
    for (i = 1; i < NUM_CAT; i++) if (cat_total[i] > cat_total[top]) top = i;

    for (i = 0; i < count; i++) {
        if (strncmp(list[i].date, latest, 7) == 0) {
            int d = atoi(list[i].date + 8);
            month_total += list[i].amount;
            if (d > max_day) max_day = d;
        }
    }

    fprintf(out, "1. Average per transaction : Rs.%.2f\n", grand / count);
    fprintf(out, "2. Top spending category   : %s (Rs.%.2f, %.1f%% of total)\n",
            CATEGORIES[top], cat_total[top], cat_total[top] * 100 / grand);
    fprintf(out, "3. Biggest single expense  : Rs.%.2f on %s (%s)\n",
            list[max_i].amount, list[max_i].date, list[max_i].desc);
    fprintf(out, "4. Latest month (%s)    : Rs.%.2f spent, about Rs.%.2f per day\n",
            latest, month_total, month_total / max_day);
    fprintf(out, "5. Projected full month    : Rs.%.2f\n", month_total / max_day * 30);

    if (budget > 0) {
        float used = month_total * 100 / budget;
        fprintf(out, "6. Budget (Rs.%.2f)     : %.1f%% used\n", budget, used);
        if (used >= 100)
            fprintf(out, "   ALERT: You have exceeded this month's budget by Rs.%.2f!\n",
                    month_total - budget);
        else if (used >= 80)
            fprintf(out, "   WARNING: You have used over 80%% of your budget.\n");
        else if (month_total / max_day * 30 > budget)
            fprintf(out, "   TIP: At this pace you will cross your budget before month end.\n");
        else
            fprintf(out, "   Good: you are within budget.\n");
    } else {
        fprintf(out, "6. Budget                  : not set (use menu option to set one)\n");
    }

    fprintf(out, "7. Unusual expenses (over 2x their category average):\n");
    for (i = 0; i < count; i++) {
        int c = category_index(list[i].category);
        if (cat_count[c] >= 3 && list[i].amount > 2 * (cat_total[c] / cat_count[c])) {
            fprintf(out, "   - %s  %-14s %-22s Rs.%.2f\n",
                    list[i].date, list[i].category, list[i].desc, list[i].amount);
            flagged++;
        }
    }
    if (!flagged) fprintf(out, "   None found.\n");
}

int export_report(const char *filename, const Expense list[], int count, float budget)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    fprintf(fp, "EXPENSE TRACKER REPORT\n");
    category_summary(fp, list, count);
    monthly_summary(fp, list, count);
    analyze_patterns(fp, list, count, budget);
    fclose(fp);
    return 1;
}