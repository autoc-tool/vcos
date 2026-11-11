void tcpip_main_function(void)
{
}

void ethif_mainfunction_state(void)
{
}

unsigned short crc_calculate_16(const unsigned char *data, unsigned int len, unsigned short start, unsigned char first)
{
	(void)data;
	(void)len;
	(void)start;
	(void)first;
	return 0;
}

unsigned int crc_calculate_32(const unsigned char *data, unsigned int len, unsigned int start, unsigned char first)
{
	(void)data;
	(void)len;
	(void)start;
	(void)first;
	return 0;
}

void ea_set_mode(unsigned char mode)
{
	(void)mode;
}

unsigned char ea_read_block(unsigned short block, unsigned short offset, unsigned char *data, unsigned short length)
{
	(void)block;
	(void)offset;
	(void)data;
	(void)length;
	return 1;
}

unsigned char ea_write_block(unsigned short block, const unsigned char *data)
{
	(void)block;
	(void)data;
	return 1;
}

void ea_cancel(void)
{
}

unsigned char ea_get_status(void)
{
	return 0;
}

unsigned char ea_get_job_result(void)
{
	return 0;
}

unsigned char ea_invalidate_block(unsigned short block)
{
	(void)block;
	return 1;
}

unsigned char ea_erase_immediate_block(unsigned short block)
{
	(void)block;
	return 1;
}

void eth0_handle(void *arg)
{
	(void)arg;
}

void up_uart_rx_handler(void *arg)
{
	(void)arg;
}

void up_ipicall_isr_handler(void *arg)
{
	(void)arg;
}

void up_systick_isr_handler(void);

void rt_fwk_tick_handler(void *arg)
{
	(void)arg;
	up_systick_isr_handler();
}
