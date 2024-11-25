# 컴파일러 및 플래그 설정
CC = aarch64-linux-gnu-gcc
CFLAGS = -Wall -I/home/vboxuser/teamproject/kim/include -g

# 디렉터리 설정
ROOT_DIR = /home/vboxuser/teamproject/kim
SRC_DIRS = $(ROOT_DIR)/src/app $(ROOT_DIR)/src/gpio $(ROOT_DIR)/src/i2c $(ROOT_DIR)/src/motor $(ROOT_DIR)/src/ultra
BUILD_DIR = $(ROOT_DIR)/build
BUILD_SUBDIRS = $(addprefix $(BUILD_DIR)/, app gpio i2c motor ultra)

# 소스 파일 및 객체 파일
SRC_FILES = $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJ_FILES = $(patsubst $(ROOT_DIR)/src/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# 실행 파일 이름
TARGET = $(BUILD_DIR)/vent_app

# 기본 목표
all: $(BUILD_SUBDIRS) $(TARGET)

# 실행 파일 생성
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# 개별 객체 파일 생성 규칙
$(BUILD_DIR)/%.o: $(ROOT_DIR)/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 빌드 디렉터리 및 하위 디렉터리 생성
$(BUILD_SUBDIRS):
	mkdir -p $@

# 클린 목표
clean:
	rm -rf $(BUILD_DIR)

# 파일을 다시 컴파일하지 않도록 설정
.PHONY: all clean
