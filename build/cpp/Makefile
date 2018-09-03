CPPFLAGS := -g -std=c++11 -I/usr/local/include -pthread
RM = rm -fr  # rm command
TARGET_LIB = libxrtelemetry.a # target lib
LIBDIR = /usr/local/lib/
INCLUDEDIR = /usr/local/include/xrtelemetry

SRCDIR = src/gen-ipv6-nd-cpp
OBJDIR = src/gen-ipv6-nd-obj

STRUCTURE := $(shell find ./$(SRCDIR) -type d -links 2)   
INCSTRUCTURE := $(addprefix $(INCLUDEDIR)/,$(STRUCTURE)) 
CODEFILES := $(addsuffix /*,$(STRUCTURE))
CODEFILES := $(wildcard $(CODEFILES))   
SRCFILES := $(filter %.cc,$(CODEFILES))
OBJFILES := $(subst $(SRCDIR),$(OBJDIR),$(SRCFILES:%.cc=%.o)) 
OBJFILES := $(OBJFILES) $(OBJDIR)/telemetry.pb.o $(OBJDIR)/telemetry.grpc.pb.o
INCFILES := $(filter %.h,$(CODEFILES)) 
INCFILES := $(subst ./src/gen-ipv6-nd-cpp/,,$(INCFILES))
INCFILES := $(INCFILES) telemetry.pb.h telemetry.grpc.pb.h
print-%  : ; @echo $* = $($*)

.PHONY: all
all: $(TARGET_LIB)

$(OBJDIR)/%.o: $(addprefix $(SRCDIR)/,%.cc %.h)
	g++ $(CPPFLAGS) -I$(SRCDIR) -c -o $@ $<


$(TARGET_LIB): $(OBJFILES)
	ar rcv $(TARGET_LIB) $^
	ranlib $(TARGET_LIB)
        
.PHONY: install
install::
	mkdir -p $(INCLUDEDIR)
	cp -p $(TARGET_LIB) $(LIBDIR)
	cd $(SRCDIR);cp -p --parents $(INCFILES) $(INCLUDEDIR)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${INCLUDEDIR} src/
