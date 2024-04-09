include config.mk

SRCS := $(wildcard src/*.c)
DEPS := $(SRCS:src/%.c=build/%.d)
OBJS := $(SRCS:src/%.c=build/%.o)

CPPFLAGS += -Isrc $(addprefix -I,$(INC_DIRS)) -MMD -MP
LDFLAGS += $(addprefix -l,$(LIBS)) $(addprefix -L,$(LIB_DIRS))

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

-include $(DEPS)

clean:
	$(RM) $(TARGET_EXEC) $(wildcard build/*)

.PHONY: clean
