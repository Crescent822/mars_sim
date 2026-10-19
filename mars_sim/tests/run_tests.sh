#!/usr/bin/env bash
# 公开测试：编译主程序与单元测试，运行讲义示例并逐行比较 result.txt
# 用法：bash tests/run_tests.sh

set -u

cd "$(dirname "$0")/.." || exit 1

CC=${CC:-gcc}
CFLAGS="-std=c11 -Wall -Wextra -pedantic -O2"
SOURCES="main.c state.c event.c action.c report.c sim_util.c"
CORE_SOURCES="state.c event.c action.c report.c sim_util.c"

BUILD_DIR=$(mktemp -d) || exit 1
FAILED=0

trap 'rm -rf "$BUILD_DIR"' EXIT

pass() {
    echo "PASS: $1"
}

fail() {
    echo "FAIL: $1"
    FAILED=1
}

# 去掉 Windows 文本模式写出的回车符，便于和仓库中的期望文件比较
to_lf() {
    tr -d '\r' < "$1"
}

# 逐行比较两个文件，打印第一处差异（不依赖 diff 命令）
compare_files() {
    local expected=$1
    local actual=$2
    local exp_line=""
    local act_line=""
    local line_number=0

    exec 3< "$expected"
    exec 4< "$actual"

    while :; do
        IFS= read -r exp_line <&3
        exp_status=$?
        IFS= read -r act_line <&4
        act_status=$?

        # 两个文件都读完则完全一致
        if [ "$exp_status" -ne 0 ] && [ "$act_status" -ne 0 ]; then
            break
        fi

        line_number=$((line_number + 1))

        if [ "$exp_line" != "$act_line" ]; then
            echo "  第 $line_number 行不一致：期望 [$exp_line]，实际 [$act_line]"
            exec 3<&-
            exec 4<&-
            return 1
        fi
    done

    exec 3<&-
    exec 4<&-
    return 0
}

echo "== 编译主程序 =="

if $CC $CFLAGS -o "$BUILD_DIR/mars_sim" $SOURCES; then
    pass "编译 mars_sim"
else
    fail "编译 mars_sim"
    exit 1
fi

echo
echo "== 单元测试 =="

for test_src in tests/test_*.c; do
    name=$(basename "$test_src" .c)

    if ! $CC $CFLAGS -I. -o "$BUILD_DIR/$name" "$test_src" $CORE_SOURCES; then
        fail "编译 $name"
        continue
    fi

    if "$BUILD_DIR/$name" > "$BUILD_DIR/$name.log" 2>&1; then
        pass "$name"
    else
        fail "$name"
        cat "$BUILD_DIR/$name.log"
    fi
done

echo
echo "== 讲义示例：base.txt / events.txt / actions.txt =="

if "$BUILD_DIR/mars_sim" --base base.txt --events events.txt \
        --actions actions.txt --output "$BUILD_DIR/result.txt"; then
    to_lf "$BUILD_DIR/result.txt" > "$BUILD_DIR/result_lf.txt"

    if compare_files tests/expected.txt "$BUILD_DIR/result_lf.txt"; then
        pass "示例结果与 tests/expected.txt 完全一致"
    else
        fail "示例结果与 tests/expected.txt 不一致"
    fi
else
    fail "示例运行返回非零状态"
fi

echo
echo "== 错误处理 =="

printf '3\nACTIVATE ROBOT\nREPAIR SOLAR\nWAIT\n' > "$BUILD_DIR/actions_m3.txt"

"$BUILD_DIR/mars_sim" --base base.txt --events events.txt \
    --actions "$BUILD_DIR/actions_m3.txt" --output "$BUILD_DIR/err_m3.txt" \
    2> /dev/null
status=$?

if [ "$status" -eq 2 ]; then
    pass "行动数量与事件数量不一致时返回 2"
else
    fail "行动数量与事件数量不一致时应返回 2，实际返回 $status"
fi

if [ "$(to_lf "$BUILD_DIR/err_m3.txt" | head -n 1)" = "RESULT ERROR" ]; then
    pass "数量不一致时写出 RESULT ERROR"
else
    fail "数量不一致时未写出 RESULT ERROR"
fi

printf 'POWER -100\nOXYGEN 80\nWATER 60\nTEMPERATURE 22\nHULL 100\nROBOT 1\n' \
    > "$BUILD_DIR/base_bad.txt"

"$BUILD_DIR/mars_sim" --base "$BUILD_DIR/base_bad.txt" --events events.txt \
    --actions actions.txt --output "$BUILD_DIR/err_range.txt" 2> /dev/null
status=$?

if [ "$status" -ne 0 ]; then
    pass "数值越界返回非零状态（实际 $status）"
else
    fail "数值越界应返回非零状态"
fi

if [ "$(to_lf "$BUILD_DIR/err_range.txt" | head -n 1)" = "RESULT ERROR" ]; then
    pass "数值越界写出 RESULT ERROR"
else
    fail "数值越界未写出 RESULT ERROR"
fi

echo

if [ "$FAILED" -eq 0 ]; then
    echo "全部公开测试通过"
else
    echo "存在未通过的测试"
fi

exit "$FAILED"
