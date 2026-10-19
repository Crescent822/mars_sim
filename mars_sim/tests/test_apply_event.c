
#include <stdio.h>
#include "../action.h"

int main(void)
{
    SimState state = {0};

    state.base.POWER = 100;
    state.base.OXYGEN = 80;
    state.base.WATER = 60;
    state.base.TEMPERATURE = 22;
    state.base.HULL = 100;
    state.base.ROBOT = 1;

    Event event = {0};
    snprintf(event.name, sizeof(event.name), "SOLAR_FAULT");
    event.change_count = 2;

    snprintf(event.resource_names[0],
             sizeof(event.resource_names[0]), "POWER");
    event.changes[0] = -20;

    snprintf(event.resource_names[1],
             sizeof(event.resource_names[1]), "HULL");
    event.changes[1] = -5;

    if (!apply_event(&state, &event)) {
        printf("FAIL: apply_event returned 0\n");
        return 1;
    }

    printf("POWER=%d\n", state.base.POWER);
    printf("HULL=%d\n", state.base.HULL);
    printf("SOLAR_FAULT=%d\n", state.FAULT_SOLAR);

    if (state.base.POWER == 80 &&
        state.base.HULL == 95 &&
        state.FAULT_SOLAR == 1) {
        printf("PASS\n");
        return 0;
    }

    printf("FAIL\n");
    return 1;
}
