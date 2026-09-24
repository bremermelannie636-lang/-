#ifndef _code_encoder_h_
#define _code_encoder_h_

#include "zf_common_headfile.h"

// ============================================================
// 双路里程编码器 (X / Y 两个方向)
// ------------------------------------------------------------
// 接线:
//   X 轴: A相 -> C0    B相 -> C1     (QTIMER1_ENCODER1   TIMER0/TIMER1)
//   Y 轴: A相 -> C2    B相 -> C24    (QTIMER1_ENCODER2   TIMER2/TIMER3)
// 两路都挂在 QTIMER1 上, 但用的是不同的计数通道, 互不影响。
//
// 计数方式 (encoder_dir_init -> encoder_quad_init):
//   primarySource   = A相, 上升沿计数
//   secondarySource = B相, 只决定正负号 (kQTMR_PriSrcRiseEdgeSecDir)
// 因此: A相断线 -> 该轴计数恒 0; B相断线 -> 照常计数, 但方向乱。
//
// 使用顺序 (warm-up 依赖 ms 计数器, 顺序不能反):
//   timer_init(GPT_TIM_1, TIMER_MS);  timer_start(GPT_TIM_1);
//   encoder_xy_init();
//   while(1) { encoder_xy_update(); ... }
// ============================================================

void    encoder_xy_init           (void);   // 初始化两路编码器, 丢弃上电瞬态并清零
void    encoder_xy_update         (void);   // 读一次脉冲增量, 累加到 X/Y 位移 (需周期调用)

float   encoder_xy_get_x_disp_cm  (void);   // X 轴带符号净位移 cm (前进+, 后退-)
float   encoder_xy_get_y_disp_cm  (void);   // Y 轴带符号净位移 cm

float   encoder_xy_get_x_path_cm  (void);   // X 轴累计路程 cm (绝对值, 来回都累加)
float   encoder_xy_get_y_path_cm  (void);   // Y 轴累计路程 cm

#endif
