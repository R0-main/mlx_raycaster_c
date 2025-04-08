# Main
NAME				=	mlx_raycatser_c
CC					=	clang

CFLAGS				=	-O3 -O2 -ffast-math -march=native -Ilibs/ft_libc/includes -Ilibs/minilibx_linux -Iincludes -fPIE -g
CMLX_FLAGS				= -lXext -lX11 -lm -lz
RM					=	rm -rf
MAKE				=	make --no-print-directory -C

# Colors
RESET				=	\033[0m
BLACK				=	\033[0;30m
RED					=	\033[0;31m
GREEN				=	\033[1;32m
YELLOW				=	\033[0;33m
BLUE				=	\033[0;34m
PURPLE				=	\033[1;35m
CYAN				=	\033[1;36m
WHITE				=	\033[0;37m

# Symbols
OK					=	✓
FAIL				=	✗
INFO				=	ⓘ

# Sources and Objects
FT_LIBC 			= ./libs/ft_libc/ft_libc.a
MLX_LIB 			= ./libs/minilibx_linux/libmlx_Linux.a
MLX_LIB_PATH 		= ./libs/minilibx_linux/

SRCS				=	src/main.c

SRC_COUNT			=	$(words $(SRCS))
OBJ_DIR				=	objects/main
OBJS				=	$(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

all : $(NAME)

$(NAME) : header $(FT_LIBC) $(OBJ_DIR) $(MLX_LIB)
		@cc $(CFLAGS) $(CMLX_FLAGS) $(OBJS) $(FT_LIBC) $(MLX_LIB) -o $(NAME)

$(OBJ_DIR)/%.o: %.c
		@mkdir -p $(dir $@)
		@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR): $(OBJS)
		@if [ ! -d "$(OBJ_DIR)" ]; \
			then mkdir -p "$(OBJ_DIR)"; \
		fi
		@if [ ! -e "$(NAME)" ]; then \
			printf "$(GREEN)Compiled $(OK)$(RESET)\n" ; \
		fi

$(FT_LIBC) :
	@if [ ! -e "$(shell dirname $@)" ]; then \
		git clone git@github.com:R0-main/ft_libc.git $(shell dirname $@); \
	fi;
	@$(MAKE) $(shell dirname $@) SAFE=0

$(MLX_LIB):
	@if [ ! -e "$($@)" ]; then \
		git clone https://github.com/42Paris/minilibx-linux.git $(shell dirname $@); \
		cd $(shell dirname $@) && ./configure && cd .. ;\
	fi;
	make re -C $(MLX_LIB_PATH) --no-print-directory

dev	 : clean-objs all tests
	valgrind --trace-children=yes --show-leak-kinds=all --leak-check=full -q ./$(NAME)

clean-objs :
		@$(RM) $(OBJ_DIR)
		@$(RM) $(NAME)

clean : clean-objs
		@$(MAKE) $(shell dirname $(FT_LIBC)) fclean
		@make clean -C $(MLX_LIB_PATH) --no-print-directory

fclean : clean
		@$(RM) $(NAME)

re : fclean all

.PHONY: all clean fclean re header tests clean-objs
