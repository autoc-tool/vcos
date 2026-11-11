#include <nuttx/config.h>

#include <stdint.h>

/* Provided by the SEGGER RTT library selected with CONFIG_SERIAL_RTT. */

unsigned SEGGER_RTT_Write(unsigned buffer_index, const void *buffer,
                          unsigned length);

void up_putc(int ch)
{
  char byte = (char)ch;

  SEGGER_RTT_Write(0, &byte, 1);
}

void arm_earlyserialinit(void)
{
}

void arm_serialinit(void)
{
}
