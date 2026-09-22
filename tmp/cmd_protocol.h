#ifndef CMD_PROTOCOL_H
#define CMD_PROTOCOL_H

#include <stdint.h>
#include <string.h>

#define PROTOCOL_HEAD_1 0xAA
#define PROTOCOL_HEAD_2 0x55
#define PROTOCOL_TAIL_1 0x0D
#define PROTOCOL_TAIL_2 0x0A

#define MAX_DATA_LEN 32
#define FREAME_MAX_LEN (2+1+1+MAX_DATA_LEN+1+2)


/* ================= 枚举：功能码定义 ================= */
typedef enum {
    // App -> 机器狗 (控制指令)
    CMD_SERVO_SINGLE    = 0x01,   // 单舵机控制
    CMD_SERVO_FOUR      = 0x02,   // 四腿联动
    CMD_HEAD_CTRL       = 0x03,   // 头部舵机控制
    CMD_FACE_SWITCH     = 0x04,   // 表情切换
    CMD_ACTION_CTRL     = 0x05,   // 预设动作
    CMD_HEARTBEAT       = 0x06,   // 心跳包
    CMD_JOYSTICK_CTRL   = 0x07,   // 摇杆控制（XY轴）
    CMD_SLIDER_CTRL     = 0x08,   // 滑块控制（摇头）
    // 机器狗 -> App (上报指令)
    CMD_STATUS_REPORT   = 0x81,   // 状态上报
    CMD_ERROR_REPORT    = 0x82    // 错误码上报
} ProtocolCmd_t;

/* ================= 枚举：错误码定义 ================= */
typedef enum {
    ERR_NONE            = 0x00,   // 无错误/操作成功
    ERR_UNKNOWN_CMD     = 0x01,   // 未知指令/不支持的功能码
    ERR_DATA_LEN        = 0x02,   // 数据长度异常
    ERR_CHECKSUM        = 0x03,   // 校验和错误
    ERR_SERVO_ANGLE     = 0x04,   // 舵机角度越界 (如 >180)
    ERR_SERVO_ID        = 0x05,   // 舵机ID不存在
    ERR_LOW_BATTERY     = 0x06,   // 低电量报警
    ERR_ACTION_BUSY     = 0x07    // 正在执行动作，无法接收新指令
} ProtocolError_t;

/* ================= 数据结构：摇杆数据 ================= */
typedef struct {
    int8_t x_axis;    // X轴：-100(左) ~ 0(中) ~ 100(右)
    int8_t y_axis;    // Y轴：-100(后退) ~ 0(中) ~ 100(前进)
} JoystickData_t;
/* ================= 枚举：动作编号定义 ================= */
typedef enum {
    ACTION_STOP         = 0,      // 停止
    ACTION_STAND        = 1,      // 站立
    ACTION_SIT          = 2,      // 坐下/趴下
    ACTION_SHAKE        = 3,      // 握手
    ACTION_BOW          = 4,      // 低头
    ACTION_SWAY         = 5       // 摇摆
} ActionId_t;

/* ================= 数据结构定义 ================= */
typedef struct {
    uint8_t head[2];
    uint8_t cmd;
    uint8_t len;
    uint8_t data[MAX_DATA_LEN];
    uint8_t checksum;
    uint8_t tail[2];
} ProtocolFrame_t;

/* ================= 回调函数类型定义 ================= */
typedef void (*CmdCallback_t)(ProtocolCmd_t cmd, uint8_t *data, uint8_t len);

/* ================= 对外接口 ================= */
void Protocol_Init(CmdCallback_t callback);
void Protocol_InputByte(uint8_t byte);
void Protocol_SendStatus(DogStatus_t *status);
void Protocol_SendError(ProtocolError_t err_code);

#endif
