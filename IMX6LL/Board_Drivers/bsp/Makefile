CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET		  	?= bsp

CC 				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP 		:= $(CROSS_COMPILE)objdump

LIBPATH			:= -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4

INCDIRS 		:= imx6ul \
				   stdio/include \
				   bsp/clk \
				   bsp/led \
				   bsp/delay \
				   bsp/beep \
				   bsp/gpio \
				   bsp/key \
				   bsp/int \
				   bsp/exit \
				   bsp/epit_timer \
				   bsp/keyfilter \
				   bsp/uart \
				   bsp/lcd

SRCDIRS			:= project \
                   stdio/lib \
				   bsp/clk \
				   bsp/led \
				   bsp/delay \
				   bsp/beep \
				   bsp/gpio \
				   bsp/key \
				   bsp/int \
				   bsp/exit \
				   bsp/epit_timer \
				   bsp/keyfilter \
				   bsp/uart \
				   bsp/lcd


INCLUDE			:= $(patsubst %, -I %, $(INCDIRS))

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR		:= $(notdir  $(SFILES))
CFILENDIR		:= $(notdir  $(CFILES))

SOBJS			:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS			:= $(SOBJS) $(COBJS)

VPATH			:= $(SRCDIRS)

.PHONY: clean
	
$(TARGET).bin : $(OBJS)
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^ $(LIBPATH)
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2 -fno-builtin $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 -fno-builtin -Wa,-mimplicit-it=thumb $(INCLUDE) -o $@ $<
	
clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(COBJS) $(SOBJS)

# -fno-builtin选项的作用是:
# 1. 禁止GCC使用内建函数
# 2. 产生更加易于阅读和调试的代码
# 3. 确保我们自己实现的函数被调用

# -Wa,-mimplicit-it=thumb是一个GCC的汇编器选项。
# -Wa选项将参数传递给汇编器,后面的-mimplicit-it=thumb表示:
# 将汇编文件中缺失的it指令自动插入thumb模式的it指令。
# it指令是ARM汇编中的条件执行指令,用于在thumb模式下的条件执行。Thumb模式下没有condition codes,所以需要it指令来进行条件执行。
# 假设有这样一段thumb模式的汇编代码:
# asm
# cmp r0, #0
# bne label
# add r1, r1, #1
# label:
# sub r2, r2, #1
# 由于没有it指令,这段代码会无条件执行add和sub指令,而不论cmp的结果如何。
# 如果使用了-Wa,-mimplicit-it=thumb选项,GCC会自动为这段代码插入it指令,变成:
# asm
# cmp r0, #0
# it ne
# bne label
# it eq    
# add r1, r1, #1    
# label:
# it ne
# sub r2, r2, #1
# 此时,add和sub指令会根据it指令的条件进行条件执行,实现和C代码等价的功能。
# 所以,-Wa,-mimplicit-it=thumb选项的作用是:
# 在Thumb模式的汇编文件中自动插入it指令,用于实现条件执行。
# 这样,我们在编写Thumb模式的汇编代码时,可以不再显式的添加it指令,GCC会自动帮我们添加,使得汇编代码更加简洁,并且功能等价于C代码。
# 这是一个很有用的选项,可以减少Thumb模式汇编编程的难度。
