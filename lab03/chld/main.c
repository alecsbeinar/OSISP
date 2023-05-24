#include <stdbool.h>

#include "child.h"

static const binary_t zero  = {0, 0};
static const binary_t three = {1, 1};

binary_t data;
control_t control;

int alarm_count;
bool output_allowed;

int main(void) {
    struct itimerval timer;
    initialization(&timer);

    while (1) {
        // nonatomic operations - not implement in one step
        // = running in 2 steps, so alarm signal can be raised when operation no finished
        // we recive unexpected result
        // bytes in data proceed, stat see
        data = zero;
        data = three;

        if (alarm_count == ALARM_COUNTS_TO_PRINT) {
            ask_to_print();
            if (output_allowed) {
                print_info();
                inform_about_print();
            }
            reset_cycle();
        }
    }
}
