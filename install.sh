#!/bin/bash

# 大彩串口屏控制程式安裝腳本
# 
# 使用方法：
#   chmod +x install.sh
#   ./install.sh

set -e

echo "====================================="
echo "大彩串口屏控制程式安裝腳本"
echo "====================================="

# 檢查是否為root用戶
if [[ $EUID -eq 0 ]]; then
   echo "請不要使用root用戶運行此腳本" 
   echo "使用普通用戶並在需要時輸入sudo密碼"
   exit 1
fi

# 檢查必要的工具
echo "檢查系統環境..."

# 檢查gcc
if ! command -v gcc &> /dev/null; then
    echo "錯誤: 未找到gcc編譯器"
    echo "請安裝build-essential (Ubuntu/Debian) 或 Development Tools (CentOS/RHEL)"
    exit 1
fi

# 檢查make
if ! command -v make &> /dev/null; then
    echo "錯誤: 未找到make工具"
    echo "請安裝build-essential (Ubuntu/Debian) 或 Development Tools (CentOS/RHEL)"
    exit 1
fi

echo "✓ 編譯環境檢查通過"

# 檢查串口設備
echo ""
echo "檢查串口設備..."
if ls /dev/ttyUSB* &> /dev/null || ls /dev/ttyACM* &> /dev/null || ls /dev/ttyS* &> /dev/null; then
    echo "✓ 找到串口設備:"
    ls /dev/ttyUSB* /dev/ttyACM* /dev/ttyS* 2>/dev/null || true
else
    echo "⚠ 警告: 未找到串口設備"
    echo "  請確保:"
    echo "  1. 串口屏已連接到電腦"
    echo "  2. USB轉串口驅動已安裝"
    echo "  3. 設備節點權限正確"
fi

# 檢查用戶組權限
echo ""
echo "檢查用戶權限..."
if groups $USER | grep -q dialout; then
    echo "✓ 用戶已在dialout組中"
else
    echo "⚠ 警告: 用戶不在dialout組中"
    echo "  建議執行: sudo usermod -a -G dialout $USER"
    echo "  然後重新登錄系統"
fi

# 編譯程式
echo ""
echo "開始編譯..."
make clean
make

if [ $? -eq 0 ]; then
    echo "✓ 編譯成功"
else
    echo "✗ 編譯失敗"
    exit 1
fi

# 詢問是否安裝到系統
echo ""
read -p "是否要安裝庫文件到系統目錄? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "安裝庫文件到系統..."
    sudo make install
    echo "✓ 系統安裝完成"
fi

# 創建桌面快捷方式（如果是桌面環境）
if [ -n "$DISPLAY" ] && [ -d "$HOME/Desktop" ]; then
    echo ""
    read -p "是否創建桌面快捷方式? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        cat > "$HOME/Desktop/HMI_Demo.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=大彩串口屏控制程式
Comment=大彩串口屏演示和控制程式
Exec=$(pwd)/hmi_demo
Icon=applications-electronics
Terminal=true
Categories=Development;Electronics;
EOF
        chmod +x "$HOME/Desktop/HMI_Demo.desktop"
        echo "✓ 桌面快捷方式已創建"
    fi
fi

# 顯示使用說明
echo ""
echo "====================================="
echo "安裝完成！"
echo "====================================="
echo ""
echo "使用方法："
echo "1. 直接運行演示程式："
echo "   ./hmi_demo"
echo ""
echo "2. 指定串口設備和波特率："
echo "   ./hmi_demo /dev/ttyUSB0 115200"
echo ""
echo "3. 使用make快捷命令："
echo "   make run-device"
echo ""
echo "4. 查看幫助："
echo "   make help"
echo ""

# 檢查常見問題
echo "常見問題檢查："
echo ""

# 檢查串口權限
if ls /dev/ttyUSB* &> /dev/null; then
    DEVICE=$(ls /dev/ttyUSB* | head -1)
    if [ -r "$DEVICE" ] && [ -w "$DEVICE" ]; then
        echo "✓ 串口設備權限正常"
    else
        echo "⚠ 串口設備權限問題"
        echo "  解決方法: sudo chmod 666 $DEVICE"
    fi
fi

# 提供文檔鏈接
echo ""
echo "文檔和支持："
echo "- 詳細使用說明: cat README.md"
echo "- 大彩官方文檔: 大彩串口屏指令集V5.1"
echo ""
echo "如遇問題，請檢查："
echo "1. 串口連接是否正確"
echo "2. 波特率設置是否匹配"
echo "3. 設備權限是否正確"
echo "4. 驅動程式是否安裝"
echo ""
echo "安裝腳本執行完成！" 