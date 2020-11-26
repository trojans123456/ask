

###################����ѡ�������###################################
CROSS = 

CPP	= @echo " g++ $@"; $(CROSS)g++
CC	= @echo " gcc $@"; $(CROSS)gcc
LD	= @echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
RM	= @echo " RM	$@"; rm -rf
STRIP	= @echo " strip  $@"; $(CROSS)strip

###################��Ŀ·���ͳ�������#################################
CFLAGS += -Wall -O -Os
CFLAGS += -D_REENTRANT

LDFLAGS +=	-llapi -lpthread -static
#LDFLAGS += "-Wl"

AFLAGS += -r

LIBDIR = ./lib/x86

BINDIR = ./bin/x86

OBJDIR=./obj/x86
SRCDIR = source
TESTDIR = .

###################includeͷ�ļ�·��##################################
CFLAGS	+= -I ../include -I . -L ../lib/x86
CFLAGS += -fpic
LFLAGS += -shared

###################lib���ļ�·��##################################
LDFLAGS += -llapi -lpthread

###################OBJ�ļ���·��############################################

LIB_SRCS_C = 		\



LIB_SRCS_CPP = 		\


LIB_OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(LIB_SRCS_C))
LIB_OBJS+=$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(LIB_SRCS_CPP))

TEST_SRCS = \
            $(TESTDIR)/command.c \
			$(TESTDIR)/main.c

TEST_OBJS = $(patsubst $(TESTDIR)/%.cpp, $(OBJDIR)/%.o, $(TEST_SRCS))

###################����Ŀ��###########################################
LIB_TARGET = $(LIBDIR)/liblapi.a
LIB_TARGET_SO = $(LIBDIR)/liblapi.so

TEST_TARGET = $(BINDIR)/liblapi_test

#TARGET	= $(LIB_TARGET) $(TEST_TARGET)
TARGET	= $(TEST_TARGET)

.PHONY: all clean rebuild

all:  clean $(TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(RM) $@;
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	$(AR) $(AFLAGS) $@ $^


$(LIB_TARGET_SO): $(LIB_OBJS)
	$(RM) $@;
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	echo $(LIB_OBJS)
	$(CPP) $(LFLAGS) -o $@ $^ $(CFLAGS)
	$(STRIP) $(LIB_TARGET_SO)

$(TEST_TARGET):  $(EXTLIBS) $(TEST_OBJS) $(LIB_TARGET)
	@$(RM) -fr $(dir $@)
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(OBJDIR)/%.o:$(SRCDIR)/%.c
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	$(CPP) -c $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o:$(TESTDIR)/%.cpp
	@[ ! -e $@ ] && mkdir -p $(dir $@)
	$(CPP) -c $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDFLAGS)

rebuild: clean all

clean:
	$(RM) $(TARGET) $(OBJDIR);