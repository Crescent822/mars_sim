
#include <stdio.h>
#include "../state.h"

int main(void)
{
    SimState state = {0};

    state.base.POWER = 100;
    state.base.OXYGEN = 10;
    state.base.WATER = 5;
    state.base.TEMPERATURE = 22;
    state.base.HULL = 100;
    state.base.ROBOT = 1;

    state.LAB_ON = 1;
    state.TIME = 0;

    /* 测试 1：实验舱开启，氧气减 2，水减 1 */
    if (!settle_tick(&state)) {
        printf("FAIL: unexpected failure in test 1\n");
        return 1;
    }

    if (state.base.OXYGEN != 8 ||
        state.base.WATER != 4 ||
        state.TIME != 1) {
        printf("FAIL: lab-on settlement incorrect\n");
        return 1;
    }

    /* 测试 2：实验舱关闭，氧气减 1，水不变 */
    state.LAB_ON = 0;

    if (!settle_tick(&state)) {
        printf("FAIL: unexpected failure in test 2\n");
        return 1;
    }

    if (state.base.OXYGEN != 7 ||
        state.base.WATER != 4 ||
        state.TIME != 2) {
        printf("FAIL: lab-off settlement incorrect\n");
        return 1;
    }

    /* 测试 3：氧气耗尽后，基地应失效 */
    state.base.OXYGEN = 1;

    if (settle_tick(&state)) {
        printf("FAIL: oxygen depletion should cause failure\n");
        return 1;
    }

    if (state.base.OXYGEN != 0 || state.TIME != 3) {
        printf("FAIL: failure state incorrect\n");
        return 1;
    }

    printf("All settlement tests passed!\n");
    return 0;
}
