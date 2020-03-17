ifeq ($(ARCH),arm)
COMPILER = arm-linux-gnueabihf-gcc
CFLAGS = -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7
else
COMPILER = gcc
endif

#CFLAGS += -W -Wall -DUSB_KBD_TRIGGER -I./lib/include
CFLAGS += -W -Wall -I/usr/local/include 
#CFLAGS += -W -Wall -I./lib/include
#CFLAGS += -W -Wall -DGPIO_SW-TRIGGER -I./lib/include
#LDFLAGS = -lasound -lpthread -ljson-c -lcurl -lmpg123 -lao -lFLAC
LDFLAGS = -lfftw3 -lm -lpthread -lpigpio -lz

 
TARGET = vib_anal

#vpath %.c src
SRC_PATH = .

SRCS = $(foreach dir, $(SRC_PATH), $(wildcard $(dir)/*.c))
HEADER_FILES = $(foreach dir, $(SRC_PATH), $(wildcard $(dir)/*.h))
OBJS = $(SRCS:.c=.o)

%.o : %.c $(HEADER_FILES)
	$(COMPILER) $(CFLAGS) -c -o $@ $<


all: $(TARGET)

$(TARGET): $(OBJS)
	$(COMPILER) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET)


#This is for compile test
.PHONY: echo
echo:
	@echo $(SRCS)
	@echo $(OBJS)


