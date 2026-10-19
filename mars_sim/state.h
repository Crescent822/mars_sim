#ifndef STATE_H
#define STATE_H

#include <stddef.h>

typedef struct {
    int POWER;  /* 初始电力：0-1000 */
    int OXYGEN;  /* 初始氧气：0-1000 */
    int WATER;  /* 初始水资源：0-1000 */
    int TEMPERATURE;  /* 初始温度：-100-100 */
    int HULL;   /* 初始舱体完整度：0-100 */
    int ROBOT;   /* 维修机器人总数：0-10 */
    
} BaseState;

/* 读取 base 文件；失败时把具体原因写入 reason（长度 reason_len） */
int load_base(const char *filename, BaseState *base,
              char *reason, size_t reason_len);


typedef struct {
    BaseState base;

    int ACTIVE_ROBOT;       /* 是否已激活机器人：0 或 1 */
    int LAB_ON;             /* 实验舱是否运行：0 或 1 */

    int FAULT_SOLAR;        /* SOLAR 故障：0 或 1 */
    int FAULT_OXYGEN;       /* OXYGEN 故障：0 或 1 */
    int FAULT_COMMUNICATION;/* COMMUNICATION 故障：0 或 1 */

    int REPAIRED;           /* 成功维修次数 */
    int FAILED_ACTION;      /* 行动失败次数 */
    int TIME;               /* 已处理的时间片数 */
} SimState;

int settle_tick(SimState *state);

#endif
