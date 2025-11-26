################################################################################
#                               COLORS                                         #
################################################################################

END = \033[m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
LIGTH = \033[1m
DARK = \033[2m 
ITALIC = \033[3m

################################################################################
#                               LABELS                                         #
################################################################################

SUCCESS = $(LIGTH)$(GREEN)[SUCCESS]$(END)
WARNING = $(LIGTH)$(YELLOW)[WARNING]$(END)
INFO = $(LIGTH)$(BLUE)[INFO]$(END)
ERROR = $(LIGTH)$(RED)[ERROR]$(END)

################################################################################
#                               VARIABLES                                      #
################################################################################

NAME := libasm
RMV = rm -rf

SOURCES := \
	ft_strlen.s

DIRECTORY_OBJ = .obj
DIRECTORY_DEP = .dep
OBJECTS = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES:.s=.o))
DEPENDENCIES = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES:.s=.d))
TOTAL_FILES = $(words $(SOURCES))

ASM := nasm -f elf64 -w+all -w+error -g
FLAGS_ASM := -MD -MF
LIB_ASM := ar rcs

CC := gcc -Wall -Werror -Wextra

DIRS_TO_CREATE = $(DIRECTORY_OBJ) $(DIRECTORY_DEP)

################################################################################
#                               MAKE RULES                                     #
################################################################################

all: dir $(OBJECTS) $(NAME)

help:
	@printf "$(INFO) Available rules:\n\n"
	@printf "  $(BLUE)make all$(END)        $(INFO) Build directories, compile objects and create the library\n"
	@printf "  $(BLUE)make dir$(END)        $(INFO) Create required directories (.obj and .dep)\n"
	@printf "  $(BLUE)make test$(END)       $(INFO) Show NASM command being used\n"
	@printf "  $(BLUE)make $(NAME)$(END)     $(INFO) Build the static library $(NAME).a\n"
	@printf "  $(BLUE)make main$(END)       $(INFO) Build test executable 'main' linking libasm\n"
	@printf "  $(BLUE)make clean$(END)      $(INFO) Remove object and dependency directories\n"
	@printf "  $(BLUE)make fclean$(END)     $(INFO) Run clean and also remove library and executable\n"
	@printf "  $(BLUE)make re$(END)         $(INFO) Rebuild everything from scratch\n"
	@printf "  $(BLUE)make help$(END)       $(INFO) Show this help message\n"

test:
	@echo "$(INFO) NASM command: $(ASM)"

dir:
	@for DIR in $(DIRS_TO_CREATE); do \
		if [ ! -d $$DIR ]; then \
			mkdir -p $$DIR; \
			printf "$(SUCCESS) Directory created: $$DIR\n"; \
		else \
			printf "$(INFO) Directory already exists: $$DIR\n"; \
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

$(NAME): $(OBJECTS)
	@printf "\n$(INFO) Creating static library $(NAME).a ... \n"
	@if $(LIB_ASM) $(NAME).a $(OBJECTS); then \
		printf "$(SUCCESS) Library created: $(NAME).a\n"; \
	else \
		printf "$(ERROR) Failed to create library\n"; \
	fi

main: main.c $(NAME)
	@printf "$(INFO) Building main executable ... "
	@if $(CC) main.c -L. -lasm -o main; then \
		printf "$(SUCCESS) Executable created: main\n"; \
	else \
		printf "$(ERROR) Failed to build main\n"; \
	fi

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

	@if [ -f main ]; then \
		$(RMV) main; \
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

COMPILED_FILES=0

-include $(DEPENDENCIES)

.SILENT:
.PHONY: all help test $(NAME) main clean fclean re