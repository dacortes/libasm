# SPDX-License-Identifier: GPL-2.0

################################################################################
#                               COLOR DEFINITIONS                              #
################################################################################
END = \033[m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
LIGTH = \033[1m
DARK = \033[2m
ITALIC = \033[3m

SUCCESS = $(LIGTH)$(GREEN)[SUCCESS]$(END)
WARNING = $(LIGTH)$(YELLOW)[WARNING]$(END)
INFO = $(LIGTH)$(BLUE)[INFO]$(END)
ERROR = $(LIGTH)$(RED)[ERROR]$(END)

################################################################################
#                               BUILD VARIABLES                                     #
################################################################################

NAME := libasm
RMV = rm -rf

SOURCES := \
	ft_strlen.s ft_strcpy.s ft_strcmp.s

DIR_TEST := tests/src/
SOURCES_TEST := \
	$(DIR_TEST)test_strlen.c $(DIR_TEST)test_strcpy.c \
	$(DIR_TEST)test_strcmp.c \
	$(DIR_TEST)main.c

DIRECTORY_OBJ = .obj
DIRECTORY_DEP = .dep
OBJECTS = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES:.s=.o))
DEPENDENCIES = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES:.s=.d))
TOTAL_FILES = $(words $(SOURCES))

OBJECTS_TEST = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES_TEST:.c=.o))
DEPENDENCIES_TEST = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES_TEST:.c=.d))
TOTAL_FILES_TEST = $(words $(SOURCES_TEST))


ASM := nasm -f elf64 -w+all -w+error -w+orphan-labels -g
FLAGS_ASM := -MD -MF
LIB_ASM := ar rcs

INCLUDES := $(addprefix -I, ./tests/includes)
CC := gcc -Wall #-Werror #-Wextra

DIRS_TO_CREATE = $(DIRECTORY_OBJ) $(DIRECTORY_DEP)
SUB_DIRECTORIES := tests/src

REQUIRED_TOOLS = nasm gcc

################################################################################
#                               DEFAULT TARGET                                 #
################################################################################
all: dir $(OBJECTS) $(NAME)

################################################################################
#                               REQUIREMENTS CHECK                             #
################################################################################
check_requirements:
	@failed=false; \
	for tool in $(REQUIRED_TOOLS); do \
		if command -v $$tool >/dev/null 2>&1; then \
			printf "$(GREEN)$(LIGTH)[$$tool is installed.]$(END) Proceeding...\n"; \
		else \
			printf "$(RED)$(LIGTH)[$$tool is not installed.]$(END) Please install $$tool to proceed.\n"; \
			failed=true; \
		fi; \
	done; \
	if [ "$$failed" = true ]; then exit 1; fi
	@if [ ! -f $(LINKER) ]; then \
		printf "$(ERROR) Linker script $(LINKER) not found!\n"; \
		exit 1; \
	fi

################################################################################
#                               HELP TARGET                                    #
################################################################################
help:
	@printf "$(INFO) Available rules:\n\n"
	@printf "  $(BLUE)make all$(END)        $(INFO) Build directories, compile objects and create the library\n"
	@printf "  $(BLUE)make dir$(END)        $(INFO) Create required directories (.obj and .dep)\n"
	@printf "  $(BLUE)make $(NAME)$(END)     $(INFO) Build the static library $(NAME).a\n"
	@printf "  $(BLUE)make test$(END)       $(INFO) Build test executable 'test' linking libasm\n"
	@printf "  $(BLUE)make clean$(END)      $(INFO) Remove object and dependency directories\n"
	@printf "  $(BLUE)make fclean$(END)     $(INFO) Run clean and also remove library and executable\n"
	@printf "  $(BLUE)make re$(END)         $(INFO) Rebuild everything from scratch\n"
	@printf "  $(BLUE)make help$(END)       $(INFO) Show this help message\n"

################################################################################
#                               DIRECTORY CREATION                             #
################################################################################
dir:
	@for DIR in $(DIRS_TO_CREATE); do \
		if [ ! -d $$DIR ]; then \
			mkdir -p $$DIR; \
			$(foreach SUB, $(SUB_DIRECTORIES), mkdir -p $$DIR/$(SUB);) \
			printf "$(BLUE)$(LIGTH)Creating directory:$(END) $$DIR\n"; \
		else \
			printf "$(BLUE)$(LIGTH)Directory already exists:$(END) $$DIR\n"; \
		fi \
	done

