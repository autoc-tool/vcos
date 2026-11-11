
#include <nuttx/config.h>
#include <stdio.h>
#include "icum_main.h"

extern volatile int command;
extern void up_clear_irq(int irq);
#define HOST_TO_HSM_INIT_MAGIC_NUM   (0xEEEEEEEEu)
#define HSM_TO_HOST_INIT_MAGIC_NUM   (0xDDDDDDDDu)

void Bridge_IRQHandler(int irq, void *context, void *arg){
    volatile uint32_t HostToHsmFValue = VHSM_HAL_ICUM_PE2ICUF;
    volatile uint32_t * volatile pe2icumbuff = (uint32_t *)VHSM_HAL_ICUM_PE2ICUS;
    up_clear_irq(irq);
    if(pe2icumbuff[0] == HOST_TO_HSM_INIT_MAGIC_NUM) {
        pHOST2HSMbuf = (uint32_t *)VHSM_HAL_ICUM_PE2ICUS;
        command = CMD_SETADDR_HOST2HSMBUF;
        VHSM_HAL_ICUM_PE2ICUFC = HostToHsmFValue;
        return;
    }
    if(pe2icumbuff[0] == HSM_TO_HOST_INIT_MAGIC_NUM) {
        pHSM2HOSTbuf = (uint32_t *)VHSM_HAL_ICUM_PE2ICUS;
        command = CMD_SETADDR_HSM2HOSTBUF;
        VHSM_HAL_ICUM_PE2ICUFC = HostToHsmFValue;
        return;
    }
}
