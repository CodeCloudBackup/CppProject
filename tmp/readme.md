
现在需要设计界面操作和蓝牙指令发送的逻辑，
1、四足复位和机器重启按钮，分别占两个功能码
2、摇杆发送坐标数据，占一个功能码
3、滑杆发送角度数据，占一个功能码
4、12个按键，占12个功能码，
指令的数据结构如下：
typedef struct {
    uint8_t head[2];
    uint8_t cmd;
    uint8_t len;
    uint8_t data[MAX_DATA_LEN];
    uint8_t checksum;
    uint8_t tail[2];
} ProtocolFrame_t;