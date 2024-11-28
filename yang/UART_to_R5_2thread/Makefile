CC = aarch64-linux-gnu-gcc
CFLAGS = -g -I/media/sf_linux_virtual/telechips_2team/yang/UART_to_R5_2thread/header -lpthread
SRCS = src/i2c.c src/uart.c src/i2c_uart_R5.c
TARGET = program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(TARGET)
