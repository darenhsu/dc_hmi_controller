#include "dc_hmi_controller.h"
#include <pthread.h>
#include <signal.h>

// 全局變量
static hmi_controller_t hmi;
static int running = 1;

// 信號處理函數
void signal_handler(int sig) {
    printf("\n收到信號 %d，正在退出程式...\n", sig);
    running = 0;
}

// 觸摸事件處理線程
void* touch_handler_thread(void *arg) {
    hmi_controller_t *hmi_dev = (hmi_controller_t*)arg;
    uint8_t buffer[256];
    
    printf("觸摸事件處理線程已啟動\n");
    
    while (running) {
        int n = read(hmi_dev->fd, buffer, sizeof(buffer));
        if (n > 0) {
            // 檢查是否是觸摸事件
            if (n >= 6 && buffer[0] == FRAME_HEADER) {
                if (buffer[1] == 0x01) { // 觸摸按下
                    uint16_t x = (buffer[2] << 8) | buffer[3];
                    uint16_t y = (buffer[4] << 8) | buffer[5];
                    printf("觸摸按下: (%d, %d)\n", x, y);
                } else if (buffer[1] == 0x03) { // 觸摸釋放
                    uint16_t x = (buffer[2] << 8) | buffer[3];
                    uint16_t y = (buffer[4] << 8) | buffer[5];
                    printf("觸摸釋放: (%d, %d)\n", x, y);
                } else if (buffer[1] == CMD_CONFIG_BASE && buffer[2] == 0x11) {
                    // 控件事件上傳
                    uint16_t screen_id = (buffer[3] << 8) | buffer[4];
                    uint16_t control_id = (buffer[5] << 8) | buffer[6];
                    uint8_t control_type = buffer[7];
                    printf("控件事件: 畫面=%d, 控件=%d, 類型=0x%02X\n", 
                           screen_id, control_id, control_type);
                }
            }
        }
        usleep(10000); // 10ms延遲
    }
    
    printf("觸摸事件處理線程已退出\n");
    return NULL;
}

// 演示基本功能
void demo_basic_functions() {
    printf("\n=== 基本功能演示 ===\n");
    
    // 清屏
    printf("清屏...\n");
    hmi_clean_screen(&hmi);
    hmi_delay_ms(1000);
    
    // 設置背光
    printf("調節背光亮度...\n");
    for (int i = 0; i <= 255; i += 32) {
        hmi_set_backlight(&hmi, i);
        hmi_delay_ms(200);
    }
    hmi_set_backlight(&hmi, 0); // 恢復最亮
    
    // 蜂鳴器測試
    printf("蜂鳴器測試...\n");
    hmi_set_buzzer(&hmi, 10); // 100ms蜂鳴
    hmi_delay_ms(500);
}

// 演示繪圖功能
void demo_drawing() {
    printf("\n=== 繪圖功能演示 ===\n");
    
    // 設置顏色
    hmi_set_colors(&hmi, COLOR_YELLOW, COLOR_BLUE);
    hmi_clean_screen(&hmi);
    
    // 畫點
    printf("畫點...\n");
    for (int i = 0; i < 100; i += 5) {
        hmi_draw_point(&hmi, 10 + i, 10 + i/2);
    }
    hmi_delay_ms(1000);
    
    // 畫線
    printf("畫線...\n");
    hmi_set_fg_color(&hmi, COLOR_RED);
    hmi_draw_line(&hmi, 50, 50, 200, 100);
    hmi_draw_line(&hmi, 50, 100, 200, 50);
    hmi_delay_ms(1000);
    
    // 畫矩形
    printf("畫矩形...\n");
    hmi_set_fg_color(&hmi, COLOR_GREEN);
    hmi_draw_rectangle(&hmi, 220, 30, 350, 120, 0); // 空心
    hmi_draw_rectangle(&hmi, 230, 40, 340, 110, 1); // 實心
    hmi_delay_ms(1000);
    
    // 畫圓
    printf("畫圓...\n");
    hmi_set_fg_color(&hmi, COLOR_CYAN);
    hmi_draw_circle(&hmi, 400, 75, 40, 0); // 空心
    hmi_draw_circle(&hmi, 400, 75, 20, 1); // 實心
    hmi_delay_ms(1000);
    
    // 顯示文字
    printf("顯示文字...\n");
    hmi_set_fg_color(&hmi, COLOR_WHITE);
    hmi_display_text(&hmi, 50, 150, 1, FONT_GBK_16X16, "大彩串口屏測試程式");
    hmi_display_text(&hmi, 50, 180, 0, FONT_ASCII_12X24, "HMI Controller Demo");
    hmi_delay_ms(2000);
}

