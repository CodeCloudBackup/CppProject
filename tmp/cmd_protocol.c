#include "cmd_protocol.h"


/* ================= 内部状态机枚举 (保持不变) ================= */
typedef enum {
    STATE_WAIT_HEAD1 = 0,
    STATE_WAIT_HEAD2,
    STATE_WAIT_CMD,
    STATE_WAIT_LEN,
    STATE_WAIT_DATA,
    STATE_WAIT_CHECKSUM,
    STATE_WAIT_TAIL1,
    STATE_WAIT_TAIL2
} ParseState_t;

/* ================= 内部全局变量 ================= */
static ParseState_t g_state = STATE_WAIT_HEAD1;
static ProtocolFrame_t g_frame;
static uint8_t g_data_index = 0;
static CmdCallback_t g_callback = NULL;

/* ================= 内部辅助函数 ================= */
// 计算校验和（累加和取低8位）
static uint8_t CalcChecksum(uint8_t *buf, uint8_t len) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(sum & 0xFF);
}

// 重置状态机
static void ResetParser(void) {
    g_state = STATE_WAIT_HEAD1;
    g_data_index = 0;
    memset(&g_frame, 0, sizeof(ProtocolFrame_t));
}

// 内部通用发送函数：组包并通过硬件发送
static void Protocol_SendFrame(uint8_t cmd, uint8_t *data, uint8_t len) {
    uint8_t tx_buf[FRAME_MAX_LEN];
    uint8_t index = 0;
    
    // 1. 帧头
    tx_buf[index++] = PROTOCOL_HEAD_1;
    tx_buf[index++] = PROTOCOL_HEAD_2;
    // 2. 功能码
    tx_buf[index++] = (uint8_t)cmd;
    // 3. 数据长度
    tx_buf[index++] = len;
    // 4. 数据区
    if (len > 0 && data != NULL) {
        memcpy(&tx_buf[index], data, len);
        index += len;
    }
    // 5. 校验和 (帧头+功能码+长度+数据)
    tx_buf[index] = CalcChecksum(tx_buf, index); 
    index++;
    // 6. 帧尾
    tx_buf[index++] = PROTOCOL_TAIL_1;
    tx_buf[index++] = PROTOCOL_TAIL_2;
    
    // 【TODO】替换为你的实际硬件发送接口，例如 ESP8266 的 TCP 发送或 UART 发送
    // ESP8266_SendData(tx_buf, index); 
}

// 处理解析完成的指令帧
static void ProcessFrame(void) {
    // 1. 校验数据长度
    if (g_frame.len > MAX_DATA_LEN) {
        // 长度异常，可以选择上报错误，这里直接丢弃
        ResetParser();
        return;
    }
    
    // 2. 校验和验证
    uint8_t verify_buf[2 + 1 + 1 + MAX_DATA_LEN];
    verify_buf[0] = g_frame.head[0];
    verify_buf[1] = g_frame.head[1];
    verify_buf[2] = g_frame.cmd;
    verify_buf[3] = g_frame.len;
    memcpy(&verify_buf[4], g_frame.data, g_frame.len);
    uint8_t real_sum = CalcChecksum(verify_buf, 4 + g_frame.len);

    if (real_sum != g_frame.checksum) {
        // 校验失败，上报校验和错误
        Protocol_SendError(ERR_CHECKSUM);
        ResetParser();
        return;
    }
    
    // 3. 调用用户回调处理业务逻辑
    if (g_callback != NULL) {
        g_callback((ProtocolCmd_t)g_frame.cmd, g_frame.data, g_frame.len);
    }
    ResetParser();
}

/* ================= 对外接口实现 ================= */

// 初始化协议栈
void Protocol_Init(CmdCallback_t callback) {
    g_callback = callback;
    ResetParser();
}

// 核心解析函数：逐字节解析 TCP 数据流
void Protocol_InputByte(uint8_t byte) {
    switch (g_state) {
        case STATE_WAIT_HEAD1:
            if (byte == PROTOCOL_HEAD_1) {
                g_frame.head[0] = byte;
                g_state = STATE_WAIT_HEAD2;
            }
            break;
        case STATE_WAIT_HEAD2:
            if (byte == PROTOCOL_HEAD_2) {
                g_frame.head[1] = byte;
                g_state = STATE_WAIT_CMD;
            } else {
                ResetParser();
                Protocol_InputByte(byte); // 递归检查当前字节
            }
            break;
        case STATE_WAIT_CMD:
            g_frame.cmd = byte;
            g_state = STATE_WAIT_LEN;
            break;
        case STATE_WAIT_LEN:
            if (byte <= MAX_DATA_LEN) {
                g_frame.len = byte;
                g_data_index = 0;
                g_state = (byte > 0) ? STATE_WAIT_DATA : STATE_WAIT_CHECKSUM;
            } else {
                Protocol_SendError(ERR_DATA_LEN); // 上报长度异常错误
                ResetParser();
            }
            break;
        case STATE_WAIT_DATA:
            g_frame.data[g_data_index++] = byte;
            if (g_data_index >= g_frame.len) {
                g_state = STATE_WAIT_CHECKSUM;
            }
            break;
        case STATE_WAIT_CHECKSUM:
            g_frame.checksum = byte;
            g_state = STATE_WAIT_TAIL1;
            break;
        case STATE_WAIT_TAIL1:
            if (byte == PROTOCOL_TAIL_1) {
                g_state = STATE_WAIT_TAIL2;
            } else {
                ResetParser();
            }
            break;
        case STATE_WAIT_TAIL2:
            if (byte == PROTOCOL_TAIL_2) {
                ProcessFrame();
            } else {
                ResetParser();
            }
            break;
        default:
            ResetParser();
            break;
    }
}

// 发送完整状态上报帧
void Protocol_SendStatus(DogStatus_t *status) {
    uint8_t data_buf[8]; // 4腿 + 1头 + 1表情 + 1电量 + 1错误码 = 8字节
    data_buf[0] = status->servo_legs[0];
    data_buf[1] = status->servo_legs[1];
    data_buf[2] = status->servo_legs[2];
    data_buf[3] = status->servo_legs[3];
    data_buf[4] = status->servo_head;
    data_buf[5] = status->face_id;
    data_buf[6] = status->battery;
    data_buf[7] = (uint8_t)status->error_code; // 将当前错误码带上
    
    Protocol_SendFrame(CMD_STATUS_REPORT, data_buf, 8);
}

// 单独发送错误码上报帧
void Protocol_SendError(ProtocolError_t err_code) {
    uint8_t data_buf[1];
    data_buf[0] = (uint8_t)err_code;
    Protocol_SendFrame(CMD_ERROR_REPORT, data_buf, 1);
}