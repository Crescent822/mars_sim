#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "state.h"
#include "sim_util.h"

/* base.txt 中六个字段的顺序，用于检查字段是否缺失 */
static const char *const base_field_names[6] = {
    "POWER", "OXYGEN", "WATER", "TEMPERATURE", "HULL", "ROBOT"
};

static const long base_field_min[6] = {0, 0, 0, -100, 0, 0};
static const long base_field_max[6] = {1000, 1000, 1000, 100, 100, 10};

static void base_set_value(BaseState *base, int index, long value)
{
    switch (index) {
    case 0:
        base->POWER = (int)value;
        break;
    case 1:
        base->OXYGEN = (int)value;
        break;
    case 2:
        base->WATER = (int)value;
        break;
    case 3:
        base->TEMPERATURE = (int)value;
        break;
    case 4:
        base->HULL = (int)value;
        break;
    case 5:
        base->ROBOT = (int)value;
        break;
    default:
        break;
    }
}

int load_base(const char *filename, BaseState *base,
              char *reason, size_t reason_len)
{
    FILE *file;
    char line[256];
    char key[32];
    char value_text[64];
    char extra;
    char *end;
    long value;
    int seen[6] = {0};
    int index;
    int line_number = 0;

    if (filename == NULL || base == NULL) {
        sim_set_reason(reason, reason_len, "内部错误：参数为空");
        return 0;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        sim_set_reason(reason, reason_len,
                       "无法打开 base 文件 %s", filename);
        return 0;
    }

    /* 先清空状态，避免留下旧数据 */
    memset(base, 0, sizeof(*base));

    while (fgets(line, sizeof(line), file) != NULL) {
        int count;

        line_number++;

        /* 空行不计入记录 */
        if (sim_line_is_blank(line)) {
            continue;
        }

        /* 每行必须包含字段名和一个数值 */
        count = sscanf(line, " %31s %63s %c", key, value_text, &extra);

        if (count != 2) {
            sim_set_reason(reason, reason_len,
                           "base 文件第 %d 行格式错误", line_number);
            goto failure;
        }

        errno = 0;
        value = strtol(value_text, &end, 10);

        if (end == value_text || *end != '\0') {
            sim_set_reason(reason, reason_len,
                           "base 文件第 %d 行数值不是整数", line_number);
            goto failure;
        }

        if (errno == ERANGE) {
            sim_set_reason(reason, reason_len,
                           "base 文件第 %d 行数值超出整数范围",
                           line_number);
            goto failure;
        }

        if (strcmp(key, "POWER") == 0) {
            index = 0;
        } else if (strcmp(key, "OXYGEN") == 0) {
            index = 1;
        } else if (strcmp(key, "WATER") == 0) {
            index = 2;
        } else if (strcmp(key, "TEMPERATURE") == 0) {
            index = 3;
        } else if (strcmp(key, "HULL") == 0) {
            index = 4;
        } else if (strcmp(key, "ROBOT") == 0) {
            index = 5;
        } else {
            sim_set_reason(reason, reason_len,
                           "未知字段 %s（base 文件第 %d 行）",
                           key, line_number);
            goto failure;
        }

        if (value < base_field_min[index] ||
            value > base_field_max[index]) {
            sim_set_reason(reason, reason_len,
                           "数值范围错误 %s %ld", key, value);
            goto failure;
        }

        if (seen[index]) {
            sim_set_reason(reason, reason_len,
                           "字段重复 %s（base 文件第 %d 行）",
                           key, line_number);
            goto failure;
        }

        seen[index] = 1;
        base_set_value(base, index, value);
    }

    if (ferror(file)) {
        sim_set_reason(reason, reason_len, "读取 base 文件失败");
        goto failure;
    }

    for (int i = 0; i < 6; i++) {
        if (!seen[i]) {
            sim_set_reason(reason, reason_len,
                           "缺少必需字段 %s", base_field_names[i]);
            goto failure;
        }
    }

    fclose(file);
    return 1;

failure:
    fclose(file);
    return 0;
}


int settle_tick(SimState *state)
{
    if (state == NULL) {
        return 0;
    }

    /* 1. 消耗氧气和水 */
    if (state->LAB_ON == 1) {
        state->base.OXYGEN -= 2;
        state->base.WATER -= 1;
    } else {
        state->base.OXYGEN -= 1;
    }

    /* 2. 限制资源下限 */
    if (state->base.POWER < 0) {
        state->base.POWER = 0;
    }

    if (state->base.OXYGEN < 0) {
        state->base.OXYGEN = 0;
    }

    if (state->base.WATER < 0) {
        state->base.WATER = 0;
    }

    /* 3. 限制舱体完整度在 0 到 100 */
    if (state->base.HULL < 0) {
        state->base.HULL = 0;
    }

    if (state->base.HULL > 100) {
        state->base.HULL = 100;
    }

    /* 4. 时间推进一个时间片 */
    state->TIME++;

    /* 5. 检查基地是否失效 */
    if (state->base.OXYGEN == 0 ||
        state->base.HULL == 0 ||
        state->base.TEMPERATURE < 0) {
        return 0;
    }

    return 1;
}
