
#include <stdio.h>
#include "../action.h"

int main(void)
{
    SimState state = {0};

    state.base.POWER = 100;
    state.base.ROBOT = 1;
    state.ACTIVE_ROBOT = 0;
    state.LAB_ON = 1;
    state.FAULT_SOLAR = 1;
    state.FAULT_OXYGEN = 1;
    state.FAULT_COMMUNICATION = 1;

    /* 测试 1：启动机器人 */
    if (!apply_action(&state, "ACTIVATE", "ROBOT")) {
        printf("FAIL: ACTIVATE ROBOT\n");
        return 1;
    }

    if (state.ACTIVE_ROBOT != 1) {
        printf("FAIL: robot state incorrect\n");
        return 1;
    }

    /* 测试 2：修复太阳能故障，消耗 10 电力 */
    if (!apply_action(&state, "REPAIR", "SOLAR")) {
        printf("FAIL: REPAIR SOLAR\n");
        return 1;
    }

    if (state.base.POWER != 90 ||
        state.FAULT_SOLAR != 0 ||
        state.REPAIRED != 1) {
        printf("FAIL: solar repair state incorrect\n");
        return 1;
    }

    /* 测试 3：重启通信，消耗 5 电力 */
    if (!apply_action(&state, "RESTART", "COMMUNICATION")) {
        printf("FAIL: RESTART COMMUNICATION\n");
        return 1;
    }

    if (state.base.POWER != 85 ||
        state.FAULT_COMMUNICATION != 0) {
        printf("FAIL: communication restart state incorrect\n");
        return 1;
    }

    /* 测试 4：关闭实验舱 */
    if (!apply_action(&state, "SHUTDOWN", "LAB")) {
        printf("FAIL: SHUTDOWN LAB\n");
        return 1;
    }

    if (state.LAB_ON != 0) {
        printf("FAIL: lab state incorrect\n");
        return 1;
    }

    /* 测试 5：重复启动机器人，应失败并计数 */
    if (apply_action(&state, "ACTIVATE", "ROBOT")) {
        printf("FAIL: repeated activation should fail\n");
        return 1;
    }

    if (state.FAILED_ACTION != 1) {
        printf("FAIL: failed_action count incorrect\n");
        return 1;
    }

    printf("All action tests passed!\n");
    return 0;
}
