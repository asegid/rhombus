SOURCES := $(patsubst %.c,%.o,$(shell find . -name "*.c"))
SOURCES += $(patsubst %.s,%.o,$(shell find . -name "*.s"))
HEADERS := $(shell find . -name "*.h")
SUBDIR	:= librdi
STATIC	:= librdi.a
CFLAGS  += -I$(BUILDDIR)/$(SUBDIR)/inc
ASFLAGS := -felf

.PHONY: $(HEADERS) clean

all: $(STATIC)

$(HEADERS):
	@ echo " CP	" $(SUBDIR)/$@ "->" $(BUILDDIR)/inc
	@ cp $@ $(BUILDDIR)/inc

$(STATIC): $(HEADERS) $(SOURCES)
	@ echo " AR	" $(SUBDIR)/$(STATIC)
	@ $(AR) $(ARFLAGS) $(STATIC) $(SOURCES)
	@ echo " CP	" $(SUBDIR)/$(STATIC) "->" $(BUILDDIR)/lib
	@ cp $(STATIC) $(BUILDDIR)/lib

%.o: %.s
	@ echo " AS	" $(SUBDIR)/$<
	@ $(AS) $(ASFLAGS) $<

%.o: %.c
	@ echo " CC	" $(SUBDIR)/$<
	@ $(CC) $(CFLAGS) -c $< -o $@

clean:
	@ rm $(TARGET) $(SOURCES)