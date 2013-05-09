include $(TOPDIR)/config.mk

LIB    = $(obj)libgeneric.a

COBJS  = crc16.o crc32.o adler32.o infblock.o infcodes.o inflate.o \
         inftrees.o infutil.o zutil.o uncompr.o LzmaDecode.o \
         ctype.o display_options.o string.o vsprintf.o tbs_common.o \
	 led.o

SRCS   := $(COBJS:.o=.c)
OBJS   := $(addprefix $(obj),$(COBJS))


all : $(LIB)


$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)



#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################