// 演示控件操作
void demo_controls() {
    printf("\n=== 控件操作演示 ===\n");
    
    // 切換到畫面1（假設存在）
    printf("切換畫面...\n");
    hmi_switch_screen(&hmi, 1);
    hmi_delay_ms(1000);
    
    // 更新文本控件
    printf("更新文本控件...\n");
    for (int i = 0; i < 100; i++) {
        char text[32];
        sprintf(text, "數值: %d", i);
        hmi_update_text(&hmi, 1, 1, text); // 畫面1, 控件1
        hmi_delay_ms(50);
    }
    
    // 更新進度條
    printf("更新進度條...\n");
    for (int i = 0; i <= 100; i++) {
        hmi_update_progress(&hmi, 1, 2, i); // 畫面1, 控件2
        hmi_delay_ms(50);
    }
    
    // 儀表控制
    printf("控制儀表...\n");
    for (int i = 0; i <= 100; i += 5) {
        hmi_update_meter(&hmi, 1, 3, i); // 畫面1, 控件3
        hmi_delay_ms(100);
    }
    
    // 按鈕狀態切換
    printf("按鈕狀態切換...\n");
    for (int i = 0; i < 5; i++) {
        hmi_set_button_state(&hmi, 1, 4, 1); // 按下
        hmi_delay_ms(500);
        hmi_set_button_state(&hmi, 1, 4, 0); // 彈起
        hmi_delay_ms(500);
    }
}

// 演示動畫和圖標
void demo_animation() {
    printf("\n=== 動畫和圖標演示 ===\n");
    
    // 切換到畫面2（假設存在動畫控件）
    hmi_switch_screen(&hmi, 2);
    hmi_delay_ms(1000);
    
    // 啟動動畫
    printf("啟動動畫...\n");
    hmi_start_animation(&hmi, 2, 1);
    hmi_delay_ms(3000);
    
    // 暫停動畫
    printf("暫停動畫...\n");
    hmi_pause_animation(&hmi, 2, 1);
    hmi_delay_ms(2000);
    
    // 繼續動畫
    printf("繼續動畫...\n");
    hmi_start_animation(&hmi, 2, 1);
    hmi_delay_ms(2000);
    
    // 停止動畫
    printf("停止動畫...\n");
    hmi_stop_animation(&hmi, 2, 1);
    
    // 圖標切換演示
    printf("圖標切換...\n");
    for (int i = 0; i < 5; i++) {
        hmi_show_icon(&hmi, 2, 2, i % 3); // 循環顯示不同幀
        hmi_delay_ms(500);
    }
}

// 演示顏色和文本效果
void demo_text_effects() {
    printf("\n=== 文本效果演示 ===\n");
    
    // 切換到畫面3
    hmi_switch_screen(&hmi, 3);
    hmi_delay_ms(1000);
    
    // 文本顏色變化
    printf("文本顏色變化...\n");
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA};
    for (int i = 0; i < 6; i++) {
        hmi_set_text_color(&hmi, 3, 1, colors[i], COLOR_BLACK);
        hmi_update_text(&hmi, 3, 1, "顏色測試文字");
        hmi_delay_ms(1000);
    }
    
    // 文本閃爍
    printf("文本閃爍效果...\n");
    hmi_set_text_blink(&hmi, 3, 2, 50); // 500ms閃爍週期
    hmi_update_text(&hmi, 3, 2, "閃爍文字");
    hmi_delay_ms(5000);
    hmi_set_text_blink(&hmi, 3, 2, 0); // 停止閃爍
    
    // 文本滾動
    printf("文本滾動效果...\n");
    hmi_update_text(&hmi, 3, 3, "這是一段很長的滾動文字，用來測試滾動效果");
    hmi_set_text_scroll(&hmi, 3, 3, 50); // 滾動速度50像素/秒
    hmi_delay_ms(10000);
    hmi_set_text_scroll(&hmi, 3, 3, 0); // 停止滾動
}

