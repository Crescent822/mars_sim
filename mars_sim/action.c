#include "action.h"

#include <stdio.h>
#include <string.h>

#include "sim_util.h"

/* 行动行最长：动作名 + 目标名 */
#define ACTION_LINE_MAX 128

int apply_event(SimState *state, const Event *event)
{
    if (state == NULL || event == NULL) {
        return 0;
    }

    /* 应用事件中的资源变化 */
    for (int i = 0; i < event->change_count; i++) {
        const char *resource = event->resource_names[i];
        int change = event->changes[i];

        if (strcmp(resource, "POWER") == 0) {
            state->base.POWER += change;
        } else if (strcmp(resource, "OXYGEN") == 0) {
            state->base.OXYGEN += change;
        } else if (strcmp(resource, "WATER") == 0) {
            state->base.WATER += change;
        } else if (strcmp(resource, "TEMPERATURE") == 0) {
            state->base.TEMPERATURE += change;
        } else if (strcmp(resource, "HULL") == 0) {
            state->base.HULL += change;
        } else {
            return 0;
        }
    }

    /* 根据事件名称激活故障 */
    if (strcmp(event->name, "SOLAR_FAULT") == 0) {
        state->FAULT_SOLAR = 1;
    } else if (strcmp(event->name, "OXYGEN_LEAK") == 0) {
        state->FAULT_OXYGEN = 1;
    } else if (strcmp(event->name, "COMMUNICATION_LOSS") == 0) {
        state->FAULT_COMMUNICATION = 1;
    }

    return 1;
}


int apply_action(SimState *state, const char *action_name,
                 const char *target)
{
    if (state == NULL || action_name == NULL || target == NULL) {
        return 0;
    }

    /* WAIT：不直接改变基地状态，始终合法 */
    if (strcmp(action_name, "WAIT") == 0) {
        if (strcmp(target, "") == 0) {
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* ACTIVATE ROBOT */
    if (strcmp(action_name, "ACTIVATE") == 0 &&
        strcmp(target, "ROBOT") == 0) {
        if (state->base.ROBOT > 0 && state->ACTIVE_ROBOT == 0) {
            state->ACTIVE_ROBOT = 1;
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* REPAIR SOLAR */
    if (strcmp(action_name, "REPAIR") == 0 &&
        strcmp(target, "SOLAR") == 0) {
        if (state->FAULT_SOLAR == 1 &&
            state->ACTIVE_ROBOT == 1 &&
            state->base.POWER >= 10) {
            state->base.POWER -= 10;
            state->FAULT_SOLAR = 0;
            state->REPAIRED++;
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* REPAIR OXYGEN */
    if (strcmp(action_name, "REPAIR") == 0 &&
        strcmp(target, "OXYGEN") == 0) {
        if (state->FAULT_OXYGEN == 1 &&
            state->ACTIVE_ROBOT == 1 &&
            state->base.POWER >= 10) {
            state->base.POWER -= 10;
            state->FAULT_OXYGEN = 0;
            state->REPAIRED++;
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* RESTART COMMUNICATION */
    if (strcmp(action_name, "RESTART") == 0 &&
        strcmp(target, "COMMUNICATION") == 0) {
        if (state->FAULT_COMMUNICATION == 1 &&
            state->base.POWER >= 5) {
            state->base.POWER -= 5;
            state->FAULT_COMMUNICATION = 0;
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* SHUTDOWN LAB */
    if (strcmp(action_name, "SHUTDOWN") == 0 &&
        strcmp(target, "LAB") == 0) {
        if (state->LAB_ON == 1) {
            state->LAB_ON = 0;
            return 1;
        }

        state->FAILED_ACTION++;
        return 0;
    }

    /* 不认识的行动也算行动失败 */
    state->FAILED_ACTION++;
    return 0;
}


/* 判断一项行动的「动作名 + 目标」是否合法 */
static int valid_action(const char *name, const char *target, int fields)
{
    if (strcmp(name, "WAIT") == 0) {
        return fields == 1;
    }

    if (fields != 2) {
        return 0;
    }

    if (strcmp(name, "ACTIVATE") == 0) {
        return strcmp(target, "ROBOT") == 0;
    }

    if (strcmp(name, "REPAIR") == 0) {
        return strcmp(target, "SOLAR") == 0 ||
               strcmp(target, "OXYGEN") == 0;
    }

    if (strcmp(name, "RESTART") == 0) {
        return strcmp(target, "COMMUNICATION") == 0;
    }

    if (strcmp(name, "SHUTDOWN") == 0) {
        return strcmp(target, "LAB") == 0;
    }

    return 0;
}

/* 判断一行是否被缓冲区截断 */
static int line_is_too_long(const char *line, size_t size)
{
    return strlen(line) == size - 1 && line[size - 2] != '\n';
}

int load_actions(const char *filename, Action actions[], int *action_count,
                 char *reason, size_t reason_len)
{
    FILE *fp;
    char line[ACTION_LINE_MAX];
    int line_number = 0;
    int count = 0;

    if (filename == NULL || actions == NULL || action_count == NULL) {
        sim_set_reason(reason, reason_len, "内部错误：参数为空");
        return 0;
    }

    fp = fopen(filename, "r");

    if (fp == NULL) {
        sim_set_reason(reason, reason_len,
                       "无法打开 actions 文件 %s", filename);
        return 0;
    }

    if (!sim_read_count(fp, "actions", MAX_ACTIONS, &count,
                        &line_number, reason, reason_len)) {
        goto failure;
    }

    for (int i = 0; i < count; i++) {
        char name[32] = "";
        char target[32] = "";
        char extra[32] = "";
        int fields;

        /* 空行不计入记录 */
        do {
            if (fgets(line, sizeof(line), fp) == NULL) {
                sim_set_reason(reason, reason_len,
                               "actions 文件缺少第 %d 项行动", i + 1);
                goto failure;
            }

            line_number++;
        } while (sim_line_is_blank(line));

        if (line_is_too_long(line, sizeof(line))) {
            sim_set_reason(reason, reason_len,
                           "actions 文件第 %d 行过长", line_number);
            goto failure;
        }

        fields = sscanf(line, " %31s %31s %31s", name, target, extra);

        if (fields < 1 || fields > 2) {
            if (fields > 2) {
                sim_set_reason(reason, reason_len,
                               "行动行多余内容（第 %d 行）", line_number);
            } else {
                sim_set_reason(reason, reason_len,
                               "行动行格式错误（第 %d 行）", line_number);
            }
            goto failure;
        }

        if (!valid_action(name, target, fields)) {
            if (fields == 2) {
                sim_set_reason(reason, reason_len,
                               "非法行动 %s %s（第 %d 行）",
                               name, target, line_number);
            } else {
                sim_set_reason(reason, reason_len,
                               "非法行动 %s（第 %d 行）",
                               name, line_number);
            }
            goto failure;
        }

        if (fields == 1) {
            target[0] = '\0';
        }

        strcpy(actions[i].name, name);
        strcpy(actions[i].target, target);
    }

    /* 不允许多余的非空行 */
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_number++;

        if (!sim_line_is_blank(line)) {
            sim_set_reason(reason, reason_len,
                           "actions 文件第 %d 行存在多余内容",
                           line_number);
            goto failure;
        }
    }

    if (ferror(fp)) {
        sim_set_reason(reason, reason_len, "读取 actions 文件失败");
        goto failure;
    }

    fclose(fp);
    *action_count = count;
    return 1;

failure:
    fclose(fp);
    return 0;
}
