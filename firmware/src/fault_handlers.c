#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

__attribute__((naked))
hard_fault_handler(void) {
    __asm(
        "MOVS   R0, #4  \n"
        "MOV    R1, LR  \n"
        "TST    R0, R1  \n"
        "BEQ    _MSP    \n"
        "MRS    R0, PSP \n"
        "B      HardFault_HandlerC  \n"
        "_MSP:       \n"
        "MRS    R0, MSP \n"
        "B      HardFault_HandlerC  \n"
    );
}

void HardFault_HandlerC(uint32_t *hardfault_args) {
    write_std_out("Hard fault handler\r\n", strlen("Hard fault handler\r\n"));
}

void vAssertCalled(const char *file, const char *line) {
    char *msg;
    taskDISABLE_INTERRUPTS();
    
    write_std_out("Assert called ", strlen("Assert called "));
    write_std_out(file, strlen(file));
    write_std_out(":", 1);
    write_std_out(line, strlen(line));
    write_std_out("\r\n", 2);
    
    msg = "In thread ";
    write_std_out(msg, strlen(msg));
    msg = pcTaskGetName(NULL);
    write_std_out(msg, strlen(msg));
    
    while(1) {;}
}