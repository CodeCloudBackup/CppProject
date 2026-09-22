#include "gait.h"
#include "cmd_protocol.h"

extern JoystickData_t g_joystick;

// 舵机ID定义
#define SERVO_LF  1  // 左前
#define SERVO_RF  2  // 右前
#define SERVO_LB  3  // 左后
#define SERVO_RB  4  // 右后

// 步态更新函数（建议在定时器中断中周期性调用，如每50ms一次）
void Update_Gait(void) {
    int8_t x = g_joystick.x_axis;
    int8_t y = g_joystick.y_axis;
    
    // 静止状态：四足回到中立位置
    if (x == 0 && y == 0) {
        Set_Servo(SERVO_LF, GAIT_BASE_ANGLE);
        Set_Servo(SERVO_RF, GAIT_BASE_ANGLE);
        Set_Servo(SERVO_LB, GAIT_BASE_ANGLE);
        Set_Servo(SERVO_RB, GAIT_BASE_ANGLE);
        return;
    }
    
    // 计算步态参数
    int8_t turn_factor = x / 10;   // 转向系数：-10~10
    int8_t speed_factor = y / 10;  // 速度系数：-10~10
    
    // 对角步态：左前+右后为一组，右前+左后为一组
    int8_t group_a = GAIT_BASE_ANGLE + speed_factor * GAIT_STEP_SIZE / 10;
    int8_t group_b = GAIT_BASE_ANGLE - speed_factor * GAIT_STEP_SIZE / 10;
    
    // 转向叠加：左转时左侧腿后退、右侧腿前进
    if (turn_factor > 0) {  // 右转
        group_a -= turn_factor * GAIT_STEP_SIZE / 20;
        group_b += turn_factor * GAIT_STEP_SIZE / 20;
    } else if (turn_factor < 0) {  // 左转
        group_a += abs(turn_factor) * GAIT_STEP_SIZE / 20;
        group_b -= abs(turn_factor) * GAIT_STEP_SIZE / 20;
    }
    
    // 边界限制（SG90安全范围：0~180°）
    group_a = constrain(group_a, 0, 180);
    group_b = constrain(group_b, 0, 180);
    
    // 输出到舵机
    Set_Servo(SERVO_LF, group_a);
    Set_Servo(SERVO_RB, group_a);  // 对角同步
    Set_Servo(SERVO_RF, group_b);
    Set_Servo(SERVO_LB, group_b);  // 对角同步
}

// 辅助函数：数值限幅
static int8_t constrain(int8_t val, int8_t min, int8_t max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}