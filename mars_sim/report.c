#include <stdio.h>
#include "report.h"

int write_report(const char *filename, const SimState *state,
                 const char *result)
{
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        return 0;
    }

    int score = state->TIME
              + 10 * state->REPAIRED
              - 5 * state->FAILED_ACTION
              + state->base.POWER / 10
              + state->base.OXYGEN / 10
              + state->base.WATER / 10
              + state->base.HULL / 10;

    if (score < 0) {
        score = 0;
    }

    fprintf(fp, "RESULT %s\n", result);
    fprintf(fp, "TIME %d\n", state->TIME);
    fprintf(fp, "POWER %d\n", state->base.POWER);
    fprintf(fp, "OXYGEN %d\n", state->base.OXYGEN);
    fprintf(fp, "WATER %d\n", state->base.WATER);
    fprintf(fp, "TEMPERATURE %d\n", state->base.TEMPERATURE);
    fprintf(fp, "HULL %d\n", state->base.HULL);
    fprintf(fp, "ROBOT %d\n", state->base.ROBOT);
    fprintf(fp, "ACTIVE_ROBOT %d\n", state->ACTIVE_ROBOT);
    fprintf(fp, "REPAIRED %d\n", state->REPAIRED);
    fprintf(fp, "FAILED_ACTION %d\n", state->FAILED_ACTION);
    fprintf(fp, "SCORE %d\n", score);

    if (fclose(fp) != 0) {
        return 0;
    }

    return 1;
}


int write_error_report(const char *filename, const char *reason)
{
    FILE *fp;

    if (filename == NULL || reason == NULL) {
        return 0;
    }

    fp = fopen(filename, "w");

    if (fp == NULL) {
        return 0;
    }

    fprintf(fp, "RESULT ERROR\n");
    fprintf(fp, "REASON %s\n", reason);

    if (fclose(fp) != 0) {
        return 0;
    }

    return 1;
}
