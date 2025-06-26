#ifndef DC_HMI_CONTROLLER_H
#define DC_HMI_CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>

// 指令幀格式
#define FRAME_HEADER    0xEE
#define FRAME_TAIL      {0xFF, 0xFC, 0xFF, 0xFF}
#define FRAME_TAIL_SIZE 4

// 基本指令碼
#define CMD_CLEAN_SCREEN        0x01
#define CMD_HANDSHAKE          0x04
#define CMD_RESET_DEVICE       0x07
#define CMD_GET_VERSION        0xFE
#define CMD_SET_BACKLIGHT      0x60
#define CMD_AUTO_SLEEP         0x77
#define CMD_BUZZER_CONTROL     0x61
#define CMD_TOUCH_CONFIG       0x70
#define CMD_TOUCH_CALIBRATE    0x72
#define CMD_TOUCH_TEST         0x73
#define CMD_SET_BAUDRATE       0xA0
#define CMD_SET_FCOLOR         0x41
#define CMD_SET_BCOLOR         0x42
#define CMD_SET_FB_COLOR       0x40

// 組態指令碼
#define CMD_CONFIG_BASE        0xB1
#define CMD_SWITCH_SCREEN      0x00
#define CMD_READ_SCREEN        0x01
#define CMD_SET_CURSOR         0x02
#define CMD_HIDE_CONTROL       0x03
#define CMD_DISABLE_CONTROL    0x04
#define CMD_ANIM_SWITCH        0x05
#define CMD_FORMAT_TEXT        0x07
#define CMD_UPDATE_CONTROL     0x10
#define CMD_READ_CONTROL       0x11
#define CMD_BATCH_UPDATE       0x12
#define CMD_SET_DROPDOWN       0x13
#define CMD_DROPDOWN_UPLOAD    0x14
#define CMD_SET_BLINK          0x15
#define CMD_SET_SCROLL         0x16
#define CMD_SET_TRANSPARENT    0x17
#define CMD_SET_BK_COLOR       0x18
#define CMD_SET_FG_COLOR       0x19
#define CMD_ADJUST_NUMBER      0x1A

// 動畫控制
#define CMD_ANIM_START         0x20
#define CMD_ANIM_STOP          0x21
#define CMD_ANIM_PAUSE         0x22
#define CMD_ANIM_FRAME         0x23
#define CMD_ANIM_PREV          0x24
#define CMD_ANIM_NEXT          0x25
#define CMD_ANIM_UPLOAD        0x26
#define CMD_SET_ICON_POS       0x28

// 曲線控制
#define CMD_CURVE_ADD_CH       0x30
#define CMD_CURVE_DEL_CH       0x31
#define CMD_CURVE_ADD_DATA     0x32
#define CMD_CURVE_CLEAR_CH     0x33
#define CMD_CURVE_SCALE        0x34
#define CMD_CURVE_INSERT       0x35

// 定時器控制
#define CMD_TIMER_SET          0x40
#define CMD_TIMER_START        0x41
#define CMD_TIMER_STOP         0x42
#define CMD_TIMER_READ         0x45

// 記錄控制
#define CMD_RECORD_ADD         0x52
#define CMD_RECORD_CLEAR       0x53
#define CMD_RECORD_OFFSET      0x54
#define CMD_RECORD_COUNT       0x55
#define CMD_RECORD_READ        0x56
#define CMD_RECORD_MODIFY      0x57
#define CMD_RECORD_DELETE      0x58
#define CMD_RECORD_INSERT      0x59
#define CMD_RECORD_SELECT      0x5A
#define CMD_RECORD_BATCH       0x5B
#define CMD_RECORD_EXPORT      0x5C

// 基本繪圖指令
#define CMD_TEXT_DISPLAY       0x20
#define CMD_CURSOR_DISPLAY     0x21
#define CMD_FULL_IMAGE         0x31
#define CMD_AREA_IMAGE         0x32
#define CMD_CUT_IMAGE          0x33
#define CMD_DRAW_POINT         0x50
#define CMD_DRAW_LINE          0x51
#define CMD_DRAW_CIRCLE        0x52
#define CMD_DRAW_CIRCLE_FILL   0x53
#define CMD_DRAW_RECT          0x54
#define CMD_DRAW_RECT_FILL     0x55
#define CMD_ERASE_POINT        0x58

// 控件類型
#define CONTROL_BUTTON         0x10
#define CONTROL_TEXT           0x11
#define CONTROL_PROGRESS       0x12
#define CONTROL_SLIDER         0x13
#define CONTROL_METER          0x14
#define CONTROL_ICON           0x16
#define CONTROL_DROPDOWN       0x1A
#define CONTROL_SELECT         0x1B
#define CONTROL_RECORD         0x1D

// 顏色定義 (RGB565格式)
#define COLOR_BLACK            0x0000
#define COLOR_WHITE            0xFFFF
#define COLOR_RED              0xF800
#define COLOR_GREEN            0x07E0
#define COLOR_BLUE             0x001F
#define COLOR_YELLOW           0xFFE0
#define COLOR_CYAN             0x07FF
#define COLOR_MAGENTA          0xF81F

// 波特率定義
typedef enum {
    BAUD_1200 = 0x00,
    BAUD_2400 = 0x01,
    BAUD_4800 = 0x02,
    BAUD_9600 = 0x03,
    BAUD_19200 = 0x04,
    BAUD_38400 = 0x05,
    BAUD_57600 = 0x06,
    BAUD_115200 = 0x07,
    BAUD_1M = 0x08,
    BAUD_2M = 0x09
} baud_rate_t;

