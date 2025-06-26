#include "dc_hmi_controller.h"

// 內部函數聲明
static int set_serial_params(int fd, baud_rate_t baudrate);
static uint32_t get_baud_value(baud_rate_t baudrate);
static void build_command_frame(uint8_t *frame, uint8_t cmd, uint8_t *data, uint16_t data_len, uint16_t *frame_len);

// ============================================================================
// 基本串口操作
// ============================================================================

int hmi_init(hmi_controller_t *hmi, const char *device, baud_rate_t baudrate) {
    if (!hmi || !device) {
        return -1;
    }

    memset(hmi, 0, sizeof(hmi_controller_t));
    strncpy(hmi->device, device, sizeof(hmi->device) - 1);
    hmi->baudrate = baudrate;
    hmi->fg_color = COLOR_WHITE;
    hmi->bg_color = COLOR_BLACK;

    // 打開串口設備
    hmi->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (hmi->fd < 0) {
        printf("無法打開串口設備: %s, 錯誤: %s\n", device, strerror(errno));
        return -1;
    }

    // 設置串口參數
    if (set_serial_params(hmi->fd, baudrate) < 0) {
        close(hmi->fd);
        return -1;
    }

    // 清空輸入輸出緩衝區
    tcflush(hmi->fd, TCIOFLUSH);

    hmi->is_connected = 1;
    printf("串口屏初始化成功: %s\n", device);

    // 嘗試握手
    if (hmi_handshake(hmi) == 0) {
        printf("與串口屏握手成功\n");
    } else {
        printf("警告: 與串口屏握手失敗\n");
    }

    return 0;
}

void hmi_close(hmi_controller_t *hmi) {
    if (hmi && hmi->fd >= 0) {
        close(hmi->fd);
        hmi->fd = -1;
        hmi->is_connected = 0;
        printf("串口屏連接已關閉\n");
    }
}

static int set_serial_params(int fd, baud_rate_t baudrate) {
    struct termios options;
    
    if (tcgetattr(fd, &options) < 0) {
        printf("獲取串口參數失敗\n");
        return -1;
    }

    // 設置波特率
    uint32_t baud_value = get_baud_value(baudrate);
    cfsetispeed(&options, baud_value);
    cfsetospeed(&options, baud_value);

    // 設置數據位、停止位、奇偶校驗
    options.c_cflag &= ~PARENB;   // 無奇偶校驗
    options.c_cflag &= ~CSTOPB;   // 1位停止位
    options.c_cflag &= ~CSIZE;    // 清除數據位設置
    options.c_cflag |= CS8;       // 8位數據位
    options.c_cflag |= CREAD | CLOCAL; // 使能接收和本地連接

    // 設置輸入模式
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用軟件流控
    options.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 原始輸入

    // 設置輸出模式
    options.c_oflag &= ~OPOST; // 原始輸出

    // 設置超時
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10; // 1秒超時

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        printf("設置串口參數失敗\n");
        return -1;
    }

    return 0;
}

static uint32_t get_baud_value(baud_rate_t baudrate) {
    switch (baudrate) {
        case BAUD_1200: return B1200;
        case BAUD_2400: return B2400;
        case BAUD_4800: return B4800;
        case BAUD_9600: return B9600;
        case BAUD_19200: return B19200;
        case BAUD_38400: return B38400;
        case BAUD_57600: return B57600;
        case BAUD_115200: return B115200;
        default: return B9600;
    }
}

int hmi_send_command(hmi_controller_t *hmi, uint8_t *cmd, uint16_t length) {
    if (!hmi || !cmd || !hmi->is_connected) {
        return -1;
    }

    int bytes_written = write(hmi->fd, cmd, length);
    if (bytes_written != length) {
        printf("發送指令失敗, 期望: %d, 實際: %d\n", length, bytes_written);
        return -1;
    }

    // 確保數據發送完成
    tcdrain(hmi->fd);
    return 0;
}

