#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expense.h"

#define DATA_FILE   "expenses.txt"
#define BUDGET_FILE "budget.txt"
#define REPORT_FILE "report.txt"

int main(void)
{
    static Expense list[MAX_EXP];
    int count = load_expenses(DATA_FILE, list);
    float budget = load_budget(BUDGET_FILE);
    char buf[50];
    int choice;

    printf("Loaded %d saved expense(s).\n", count);

    do {
        printf("\n===== INTELLIGENT EXPENSE TRACKER =====\n");
        printf("1. Add expense\n");
        printf("2. View all expenses\n");
        printf("3. Delete expense\n");
        printf("4. Category summary\n");
        printf("5. Monthly summary\n");
        printf("6. Smart spending analysis\n");
        printf("7. Set monthly budget\n");
        printf("8. Save report to file\n");
        printf("9. Exit\n");
        printf("Choice: ");
        if (!fgets(buf, sizeof buf, stdin)) break;
        choice = atoi(buf);

        switch (choice) {
        case 1:
            if (add_expense(list, &count)) save_expenses(DATA_FILE, list, count);
            break;
        case 2: view_expenses(list, count); break;
        case 3:
            if (delete_expense(list, &count)) save_expenses(DATA_FILE, list, count);
            break;
        case 4: category_summary(stdout, list, count); break;
        case 5: monthly_summary(stdout, list, count); break;
        case 6: analyze_patterns(stdout, list, count, budget); break;
        case 7:
            printf("Enter monthly budget (Rs.): ");
            if (fgets(buf, sizeof buf, stdin)) {
                budget = (float)atof(buf);
                save_budget(BUDGET_FILE, budget);
                printf("Budget saved.\n");
            }
            break;
        case 8:
            if (export_report(REPORT_FILE, list, count, budget))
                printf("Report saved to %s\n", REPORT_FILE);
            else
                printf("Could not write report.\n");
            break;
        case 9:
            save_expenses(DATA_FILE, list, count);
            printf("Data saved. Goodbye!\n");
            break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 9);

    return 0;
}