# Boot_STM32H743_SFU_(fast_uart)
Small, fast and simple STM3HF7xx UART bootloader.
With very small footprint but without assembler code.

* **Target MCU**: STM32H743
* **clocks**: 64 MHz
* **uart**: USART1: 921600 BOD, 8bit, noParity, recive from IRQ to fifo buffer, direct send
* **Protocol**: Packets with H/W CRC32 and acknowledgment, lost and corrupt data protection
* **Can work from locked flash**
* **Firmware update ONLY**
* **fast write speed** 400kb firmware in 9 seconds
* **partital erase** and full erase
* **timeout for start main** 5 seconds

Project finished
#### Current footprint:
|text|data|bss|
|----|----|----|
|14852|116|305444|