int hmi_send_data(hmi_controller_t *hmi, uint8_t cmd, uint8_t *data, uint16_t length) {
    uint8_t frame[1024];
    uint16_t frame_len;

    build_command_frame(frame, cmd, data, length, &frame_len);
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_receive_response(hmi_controller_t *hmi, hmi_response_t *response, int timeout_ms) {
    if (!hmi || !response || !hmi->is_connected) {
        return -1;
    }

    uint8_t buffer[1024];
    int bytes_read = 0;
    time_t start_time = time(NULL);

    while ((time(NULL) - start_time) * 1000 < timeout_ms) {
        int n = read(hmi->fd, buffer + bytes_read, sizeof(buffer) - bytes_read);
        if (n > 0) {
            bytes_read += n;
            
            // 檢查是否收到完整幀
            if (bytes_read >= 6) { // 最小幀長度
                if (buffer[0] == FRAME_HEADER && 
                    buffer[bytes_read-4] == 0xFF && buffer[bytes_read-3] == 0xFC &&
                    buffer[bytes_read-2] == 0xFF && buffer[bytes_read-1] == 0xFF) {
                    
                    response->cmd = buffer[1];
                    response->length = bytes_read - 6; // 除去幀頭和幀尾
                    if (response->length > 0) {
                        response->data = malloc(response->length);
                        memcpy(response->data, buffer + 2, response->length);
                    } else {
                        response->data = NULL;
                    }
                    return 0;
                }
            }
        }
        usleep(1000); // 1ms延遲
    }

    return -1; // 超時
}

static void build_command_frame(uint8_t *frame, uint8_t cmd, uint8_t *data, uint16_t data_len, uint16_t *frame_len) {
    uint8_t tail[] = FRAME_TAIL;
    
    frame[0] = FRAME_HEADER;
    frame[1] = cmd;
    
    uint16_t pos = 2;
    if (data && data_len > 0) {
        memcpy(frame + pos, data, data_len);
        pos += data_len;
    }
    
    memcpy(frame + pos, tail, FRAME_TAIL_SIZE);
    pos += FRAME_TAIL_SIZE;
    
    *frame_len = pos;
}

// 這個函數已被直接的幀構建方式替代，但保留以備將來使用
#if 0
static void build_config_frame(uint8_t *frame, uint8_t sub_cmd, uint8_t *data, uint16_t data_len, uint16_t *frame_len) {
    uint8_t tail[] = FRAME_TAIL;
    
    frame[0] = FRAME_HEADER;
    frame[1] = CMD_CONFIG_BASE;
    frame[2] = sub_cmd;
    
    uint16_t pos = 3;
    if (data && data_len > 0) {
        memcpy(frame + pos, data, data_len);
        pos += data_len;
    }
    
    memcpy(frame + pos, tail, FRAME_TAIL_SIZE);
    pos += FRAME_TAIL_SIZE;
    
    *frame_len = pos;
}
#endif

// ============================================================================
// 基本功能
// ============================================================================

int hmi_handshake(hmi_controller_t *hmi) {
    uint8_t frame[16];
    uint16_t frame_len;
    
    build_command_frame(frame, CMD_HANDSHAKE, NULL, 0, &frame_len);
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.cmd == 0x55) {
            if (response.data) free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

int hmi_reset_device(hmi_controller_t *hmi) {
    uint8_t data[] = {0x35, 0x5A, 0x53, 0xA5};
    return hmi_send_data(hmi, CMD_RESET_DEVICE, data, sizeof(data));
}

int hmi_get_version(hmi_controller_t *hmi, char *version) {
    uint8_t data[] = {0x01};
    uint8_t frame[16];
    uint16_t frame_len;
    
    build_command_frame(frame, CMD_GET_VERSION, data, sizeof(data), &frame_len);
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.cmd == CMD_GET_VERSION && response.data && response.length >= 6) {
            sprintf(version, "%d.%d.%d.%d", 
                    response.data[0], response.data[1],
                    (response.data[2] << 8) | response.data[3],
                    (response.data[4] << 8) | response.data[5]);
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

int hmi_clean_screen(hmi_controller_t *hmi) {
    return hmi_send_data(hmi, CMD_CLEAN_SCREEN, NULL, 0);
}

int hmi_set_backlight(hmi_controller_t *hmi, uint8_t level) {
    return hmi_send_data(hmi, CMD_SET_BACKLIGHT, &level, 1);
}

int hmi_set_buzzer(hmi_controller_t *hmi, uint8_t time_10ms) {
    return hmi_send_data(hmi, CMD_BUZZER_CONTROL, &time_10ms, 1);
}

int hmi_set_baudrate(hmi_controller_t *hmi, baud_rate_t baudrate) {
    uint8_t baud = (uint8_t)baudrate;
    return hmi_send_data(hmi, CMD_SET_BAUDRATE, &baud, 1);
}

// ============================================================================
// 顏色設置
// ============================================================================

int hmi_set_fg_color(hmi_controller_t *hmi, uint16_t color) {
    hmi->fg_color = color;
    uint8_t data[2] = {color >> 8, color & 0xFF};
    return hmi_send_data(hmi, CMD_SET_FCOLOR, data, 2);
}

int hmi_set_bg_color(hmi_controller_t *hmi, uint16_t color) {
    hmi->bg_color = color;
    uint8_t data[2] = {color >> 8, color & 0xFF};
    return hmi_send_data(hmi, CMD_SET_BCOLOR, data, 2);
}

int hmi_set_colors(hmi_controller_t *hmi, uint16_t fg_color, uint16_t bg_color) {
    hmi->fg_color = fg_color;
    hmi->bg_color = bg_color;
    uint8_t data[4] = {
        fg_color >> 8, fg_color & 0xFF,
        bg_color >> 8, bg_color & 0xFF
    };
    return hmi_send_data(hmi, CMD_SET_FB_COLOR, data, 4);
}

// ============================================================================
// 實用函數
// ============================================================================

uint16_t hmi_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void hmi_delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void hmi_print_buffer(uint8_t *buffer, uint16_t length) {
    printf("Buffer [%d bytes]: ", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (length % 16 != 0) {
        printf("\n");
    }
} 