- 内核模块

    每个内核模块在内核地址空间中只存在一份。当一个内核模块被加载时，它的代码和数据会被映射到内核地址空间的合适位置，并分配相应的内存空间。此后，所有对该模块的访问都是在内核地址空间中进行的。

    由于内核地址空间是所有内核线程和进程共享的，因此在内核地址空间中，每个模块只有一份，所有对该模块的访问都是共享的。
    
    当 gpioled.ko 模块被加载到内核中后，所有使用该模块的进程操作的 gpioled 变量都是同一个，因为这些进程都是在内核中运行的，它们共享内核中的数据结构和变量。

- unsigned long spin_lock_irqsave(spinlock_t *lock, unsigned long flags);
```c
    // spin_lock_irqsave展开后最终调用到，所以flags不必传递指针

    #define raw_local_irq_save(flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		flags = arch_local_irq_save();		\
	} while (0)
```
- int down_interruptible(struct semaphore *sem);

    在获取信号量时，如果信号量的值大于 0，down_interruptible 将减少信号量的值并返回 0。如果信号量的值等于 0，down_interruptible 将线程加入信号量的等待队列，并等待信号量的值变为非 0。如果在等待信号量的过程中接收到一个中断信号，down_interruptible 将返回一个小于 0 的值，并且线程将被唤醒。

- wait_queue_head_t and wait_queue_t

    wait_queue_head_t represents a wait queu. It contains a list of wait_queue_t structures, which represents the individual processes that are waiting on the queue.

- bootargs
    setenv bootargs 'console=tty1 console=ttymxc0,115200 root=/dev/nfs nfsroot=192.168.5.11:/home/bo/linux/nfs/rootfs,proto=tcp rw ip=192.168.5.9:192.168.5.11:192.168.5.1:255.255.255.0::eth0:off'