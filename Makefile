CFLAGS = -I.
LMYSQL = -L/usr/lib/mysql -lmysqlclient
LMD5 = -lcrypto

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# list of source files and corresponding object files
SOURCES = $(wildcard $(SRCDIR)/*/*.c)
OBJECTS := $(SOURCES:$(SRCDIR)/%=$(OBJDIR)/%)
OBJECTS := $(OBJECTS:.c=.o)

PHONIES = clean
BINFILES =

# defer until all bins are added to list
all: allbins

clean:
	rm -r $(BINDIR)
	rm -r $(OBJDIR)

# macro that generates a rule that outputs a binary file.
# first arg: name of binary to produce. must match name of source dir and source file.
# second arg: optional list of object files to link with.
# third arg: optional additional linker flags.
define make_bin =
# save bin name
BINFILES += $(1)
PHONIES += $(1)

# bin shorthand target
$(1): $$(BINDIR)/$(1) ;

# bin file target
$$(BINDIR)/$(1): $$(OBJDIR)/$(1)/$(1).o $(addprefix $(OBJDIR)/,$(2))
	mkdir -p $$(BINDIR)
	$$(CC) $$(CFLAGS) $$^ -o $$@ $(3)
endef

# generate rules for binaries
$(eval $(call make_bin,account_add,,$(LMYSQL) $(LMD5)))
$(eval $(call make_bin,char_export,,$(LMYSQL)))
$(eval $(call make_bin,convert_quest))
$(eval $(call make_bin,convert_unitxt))
$(eval $(call make_bin,login_server,,$(LMYSQL) $(LMD5)))
$(eval $(call make_bin,make_key,,$(LMYSQL)))
$(eval $(call make_bin,newtable))
$(eval $(call make_bin,patch_server))
$(eval $(call make_bin,ship_server))

.PHONY: $(PHONIES)
allbins: $(BINFILES)

# object files
$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# .d files
-include $(OBJECTS:.o=.d)
