#include "cmd_protocol.h"

DogStatus_t my_dog_status;

// 指令处理回调
void MyCmdHandler(ProtocolCmd_t cmd, uint8_t *data, uint8_t len) {
    switch (cmd) {
        case CMD_SERVO_SINGLE: {
            if (len < 2) {
                Protocol_SendError(ERR_DATA_LEN);
                return;
            }
            uint8_t id = data[0];
            uint8_t angle = data[1];
            
            // 边界检查：舵机角度越界
            if (angle > 180) {
                Protocol_SendError(ERR_SERVO_ANGLE);
                return;
            }
            // Set_Servo(id, angle); 
            my_dog_status.error_code = ERR_NONE; // 操作成功，清除错误码
            break;
        }
        case CMD_FACE_SWITCH: {
            if (data[0] < 1 || data[0] > 4) {
                 Protocol_SendError(ERR_UNKNOWN_CMD); // 表情ID无效
                 return;
            }
            // OLED_Show_Face(data[0]);
            my_dog_status.face_id = data[0];
            break;
        }
        case CMD_HEARTBEAT: {
            // 收到心跳，回复当前完整状态（包含错误码）
            Protocol_SendStatus(&my_dog_status);
            break;
        }
        case CMD_JOYSTICK_CTRL: {
            if (len < 2) {
                Protocol_SendError(ERR_DATA_LEN);
                return;
            }
            // 解析摇杆数据（有符号数）
            g_joystick.x_axis = (int8_t)data[0];
            g_joystick.y_axis = (int8_t)data[1];
            
            // 死区处理：摇杆回中时微小偏移忽略
            if (abs(g_joystick.x_axis) < 5) g_joystick.x_axis = 0;
            if (abs(g_joystick.y_axis) < 5) g_joystick.y_axis = 0;
            
            // 触发步态更新
            Update_Gait();
            break;
        }
        
        case CMD_SLIDER_CTRL: {
            if (len < 1) {
                Protocol_SendError(ERR_DATA_LEN);
                return;
            }
            g_head_angle = data[0];
            // 边界检查
            if (g_head_angle > 180) {
                Protocol_SendError(ERR_SERVO_ANGLE);
                return;
            }
            // 直接控制头部舵机
            Set_Servo(5, g_head_angle);  // ID5为头部舵机
            my_dog_status.servo_head = g_head_angle;
            break;
        }
        default:
            // 收到未知功能码，主动上报
            Protocol_SendError(ERR_UNKNOWN_CMD);
            break;
    }
}

int main(void) {
    // 硬件初始化...
    Protocol_Init(MyCmdHandler);
    
    // 初始化状态
    memset(&my_dog_status, 0, sizeof(DogStatus_t));
    my_dog_status.error_code = ERR_NONE;
    my_dog_status.battery = 85; 
    
    while (1) {
        // 模拟从 ESP8266 TCP 接收数据
        // uint8_t recv_byte = ESP8266_GetByte();
        // Protocol_InputByte(recv_byte);
        
        // 模拟低电量检测
        if (my_dog_status.battery < 20) {
            my_dog_status.error_code = ERR_LOW_BATTERY;
            // 可以主动上报一次错误
            // Protocol_SendError(ERR_LOW_BATTERY);
        }
    }
}