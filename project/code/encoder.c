#include "zf_common_headfile.h"
#include "encoder.h"

// ============================================================
// 编码器 / 里程轮参数
// ============================================================
#define ENCODER_X_INDEX     QTIMER1_ENCODER1             // C0/C1  -> QTIMER1_ENCODER1
#define ENCODER_X_CH1_PIN   QTIMER1_ENCODER1_CH1_C0      // A相 (脉冲) -> C0
#define ENCODER_X_CH2_PIN   QTIMER1_ENCODER1_CH2_C1      // B相 (方向) -> C1

#define ENCODER_Y_INDEX     QTIMER1_ENCODER2             // C2/C24 -> QTIMER1_ENCODER2
#define ENCODER_Y_CH1_PIN   QTIMER1_ENCODER2_CH1_C2      // A相 (脉冲) -> C2
#define ENCODER_Y_CH2_PIN   QTIMER1_ENCODER2_CH2_C24     // B相 (方向) -> C24

#define ENCODER_PI          3.1415926536f                        // 圆周率
#define WHEEL_DIAMETER_CM   4.8f                                 // 里程轮直径 cm (B车 4.8, 待定)
#define ENCODER_CPR         1024                                 // 编码器每转脉冲数
#define PULSE_TO_CM         (ENCODER_PI * WHEEL_DIAMETER_CM / ENCODER_CPR)   // 单脉冲对应距离 cm

#define X_DIR_SIGN          1                            // X 轴方向修正 (1=正向, -1=反向)
#define Y_DIR_SIGN          1                            // Y 轴方向修正 (1=正向, -1=反向)
#define SCALE               1.0f                         // 里程刻度缩放系数 (1.0=标准, 实测可微调)

#define ENCODER_WARMUP_MS   200                          // 上电丢弃窗口 (ms)

// ============================================================
// 里程计状态
// ============================================================
static float x_total_cm = 0.0f;                          // X 轴带符号净位移 (前进+, 后退-)
static float y_total_cm = 0.0f;                          // Y 轴带符号净位移 (前进+, 后退-)
static float x_path_cm  = 0.0f;                          // X 轴累计路程 (取绝对值, 来回都累加)
static float y_path_cm  = 0.0f;                          // Y 轴累计路程 (取绝对值, 来回都累加)

//-------------------------------------------------------------------------------------------------------------------
// 读一路编码器的脉冲增量, 换算成带符号位移 (cm)
//-------------------------------------------------------------------------------------------------------------------
static float encoder_read_delta_cm (encoder_index_enum index, int8 dir_sign)
{
    int16 pulses = encoder_get_count(index);             // 读一次
    encoder_clear_count(index);                          // 立刻清零, 下次读的就是增量

    return (float)(dir_sign * pulses) * PULSE_TO_CM * SCALE;
}

//-------------------------------------------------------------------------------------------------------------------
// 初始化 X / Y 两路编码器
//-------------------------------------------------------------------------------------------------------------------
void encoder_xy_init (void)
{
    encoder_dir_init(ENCODER_X_INDEX, ENCODER_X_CH1_PIN, ENCODER_X_CH2_PIN);     // C0/C1
    encoder_dir_init(ENCODER_Y_INDEX, ENCODER_Y_CH1_PIN, ENCODER_Y_CH2_PIN);     // C2/C24

    // ★ 两路都初始化完再一起清零。
    //   encoder_quad_init 内部是 Init -> Deinit -> Init, 其中 Deinit 会关掉 QTIMER1
    //   整个模块的时钟, 先初始化的那一路在这段空窗里是停表的。清零放在最后, 就不会
    //   把这几个虚假/丢失的脉冲算进里程。
    encoder_clear_count(ENCODER_X_INDEX);
    encoder_clear_count(ENCODER_Y_INDEX);

    // ★ 上电瞬态丢弃: 此刻编码器供电可能还没稳, 引脚又只有 keeper 没有上拉,
    //   A相会冒出几个虚假边沿。头 ENCODER_WARMUP_MS 内只清不累加, 全部扔掉。
    //   注意这里必须"一边等一边清", 不能换成 system_delay_ms 阻塞等待 —— 空窗里的
    //   虚假边沿要靠这几行一直清掉。
    //   依赖 GPT_TIM_1 已由 main 启动 (不能在 timer_start 之前调用)。
    uint32 t0 = timer_get(GPT_TIM_1);
    while((uint32)(timer_get(GPT_TIM_1) - t0) < ENCODER_WARMUP_MS)
    {
        encoder_clear_count(ENCODER_X_INDEX);
        encoder_clear_count(ENCODER_Y_INDEX);
    }

    x_total_cm = 0.0f;
    y_total_cm = 0.0f;
    x_path_cm  = 0.0f;
    y_path_cm  = 0.0f;
}

//-------------------------------------------------------------------------------------------------------------------
// 里程更新: 分别读 X / Y 的脉冲增量
// disp 保号累加 (净位移), path 取绝对值累加 (路程, 来回都累加)
//-------------------------------------------------------------------------------------------------------------------
void encoder_xy_update (void)
{
    float dx = encoder_read_delta_cm(ENCODER_X_INDEX, X_DIR_SIGN);
    float dy = encoder_read_delta_cm(ENCODER_Y_INDEX, Y_DIR_SIGN);

    x_total_cm += dx;
    y_total_cm += dy;
    x_path_cm  += (dx < 0.0f) ? -dx : dx;
    y_path_cm  += (dy < 0.0f) ? -dy : dy;
}

//-------------------------------------------------------------------------------------------------------------------
// 取 X / Y 轴带符号净位移 (cm)
//-------------------------------------------------------------------------------------------------------------------
float encoder_xy_get_x_disp_cm (void)
{
    return x_total_cm;
}

float encoder_xy_get_y_disp_cm (void)
{
    return y_total_cm;
}

//-------------------------------------------------------------------------------------------------------------------
// 取 X / Y 轴累计路程 (cm, 绝对值, 来回都累加) —— 标定轮径/CPR 用这个
//-------------------------------------------------------------------------------------------------------------------
float encoder_xy_get_x_path_cm (void)
{
    return x_path_cm;
}

float encoder_xy_get_y_path_cm (void)
{
    return y_path_cm;
}
