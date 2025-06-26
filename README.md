# 大彩串口屏控制程式

這是一個用於Linux系統的大彩串口屏完整控制程式，支持大彩串口屏指令集V5.1的所有功能。

## 功能特點

### 🚀 核心功能
- **完整指令集支持** - 實現大彩串口屏指令集V5.1的所有功能
- **組態指令** - 支持所有控件操作（按鈕、文本、進度條、儀表、動畫等）
- **基本指令** - 支持底層繪圖操作（畫點、線、圓、矩形、文字顯示等）
- **串口通信** - 可靠的串口通信機制，支援多種波特率
- **觸摸支持** - 完整的觸摸屏配置和事件處理

### 📱 支持的控件
- 🔘 **按鈕控件** - 狀態切換、事件響應
- 📝 **文本控件** - 文字顯示、顏色變更、閃爍、滾動效果
- 📊 **進度條** - 數值顯示、顏色配置
- 🎚️ **滑動條** - 位置控制、數值讀取
- 🌡️ **儀表控件** - 指針控制、數值顯示
- 🎬 **動畫控件** - GIF動畫播放、幀控制
- 🖼️ **圖標控件** - 多幀圖標切換、位置設定
- 📈 **曲線控件** - 實時數據繪製
- 📋 **下拉菜單** - 選項選擇
- 🎯 **二維碼** - 動態二維碼生成

### 🎨 繪圖功能
- 繪製點、線、矩形、圓形、橢圓
- 文字顯示（支援多種字體和編碼）
- 顏色設置（RGB565格式）
- 圖片顯示和剪切

### ⚙️ 系統功能
- 背光亮度調節
- 蜂鳴器控制
- 觸摸屏校準和配置
- 設備復位和版本查詢
- 波特率動態設定

## 系統需求

- **操作系統**: Linux（Ubuntu、Debian、CentOS等）
- **編譯器**: GCC 4.8 或更新版本
- **依賴庫**: pthread（多線程支持）
- **硬件**: 串口設備（USB轉串口或板載UART）

## 安裝和編譯

### 1. 克隆或下載源碼
```bash
# 如果從git獲取
git clone <repository-url>
cd dc_hmi_controller

# 或者解壓下載的源碼包
tar -xzf dc_hmi_controller.tar.gz
cd dc_hmi_controller
```

### 2. 編譯程式
```bash
# 編譯所有目標（推薦）
make

# 或分別編譯
make hmi_demo      # 編譯演示程式
make libdc_hmi.a   # 編譯靜態庫
make libdc_hmi.so  # 編譯動態庫
```

### 3. 安裝到系統（可選）
```bash
# 安裝庫文件到系統目錄
sudo make install

# 卸載
sudo make uninstall
```

## 使用方法

### 快速開始

1. **連接串口屏**
   ```bash
   # 檢查串口設備
   ls /dev/ttyUSB* /dev/ttyACM* /dev/ttyS*
   
   # 設置串口權限（如需要）
   sudo chmod 666 /dev/ttyUSB0
   ```

2. **運行演示程式**
   ```bash
   # 使用默認參數（/dev/ttyUSB0, 115200）
   ./hmi_demo
   
   # 指定串口設備和波特率
   ./hmi_demo /dev/ttyUSB0 115200
   
   # 或使用make快捷指令
   make run-device
   ```

3. **操作選單**
   ```
   === 串口屏控制選單 ===
   1. 基本功能演示      - 清屏、背光、蜂鳴器
   2. 繪圖功能演示      - 點、線、圓、矩形、文字
   3. 控件操作演示      - 文本、進度條、按鈕、儀表
   4. 動畫和圖標演示    - GIF動畫、圖標切換
   5. 文本效果演示      - 顏色、閃爍、滾動效果
   6. 觸摸屏校準        - 觸摸屏校準功能
   7. 清屏              - 清除屏幕內容
   8. 複位設備          - 重啟串口屏
   0. 退出程式
   ```

### API使用範例

```c
#include "dc_hmi_controller.h"

int main() {
    hmi_controller_t hmi;
    
    // 1. 初始化串口屏
    if (hmi_init(&hmi, "/dev/ttyUSB0", BAUD_115200) < 0) {
        printf("初始化失敗\n");
        return -1;
    }
    
    // 2. 握手確認連接
    if (hmi_handshake(&hmi) == 0) {
        printf("連接成功\n");
    }
    
    // 3. 基本操作
    hmi_clean_screen(&hmi);              // 清屏
    hmi_set_backlight(&hmi, 128);        // 設置背光
    hmi_set_buzzer(&hmi, 10);            // 蜂鳴100ms
    
    // 4. 繪圖操作
    hmi_set_fg_color(&hmi, COLOR_RED);   // 設置前景色為紅色
    hmi_draw_circle(&hmi, 100, 100, 50, 1);  // 畫實心圓
    hmi_display_text(&hmi, 50, 200, 1, FONT_GBK_16X16, "測試文字");
    
    // 5. 控件操作
    hmi_switch_screen(&hmi, 1);          // 切換到畫面1
    hmi_update_text(&hmi, 1, 1, "Hello World");     // 更新文本控件
    hmi_update_progress(&hmi, 1, 2, 75); // 設置進度條75%
    hmi_set_button_state(&hmi, 1, 3, 1); // 設置按鈕為按下狀態
    
    // 6. 清理
    hmi_close(&hmi);
    return 0;
}
```

