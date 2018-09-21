#include "stm32kiss.h"
#include "hw_init.h"
#include "SFU/packet_receiver.h"
#include "SFU/sfu_commands.h"

void main()
{
	hw_init_all();

	recive_packets_init();
	sfu_command_init();

	while (1)
	{
		stat_error_timeout = 0;
		while ((stat_error_timeout * PACKET_TIMEOUT_mS) < 2000)
		{
			recive_packets_worker();
			recive_packets_print_stat();
		}
		main_start();
	}
}
