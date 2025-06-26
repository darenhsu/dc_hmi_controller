# 大彩串口屏控制程式 Makefile
# 適用於Linux系統

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -pthread

# 目標文件
TARGET = hmi_demo
LIB_TARGET = libdc_hmi.a
SHARED_LIB = libdc_hmi.so

# 源文件
SOURCES = dc_hmi_controller.c dc_hmi_controls.c
LIB_OBJECTS = $(SOURCES:.c=.o)
DEMO_SOURCES = hmi_demo.c
DEMO_OBJECTS = $(DEMO_SOURCES:.c=.o)

# 頭文件
HEADERS = dc_hmi_controller.h

# 默認目標
all: $(TARGET) $(LIB_TARGET) $(SHARED_LIB)

# 編譯演示程式
$(TARGET): $(DEMO_OBJECTS) $(LIB_TARGET)
	$(CC) $(DEMO_OBJECTS) -L. -ldc_hmi $(LDFLAGS) -o $@

# 創建靜態庫
$(LIB_TARGET): $(LIB_OBJECTS)
	ar rcs $@ $^

# 創建動態庫
$(SHARED_LIB): $(LIB_OBJECTS)
	$(CC) -shared -fPIC $^ -o $@

# 編譯目標文件
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# 安裝到系統
install: $(LIB_TARGET) $(SHARED_LIB) $(HEADERS)
	@echo "安裝串口屏控制庫..."
	sudo cp $(LIB_TARGET) /usr/local/lib/
	sudo cp $(SHARED_LIB) /usr/local/lib/
	sudo cp $(HEADERS) /usr/local/include/
	sudo ldconfig
	@echo "安裝完成"

# 卸載
uninstall:
	@echo "卸載串口屏控制庫..."
	sudo rm -f /usr/local/lib/$(LIB_TARGET)
	sudo rm -f /usr/local/lib/$(SHARED_LIB)
	sudo rm -f /usr/local/include/dc_hmi_controller.h
	sudo ldconfig
	@echo "卸載完成"

# 清理編譯文件
clean:
	rm -f *.o $(TARGET) $(LIB_TARGET) $(SHARED_LIB)

# 完全清理
distclean: clean
	rm -f *~

# 運行演示程式
run: $(TARGET)
	./$(TARGET)

# 運行演示程式並指定設備
run-device: $(TARGET)
	./$(TARGET) /dev/ttyUSB0 115200

# 檢查語法
check:
	$(CC) $(CFLAGS) -fsyntax-only $(SOURCES) $(DEMO_SOURCES)

# 創建發布包
dist: clean
	@echo "創建發布包..."
	mkdir -p dist/dc_hmi_controller
	cp *.c *.h Makefile README.md dist/dc_hmi_controller/
	cd dist && tar -czf dc_hmi_controller.tar.gz dc_hmi_controller/
	@echo "發布包已創建: dist/dc_hmi_controller.tar.gz"

# 幫助信息
help:
	@echo "大彩串口屏控制程式編譯選項："
	@echo ""
	@echo "  make              - 編譯所有目標（演示程式和庫）"
	@echo "  make $(TARGET)    - 僅編譯演示程式"
	@echo "  make $(LIB_TARGET) - 僅編譯靜態庫"
	@echo "  make $(SHARED_LIB) - 僅編譯動態庫"
	@echo "  make install      - 安裝庫到系統"
	@echo "  make uninstall    - 從系統卸載庫"
	@echo "  make clean        - 清理編譯文件"
	@echo "  make distclean    - 完全清理"
	@echo "  make run          - 運行演示程式"
	@echo "  make run-device   - 運行演示程式（指定設備）"
	@echo "  make check        - 檢查語法"
	@echo "  make dist         - 創建發布包"
	@echo "  make help         - 顯示此幫助信息"
	@echo ""
	@echo "使用範例："
	@echo "  make"
	@echo "  ./$(TARGET) /dev/ttyUSB0 115200"

# 偽目標聲明
.PHONY: all clean distclean install uninstall run run-device check dist help

# 依賴關係
dc_hmi_controller.o: dc_hmi_controller.c dc_hmi_controller.h
dc_hmi_controls.o: dc_hmi_controls.c dc_hmi_controller.h
hmi_demo.o: hmi_demo.c dc_hmi_controller.h 