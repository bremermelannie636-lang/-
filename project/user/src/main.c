#include "zf_common_headfile.h"
#include "encoder.h"

// ============================================================
// 双轴里程计测试
// ------------------------------------------------------------
// 编码器接线 (详见 code/encoder.h):
//   X 轴: A相 -> C0    B相 -> C1     (QTIMER1_ENCODER1)
//   Y 轴: A相 -> C2    B相 -> C24    (QTIMER1_ENCODER2)
//
// 输出字段 (无线串口 + 调试串口, 100ms 一次):
//   x / y         = X / Y 轴带符号净位移 cm (前进+, 后退-)
//   xpath / ypath = X / Y 轴累计路程 cm (绝对值, 来回都累加; 标定轮径/CPR 看这个)
//   (轮径/CPR/方向修正都在 code/encoder.c 顶部配置)
// ============================================================

#define PRINT_PERIOD_MS     100                          // 打印周期 (ms)

int main(void)
{
    interrupt_global_disable();
    clock_init(SYSTEM_CLOCK_600M);
    debug_init();
    wireless_uart_init();               // wireless uart UART8 (TX=D16 RX=D17 RTS=D26)

    timer_init(GPT_TIM_1, TIMER_MS);    // 先启动 ms 计数器, encoder_xy_init 的 warm-up 要用
    timer_start(GPT_TIM_1);

    encoder_xy_init();                  // X: C0/C1, Y: C2/C24

    uint32 last_print = 0;
    while(1)
    {
        encoder_xy_update();

        uint32 now = timer_get(GPT_TIM_1);
        if(now - last_print >= PRINT_PERIOD_MS)
        {
            last_print = now;
            char buf[96];
            sprintf(buf, "x=%.2f  y=%.2f cm   xpath=%.2f  ypath=%.2f cm\r\n",
                    encoder_xy_get_x_disp_cm(), encoder_xy_get_y_disp_cm(),
                    encoder_xy_get_x_path_cm(), encoder_xy_get_y_path_cm());
            printf("%s", buf);                   // debug uart (UART1, USB)
            wireless_uart_send_string(buf);      // wireless uart (UART8)
        }
    }
}
