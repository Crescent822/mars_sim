#include <stdio.h>
#include <string.h>
#include "../action.h"

int main(void)
{
    Action actions[MAX_ACTIONS];
    int count = 0;
    char reason[256] = "";

    if (!load_actions("tests/actions_test.txt", actions, &count,
                      reason, sizeof(reason))) {
        printf("FAIL: could not load actions (%s)\n", reason);
        return 1;
    }

    if (count != 3) {
        printf("FAIL: wrong action count\n");
        return 1;
    }

    if (strcmp(actions[0].name, "WAIT") != 0 ||
        strcmp(actions[1].name, "ACTIVATE") != 0 ||
        strcmp(actions[1].target, "ROBOT") != 0 ||
        strcmp(actions[2].name, "SHUTDOWN") != 0 ||
        strcmp(actions[2].target, "LAB") != 0) {
        printf("FAIL: action content is incorrect\n");
        return 1;
    }

    printf("All action-loading tests passed!\n");
    return 0;
}