### 配置觸摸屏

```c
// 配置觸摸屏參數
touch_config_t touch_config = {0};
touch_config.enable = 1;                // 啟用觸摸
touch_config.beep = 1;                  // 觸摸時蜂鳴
touch_config.upload_mode = 3;           // 按下和釋放都上傳
touch_config.calibrate_disable = 0;     // 允許校準

hmi_config_touch(&hmi, &touch_config);

// 手動校準
hmi_calibrate_touch(&hmi);
```

## 項目結構

```
dc_hmi_controller/
├── dc_hmi_controller.h     # 主頭文件（定義和聲明）
├── dc_hmi_controller.c     # 核心實現（串口通信、基本功能）
├── dc_hmi_controls.c       # 控件操作實現
├── hmi_demo.c             # 演示程式
├── Makefile               # 編譯配置
└── README.md              # 說明文檔
```

## 支持的指令

### 基本指令
- `hmi_handshake()` - 握手
- `hmi_reset_device()` - 復位設備
- `hmi_get_version()` - 獲取版本
- `hmi_clean_screen()` - 清屏
- `hmi_set_backlight()` - 背光調節
- `hmi_set_buzzer()` - 蜂鳴器控制

### 畫面控制
- `hmi_switch_screen()` - 切換畫面
- `hmi_switch_screen_with_effect()` - 帶效果切換畫面
- `hmi_read_screen()` - 讀取當前畫面

### 控件操作
- `hmi_update_text()` - 更新文本
- `hmi_update_progress()` - 更新進度條
- `hmi_update_meter()` - 更新儀表
- `hmi_set_button_state()` - 設置按鈕狀態
- `hmi_show_icon()` - 顯示圖標
- `hmi_start_animation()` - 開始動畫

### 繪圖指令
- `hmi_draw_point()` - 畫點
- `hmi_draw_line()` - 畫線
- `hmi_draw_circle()` - 畫圓
- `hmi_draw_rectangle()` - 畫矩形
- `hmi_display_text()` - 顯示文字

### 顏色控制
- `hmi_set_fg_color()` - 設置前景色
- `hmi_set_bg_color()` - 設置背景色
- `hmi_rgb()` - RGB顏色轉換

## 常見問題

### Q: 無法連接串口設備
**A:** 檢查以下事項：
```bash
# 1. 確認設備存在
ls -l /dev/ttyUSB*

# 2. 檢查權限
sudo chmod 666 /dev/ttyUSB0

# 3. 檢查設備是否被占用
sudo lsof /dev/ttyUSB0

# 4. 檢查用戶組權限
sudo usermod -a -G dialout $USER
```

### Q: 編譯錯誤
**A:** 確保安裝了必要的開發工具：
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
```

### Q: 觸摸無響應
**A:** 檢查觸摸屏配置：
```c
// 確保觸摸屏已啟用
touch_config_t config = {0};
config.enable = 1;
hmi_config_touch(&hmi, &config);

// 嘗試校準
hmi_calibrate_touch(&hmi);
```

### Q: 控件無反應
**A:** 確認畫面和控件ID正確：
```c
// 確保畫面ID和控件ID與上位機配置一致
hmi_switch_screen(&hmi, 1);        // 切換到正確畫面
hmi_update_text(&hmi, 1, 1, "Test"); // 使用正確的控件ID
```

## 技術支援

- **指令參考**: 請參考「大彩串口屏指令集V5.1」官方文檔
- **硬體支援**: 支援大彩全系列串口屏產品
- **開發環境**: 推薦使用Ubuntu 18.04+或CentOS 7+

## 授權說明

本程式基於開源精神開發，請遵循相關授權條款使用。

## 更新日誌

### v1.0.0
- 實現完整的大彩串口屏指令集V5.1
- 支援所有組態指令和基本指令
- 提供完整的演示程式
- 支援多線程觸摸事件處理

---

**注意**: 使用前請確保已正確連接串口屏設備，並參考硬體說明書進行正確的接線。 