$(DIRECTORY_OBJ)/%.o: %.s Makefile
	@printf "$(INFO) Compiling $< ... \n"
	@if $(ASM) $(FLAGS_ASM) $(DIRECTORY_DEP)/$*.d $< -o $@; then \
		printf "$(SUCCESS) Object generated: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
	fi
	@$(call progress,$<)

$(DIRECTORY_OBJ)/%.o: %.c Makefile
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MF $(DIRECTORY_DEP)/$*.d -c $< -o $@
	@$(call progress_test,$<)


$(NAME): $(OBJECTS)
	@printf "\n$(INFO) Creating static library $(NAME).a ... \n"
	@if $(LIB_ASM) $(NAME).a $(OBJECTS); then \
		printf "$(SUCCESS) Library created: $(NAME).a\n"; \
	else \
		printf "$(ERROR) Failed to create library\n"; \
	fi

test: dir $(NAME) $(OBJECTS_TEST)
	@printf "\n$(INFO) Building test executable ... "
	@if $(CC) $(OBJECTS_TEST) -L. -lasm -o test; then \
		printf "\n$(SUCCESS) Test executable created: test\n"; \
	else \
		printf "$(ERROR) Failed to build tests\n"; \
	fi


################################################################################
#                               CLEANUP TARGETS                                #
################################################################################
clean:
	@for DIR in $(DIRS_TO_CREATE); do \
		if [ -d $$DIR ]; then \
			$(RMV) $$DIR; \
			printf "$(SUCCESS) Removed directory: $$DIR\n"; \
		else \
			printf "$(WARNING) Directory not found: $$DIR\n"; \
		fi \
	done

	@printf "$(SUCCESS) Object and dependency files cleaned.\n"

fclean: clean
	@if [ -f $(NAME).a ]; then \
		$(RMV) $(NAME).a; \
		printf "$(SUCCESS) Removed library: $(NAME).a\n"; \
	else \
		printf "$(WARNING) Library not found: $(NAME).a\n"; \
	fi

	@if [ -f test ]; then \
		$(RMV) test; \
		$(RMV) logs; \
		printf "$(SUCCESS) Removed executable: main\n"; \
	else \
		printf "$(WARNING) Executable not found: main\n"; \
	fi

re: fclean all

################################################################################
#                               PROGRESS BAR                                   #
################################################################################

define progress
	@$(eval COMPILED_FILES=$(shell expr $(COMPILED_FILES) + 1))
	@bash -c 'PROG_BAR_WIDTH=50; \
	PROGRESS=$$(($(COMPILED_FILES) * $$PROG_BAR_WIDTH / $(TOTAL_FILES))); \
	EMPTY=$$(($$PROG_BAR_WIDTH - $$PROGRESS)); \
	printf "\r$(BLUE)[ "; \
	for ((i = 0; i < $$PROGRESS; i++)); do echo -n "█"; done; \
	for ((i = 0; i < $$EMPTY; i++)); do echo -n " "; done; \
	printf " ]$(END) %d%%" $$((100 * $(COMPILED_FILES) / $(TOTAL_FILES)));'
endef

define progress_test
	@$(eval COMPILED_FILES_TEST=$(shell expr $(COMPILED_FILES_TEST) + 1))
	@bash -c 'PROG_BAR_WIDTH=50; \
	PROGRESS=$$(($(COMPILED_FILES_TEST) * $$PROG_BAR_WIDTH / $(TOTAL_FILES_TEST))); \
	EMPTY=$$(($$PROG_BAR_WIDTH - $$PROGRESS)); \
	printf "\r$(BLUE)[ "; \
	for ((i = 0; i < $$PROGRESS; i++)); do echo -n "█"; done; \
	for ((i = 0; i < $$EMPTY; i++)); do echo -n " "; done; \
	printf " ]$(END) %d%%" $$((100 * $(COMPILED_FILES_TEST) / $(TOTAL_FILES_TEST)));'
endef

COMPILED_FILES=0
COMPILED_FILES_TEST=0

-include $(DEPENDENCIES)
-include $(DEPENDENCIES_TEST)

.SILENT:
.PHONY: all help test $(NAME) test clean fclean re