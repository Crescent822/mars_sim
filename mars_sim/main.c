#include <stdio.h>
#include <string.h>

#include "state.h"
#include "event.h"
#include "action.h"
#include "report.h"

#define REASON_MAX 256

/* 非法输入时：终端给出提示，同时向 --output 指定的文件写入错误结果 */
static int fail(const char *output_file, int status, const char *reason)
{
    if (reason == NULL || reason[0] == '\0') {
        reason = "输入文件非法";
    }

    fprintf(stderr, "Error: %s\n", reason);

    if (output_file != NULL && !write_error_report(output_file, reason)) {
        fprintf(stderr, "Error: cannot write %s\n", output_file);
    }

    return status;
}

/* 参数解析失败时也要尽量找到 --output 路径 */
static const char *find_output_argument(int argc, char *argv[])
{
    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--output") == 0) {
            return argv[i + 1];
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    const char *base_file = NULL;
    const char *events_file = NULL;
    const char *actions_file = NULL;
    const char *output_file = NULL;
    const char *error_target;
    char reason[REASON_MAX];

    BaseState base;
    Event events[MAX_EVENTS];
    Action actions[MAX_ACTIONS];
    int event_count = 0;
    int action_count = 0;

    error_target = find_output_argument(argc, argv);

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            snprintf(reason, sizeof(reason),
                     "命令行参数错误：%s 缺少取值", argv[i]);
            return fail(error_target, 1, reason);
        }

        if (strcmp(argv[i], "--base") == 0) {
            base_file = argv[i + 1];
        } else if (strcmp(argv[i], "--events") == 0) {
            events_file = argv[i + 1];
        } else if (strcmp(argv[i], "--actions") == 0) {
            actions_file = argv[i + 1];
        } else if (strcmp(argv[i], "--output") == 0) {
            output_file = argv[i + 1];
        } else {
            snprintf(reason, sizeof(reason),
                     "命令行参数错误：未知参数 %s", argv[i]);
            return fail(error_target, 1, reason);
        }
    }

    if (base_file == NULL || events_file == NULL ||
        actions_file == NULL || output_file == NULL) {
        return fail(error_target, 1,
                    "命令行参数错误：必须提供 "
                    "--base/--events/--actions/--output");
    }

    /* 读取输入文件 */
    if (!load_base(base_file, &base, reason, sizeof(reason))) {
        return fail(output_file, 1, reason);
    }

    if (!load_events(events_file, events, &event_count,
                     reason, sizeof(reason))) {
        return fail(output_file, 1, reason);
    }

    if (!load_actions(actions_file, actions, &action_count,
                      reason, sizeof(reason))) {
        return fail(output_file, 1, reason);
    }

    /* 核心测试要求行动数量 M 必须等于事件数量 N，否则返回 2 */
    if (event_count != action_count) {
        snprintf(reason, sizeof(reason),
                 "行动数量 %d 与事件数量 %d 不一致",
                 action_count, event_count);
        return fail(output_file, 2, reason);
    }

    /* 初始化模拟状态 */
    SimState state = {0};
    state.base = base;
    state.ACTIVE_ROBOT = 0;
    state.LAB_ON = 1;
    state.FAULT_SOLAR = 0;
    state.FAULT_OXYGEN = 0;
    state.FAULT_COMMUNICATION = 0;
    state.REPAIRED = 0;
    state.FAILED_ACTION = 0;
    state.TIME = 0;

    const char *result = "SURVIVED";

    /* 按时间片执行模拟 */
    for (int i = 0; i < event_count; i++) {
        if (!apply_event(&state, &events[i])) {
            snprintf(reason, sizeof(reason),
                     "内部错误：无法处理第 %d 个事件", i + 1);
            return fail(output_file, 1, reason);
        }

        apply_action(&state, actions[i].name, actions[i].target);

        if (!settle_tick(&state)) {
            result = "FAILED";
            break;
        }
    }

    /* 输出最终报告 */
    if (!write_report(output_file, &state, result)) {
        fprintf(stderr, "Error: failed to write report\n");
        return 1;
    }

    return 0;
}
