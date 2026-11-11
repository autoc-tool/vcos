#include <nuttx/config.h>
#include <stdio.h>
#include "icum_main.h"
volatile int command;                           // command which is executed
volatile uint32_t * volatile pHOST2HSMbuf;      //pointer to buffer for Host to HSM parameter transfer
volatile uint32_t * volatile pHSM2HOSTbuf;      //pointer to buffer for HSM to Host parameter transfer

#define BUFF_SIZE (0x100)
#define HOST_TO_HSM_BUFF_SIZE BUFF_SIZE
#define HSM_TO_HOST_BUFF_SIZE BUFF_SIZE

#define PE_TO_ICUM_INTERRUPT_NUMBER 1u
#define PE_TO_ICUM_INTERRUPT_PRI    2u
#define HSM_BRING_UP_MAGIC_NUM      0x1234567Au

#define HSM_TO_HT_BUFF_HEARTBEAT_OFFSET             0
#define HSM_TO_HT_BUFF_MAGICNUM_OFFSET              1
#define HSM_TO_HT_BUFF_CMDTYPE_OFFSET               2
#define HSM_TO_HT_BUFF_HSMSTATUS_OFFSET             3
#define HSM_TO_HT_BUFF_CRASHTYPE_OFFSET             4

volatile uint32_t uiNoCommandCnt = 0;

void __attribute__ ((noinline)) Hsm_UpdataHeartbeatNum(uint32_t updateNum)
{
    hostwrite32((uint32_t)pHSM2HOSTbuf, HSM_TO_HT_BUFF_HEARTBEAT_OFFSET, updateNum);
}
#define HSM2HT_HEART(heartBeatNum)                                  Hsm_UpdataHeartbeatNum(heartBeatNum)

#define HSM2HT_INIT_FINISH(commandType)                             \
    do {                                                            \
        VHSM_HAL_ICUM_ICU2PES = 0xCCCC0000 | commandType;             \
    } while (0)

extern void Bridge_IRQHandler(int irq, void *context, void *arg);

void u2a16icum_cmdreg_init(void)
{
  REG_ICUM_CMDREG.PE2ICUFSA0 = ICUM_COMM_PE2ICUFSA0_SET;
  REG_ICUM_CMDREG.PE2ICUFSA1 = ICUM_COMM_PE2ICUFSA1_SET;
  REG_ICUM_CMDREG.PE2ICUFSA2 = ICUM_COMM_PE2ICUFSA2_SET;
  /* Set the PE2ICUSA register            */
  REG_ICUM_CMDREG.PE2ICUSA0  = ICUM_COMM_PE2ICUSA0_SET;
  REG_ICUM_CMDREG.PE2ICUSA1  = ICUM_COMM_PE2ICUSA1_SET;
  REG_ICUM_CMDREG.PE2ICUSA2  = ICUM_COMM_PE2ICUSA2_SET;
  /* Set the PE2ICUIE register            */
  REG_ICUM_CMDREG.PE2ICUIE0  = ICUM_COMM_PE2ICUIE_SET;
  REG_ICUM_CMDREG.PE2ICUSFSA = ICUM_COMM_PE2ICUSFSA_SET;
  /* Set the PE2ICUSIE register             */
  REG_ICUM_CMDREG.PE2ICUSIE  = ICUM_COMM_ICH_FM_SINGLE_INTERRUPT;
}

uint32_t __attribute__ ((noinline)) HostWrite32WithoutLog(uint32_t addr, uint32_t offset, uint32_t value)
{
    uint32_t *pmem;
    uint32_t targetAddr = addr + (offset * 4);
    pmem = (uint32_t *)targetAddr;
    pmem[0] = value;
    return 0;
}

typedef enum {
    HSM_ADDR_RW_SUCCESS,
    HSM_ADDR_RW_FAILED,
} HSM_ADDR_RW_RESULT;

volatile HSM_ADDR_RW_RESULT addrRWPermissionResult     = HSM_ADDR_RW_SUCCESS;
void __attribute__ ((noinline)) hostwrite32(uint32_t addr, uint32_t offset, uint32_t value)
{
    uint32_t ret = 0;
    ret = HostWrite32WithoutLog(addr, offset, value);
    if (ret != 0) {
        addrRWPermissionResult     = HSM_ADDR_RW_FAILED;
    }
}

uint32_t __attribute__ ((noinline)) hostread32(uint32_t addr, uint32_t offset)
{
    uint32_t *pmem;
    uint32_t targetAddr = addr + (offset * 4);

    pmem = (uint32_t *)targetAddr;
    return pmem[0];
}

#define HSM_TO_HT_HEARTBEAT_MAGICWORD_PACK(num)     (0xCA0000AC | ((0x0000FFFF & (num)) << 8))
int main(int argc, FAR char *argv[])
{
  int i;
  uint32_t ret = 0;
  int result = 0;
  command = CMD_NOTHINGTODO;

  VHSM_HAL_ICUM_ICU2PES = HSM_BRING_UP_MAGIC_NUM;  //ICUM Bring up
  irq_attach(PE_TO_ICUM_INTERRUPT_NUMBER, Bridge_IRQHandler, NULL);
  up_enable_irq(PE_TO_ICUM_INTERRUPT_NUMBER);
  up_prioritize_irq(PE_TO_ICUM_INTERRUPT_NUMBER, PE_TO_ICUM_INTERRUPT_PRI);
  while(1) {
    switch (command)
    {
      case CMD_SETADDR_HOST2HSMBUF:
        for (i = 0; i < HOST_TO_HSM_BUFF_SIZE; i++) {
            hostwrite32((uint32_t)pHOST2HSMbuf, i, (0x00010000 | i));
        }
        command = CMD_NOTHINGTODO;
        HSM2HT_INIT_FINISH(0);
        break;
      case CMD_SETADDR_HSM2HOSTBUF:
        for (i = 0; i < HSM_TO_HOST_BUFF_SIZE; i++) {
            hostwrite32((uint32_t)pHSM2HOSTbuf, i, (0x00020000 | i));
        }
        command = CMD_NOTHINGTODO;
        HSM2HT_INIT_FINISH(0);
        break;
      case CMD_NOTHINGTODO:
        uiNoCommandCnt++;
        HSM2HT_HEART(HSM_TO_HT_HEARTBEAT_MAGICWORD_PACK(uiNoCommandCnt));
        break;
      default:
        break;
    }
  }
  return 0;
}
