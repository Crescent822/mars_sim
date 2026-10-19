# mars_sim

「火星基地失控 72 小时」仿真程序：读取基地初始状态、事件序列和行动序列，按时间片推进仿真，
把固定格式的结果写入 `--output` 指定的文件。

## 构建

所有源文件都在项目根目录，Makefile 与讲义给出的版本一致：

```
make clean
make
```

不使用 `make` 时，等价的一步编译命令是：

```
gcc -std=c11 -Wall -Wextra -pedantic -O2 -o mars_sim main.c state.c event.c action.c report.c sim_util.c
```

在 Windows + MSYS2 下把 `-o mars_sim` 换成 `-o mars_sim.exe` 即可。

## 运行

```
./mars_sim --base base.txt --events events.txt --actions actions.txt --output result.txt
```

四个参数都是必需项，顺序任意。正常仿真时结果只写入 `--output` 指定的文件，终端只输出错误提示。

## 输出格式

运行成功时 `result.txt` 恰好 12 行：

```
RESULT SURVIVED
TIME 4
POWER 45
OXYGEN 47
WATER 56
TEMPERATURE 14
HULL 85
ROBOT 1
ACTIVE_ROBOT 1
REPAIRED 1
FAILED_ACTION 0
SCORE 35
```

非法输入时仍然写出 `--output` 文件，内容为两行，第一行 `RESULT ERROR`，第二行 `REASON 具体原因`，
例如 `REASON 数值范围错误 POWER -100`。此时程序返回非零状态：行动数量 M 与事件数量 N 不一致
返回 2，其他非法输入返回 1。

## 测试

```
bash tests/run_tests.sh
```

脚本会编译主程序和 `tests/test_*.c` 单元测试，运行讲义第 6 节的完整示例并与 `tests/expected.txt`
逐行比较，另外检查「行动数量与事件数量不一致返回 2」和「数值越界写出 RESULT ERROR」两种情况。

## 文件说明

| 文件 | 作用 |
| --- | --- |
| main.c | 命令行参数解析、时间片主循环 |
| state.c / state.h | base 文件读取、时间片结算与失败判定 |
| event.c / event.h | events 文件读取与合法性检查 |
| action.c / action.h | 事件应用、行动执行、actions 文件读取 |
| report.c / report.h | result.txt 输出（正常结果与错误结果） |
| sim_util.c / sim_util.h | 数量行解析、空行判断、错误原因记录 |
| tests/ | 公开测试（单元测试 + 端到端比较脚本） |
| report/ | 项目报告 |