// 字體類型定義
typedef enum {
    FONT_ASCII_8X12 = 0x00,
    FONT_ASCII_8X16 = 0x01,
    FONT_ASCII_12X24 = 0x02,
    FONT_ASCII_16X32 = 0x03,
    FONT_GBK_12X12 = 0x04,
    FONT_GBK_16X16 = 0x05,
    FONT_GBK_24X24 = 0x06,
    FONT_GB2312_32X32 = 0x07,
    FONT_ASCII_32X64 = 0x08,
    FONT_GB2312_64X64 = 0x09
} font_type_t;

// 數據類型
typedef enum {
    DATA_UINT = 0x00,
    DATA_INT = 0x01,
    DATA_FLOAT = 0x02,
    DATA_DOUBLE = 0x03
} data_type_t;

// 觸摸配置
typedef struct {
    uint8_t enable : 1;          // 觸摸使能
    uint8_t beep : 1;            // 觸摸蜂鳴
    uint8_t upload_mode : 3;     // 上傳模式
    uint8_t calibrate_disable : 1; // 禁用校準
    uint8_t reserved : 2;
} touch_config_t;

// 自動休眠配置
typedef struct {
    uint8_t enable;
    uint8_t bl_on;
    uint8_t bl_off;
    uint16_t time_s;
} auto_sleep_t;

// 串口屏控制器結構
typedef struct {
    int fd;                      // 串口文件描述符
    char device[256];            // 設備路徑
    int baudrate;                // 波特率
    uint16_t current_screen;     // 當前畫面ID
    uint16_t fg_color;           // 前景色
    uint16_t bg_color;           // 背景色
    uint8_t is_connected;        // 連接狀態
} hmi_controller_t;

// 回應數據結構
typedef struct {
    uint8_t cmd;
    uint8_t *data;
    uint16_t length;
} hmi_response_t;

// 函數聲明

// 基本串口操作
int hmi_init(hmi_controller_t *hmi, const char *device, baud_rate_t baudrate);
void hmi_close(hmi_controller_t *hmi);
int hmi_send_command(hmi_controller_t *hmi, uint8_t *cmd, uint16_t length);
int hmi_send_data(hmi_controller_t *hmi, uint8_t cmd, uint8_t *data, uint16_t length);
int hmi_receive_response(hmi_controller_t *hmi, hmi_response_t *response, int timeout_ms);

// 基本功能
int hmi_handshake(hmi_controller_t *hmi);
int hmi_reset_device(hmi_controller_t *hmi);
int hmi_get_version(hmi_controller_t *hmi, char *version);
int hmi_clean_screen(hmi_controller_t *hmi);
int hmi_set_backlight(hmi_controller_t *hmi, uint8_t level);
int hmi_set_buzzer(hmi_controller_t *hmi, uint8_t time_10ms);
int hmi_set_baudrate(hmi_controller_t *hmi, baud_rate_t baudrate);

// 顏色設置
int hmi_set_fg_color(hmi_controller_t *hmi, uint16_t color);
int hmi_set_bg_color(hmi_controller_t *hmi, uint16_t color);
int hmi_set_colors(hmi_controller_t *hmi, uint16_t fg_color, uint16_t bg_color);

// 畫面控制
int hmi_switch_screen(hmi_controller_t *hmi, uint16_t screen_id);
int hmi_switch_screen_with_effect(hmi_controller_t *hmi, uint16_t screen_id, uint8_t effect, 
                                  uint8_t area_en, uint16_t left, uint16_t right, uint16_t top, uint16_t bottom);
int hmi_read_screen(hmi_controller_t *hmi, uint16_t *screen_id);

// 觸摸屏配置
int hmi_config_touch(hmi_controller_t *hmi, touch_config_t *config);
int hmi_calibrate_touch(hmi_controller_t *hmi);
int hmi_test_touch(hmi_controller_t *hmi, uint8_t enable);

// 文本控制
int hmi_update_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, const char *text);
int hmi_clear_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id);
int hmi_read_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, char *text, uint16_t max_len);
int hmi_set_text_blink(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t cycle_10ms);
int hmi_set_text_scroll(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t speed);
int hmi_set_text_color(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t fg_color, uint16_t bg_color);
int hmi_format_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, 
                    data_type_t type, uint8_t decimal, uint32_t value);

// 按鈕控制
int hmi_set_button_state(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t state);
int hmi_read_button_state(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t *state);

// 進度條和滑動條
int hmi_update_progress(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value);
int hmi_read_progress(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value);
int hmi_update_slider(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value);
int hmi_read_slider(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value);

// 儀表控制
int hmi_update_meter(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value);
int hmi_read_meter(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value);

// 圖標控制
int hmi_show_icon(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t frame_id);
int hmi_set_icon_position(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t x, uint16_t y);
int hmi_read_icon(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t *frame_id);

// 動畫控制
int hmi_start_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id);
int hmi_stop_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id);
int hmi_pause_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id);
int hmi_set_animation_frame(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t frame_id);

// 基本繪圖
int hmi_draw_point(hmi_controller_t *hmi, uint16_t x, uint16_t y);
int hmi_draw_line(hmi_controller_t *hmi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int hmi_draw_rectangle(hmi_controller_t *hmi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t filled);
int hmi_draw_circle(hmi_controller_t *hmi, uint16_t x, uint16_t y, uint16_t radius, uint8_t filled);
int hmi_display_text(hmi_controller_t *hmi, uint16_t x, uint16_t y, uint8_t background, 
                     font_type_t font, const char *text);

// 實用函數
uint16_t hmi_rgb(uint8_t r, uint8_t g, uint8_t b);
void hmi_delay_ms(uint32_t ms);
void hmi_print_buffer(uint8_t *buffer, uint16_t length);

#endif // DC_HMI_CONTROLLER_H 