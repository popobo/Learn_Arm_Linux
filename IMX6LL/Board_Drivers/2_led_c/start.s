.global _start

_start:

    /* 进入SVC模式 */
    mrs r0, cpsr
    bic r0, r0, #0x1f /* 将r0的低5位清0，也就是cpsr的M0~M4 */
    orr r0, r0, #0x13 /* r0或上0x13，表示使用SVC模式 */

    msr cpsr, r0 /* 将r0的数据写入到cpsr中 */
    
    ldr sp, =0x80200000 /* 设置栈指针 */
    b main
    