// 配置觸摸屏
void setup_touch() {
    printf("配置觸摸屏...\n");
    
    touch_config_t touch_config = {0};
    touch_config.enable = 1;
    touch_config.beep = 1;
    touch_config.upload_mode = 3; // 按下和釋放都上傳
    touch_config.calibrate_disable = 0;
    
    hmi_config_touch(&hmi, &touch_config);
}

// 主函數
int main(int argc, char *argv[]) {
    char *device = "/dev/ttyUSB0"; // 默認設備
    baud_rate_t baudrate = BAUD_115200;
    
    // 處理命令行參數
    if (argc > 1) {
        device = argv[1];
    }
    if (argc > 2) {
        int baud = atoi(argv[2]);
        switch (baud) {
            case 9600: baudrate = BAUD_9600; break;
            case 19200: baudrate = BAUD_19200; break;
            case 38400: baudrate = BAUD_38400; break;
            case 57600: baudrate = BAUD_57600; break;
            case 115200: baudrate = BAUD_115200; break;
            default: 
                printf("不支持的波特率: %d\n", baud);
                return -1;
        }
    }
    
    // 註冊信號處理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("大彩串口屏控制演示程式\n");
    printf("設備: %s, 波特率: %d\n", device, 
           baudrate == BAUD_9600 ? 9600 :
           baudrate == BAUD_19200 ? 19200 :
           baudrate == BAUD_38400 ? 38400 :
           baudrate == BAUD_57600 ? 57600 : 115200);
    
    // 初始化串口屏
    if (hmi_init(&hmi, device, baudrate) < 0) {
        printf("初始化串口屏失敗\n");
        return -1;
    }
    
    // 獲取版本信息
    char version[64];
    if (hmi_get_version(&hmi, version) == 0) {
        printf("串口屏版本: %s\n", version);
    }
    
    // 配置觸摸屏
    setup_touch();
    
    // 創建觸摸事件處理線程
    pthread_t touch_thread;
    pthread_create(&touch_thread, NULL, touch_handler_thread, &hmi);
    
    // 交互式選單
    while (running) {
        printf("\n=== 串口屏控制選單 ===\n");
        printf("1. 基本功能演示\n");
        printf("2. 繪圖功能演示\n");
        printf("3. 控件操作演示\n");
        printf("4. 動畫和圖標演示\n");
        printf("5. 文本效果演示\n");
        printf("6. 觸摸屏校準\n");
        printf("7. 清屏\n");
        printf("8. 複位設備\n");
        printf("0. 退出程式\n");
        printf("請選擇功能 (0-8): ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            // 清空輸入緩衝區
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        switch (choice) {
            case 1:
                demo_basic_functions();
                break;
            case 2:
                demo_drawing();
                break;
            case 3:
                demo_controls();
                break;
            case 4:
                demo_animation();
                break;
            case 5:
                demo_text_effects();
                break;
            case 6:
                printf("開始觸摸屏校準...\n");
                hmi_calibrate_touch(&hmi);
                break;
            case 7:
                printf("清屏...\n");
                hmi_clean_screen(&hmi);
                break;
            case 8:
                printf("複位設備...\n");
                hmi_reset_device(&hmi);
                hmi_delay_ms(2000); // 等待設備重啟
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("無效選擇，請重新輸入\n");
                break;
        }
    }
    
    // 清理資源
    printf("正在清理資源...\n");
    running = 0;
    pthread_join(touch_thread, NULL);
    hmi_close(&hmi);
    
    printf("程式已退出\n");
    return 0;
} 