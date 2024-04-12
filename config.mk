CC := gcc

TARGET_EXEC := citadel

LIBS := :libraylib.a GL m pthread dl rt X11

CPPFLAGS :=
CFLAGS := -g
LDFLAGS := -pg

INC_DIRS := $(HOME)/install/include
LIB_DIRS := $(HOME)/install/lib
