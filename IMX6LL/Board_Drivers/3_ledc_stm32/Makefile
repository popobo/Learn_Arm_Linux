objs := start.o main.o

ledc_stm32.bin:$(objs)
	arm-linux-gnueabihf-ld -Timx6ul.lds $^ -o led_c.elf
	arm-linux-gnueabihf-objcopy -O binary -S -g led_c.elf $@
	arm-linux-gnueabihf-objdump -D led_c.elf > led_c.dis

%.o:%.s
	arm-linux-gnueabihf-gcc -Wall -Werror -nostdlib -c -o $@ $<

%.o:%.S
	arm-linux-gnueabihf-gcc -Wall -Werror -nostdlib -c -o $@ $<

%.o:%.c
	arm-linux-gnueabihf-gcc -Wall -Werror -nostdlib -c -o $@ $<

clean:
	rm -rf *.o led_c.bin led_c.elf led_c.dis
