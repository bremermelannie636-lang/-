#include "zf_common_headfile.h"

// ★ 串口中断里必须把收到的数据"读走": LPUART 的 RDRF 标志只有读 DATA 寄存器才会自动清。
//   只清 overrun 而不读 DATA, 一旦真有字节进来, RDRF 一直置位 -> 中断反复进入 -> 主循环卡死。
//   所以本工程在用的两个串口都要调用库给的接收回调:
//     UART1 (调试串口 B12/B13)  -> debug_interrupr_handler()
//     UART8 (无线串口 D16/D17)  -> wireless_module_uart_handler()  即 wireless_uart_callback

void PIT_IRQHandler(void){if(pit_flag_get(PIT_CH0))pit_flag_clear(PIT_CH0);if(pit_flag_get(PIT_CH1))pit_flag_clear(PIT_CH1);}
void LPUART1_IRQHandler(void){if(LPUART_GetStatusFlags(LPUART1) & kLPUART_RxDataRegFullFlag)debug_interrupr_handler();LPUART_ClearStatusFlags(LPUART1,kLPUART_RxOverrunFlag);}
void LPUART2_IRQHandler(void){LPUART_ClearStatusFlags(LPUART2,kLPUART_RxOverrunFlag);}
void LPUART3_IRQHandler(void){LPUART_ClearStatusFlags(LPUART3,kLPUART_RxOverrunFlag);}
void LPUART4_IRQHandler(void){LPUART_ClearStatusFlags(LPUART4,kLPUART_RxOverrunFlag);}
void LPUART5_IRQHandler(void){LPUART_ClearStatusFlags(LPUART5,kLPUART_RxOverrunFlag);}
void LPUART6_IRQHandler(void){LPUART_ClearStatusFlags(LPUART6,kLPUART_RxOverrunFlag);}
void LPUART8_IRQHandler(void){if(LPUART_GetStatusFlags(LPUART8) & kLPUART_RxDataRegFullFlag)wireless_module_uart_handler();LPUART_ClearStatusFlags(LPUART8,kLPUART_RxOverrunFlag);}
