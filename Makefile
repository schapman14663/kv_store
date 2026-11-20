CC = gcc
CFLAGS = -Wall -Wextra -I$(INC)
LDFLAGS =
LDLIBS =

NAME	=	main
BIN = bin
INC = inc
OBJ = obj
SRC = src
TEST = tests

EXEC = $(BIN)/$(NAME)
EXEC_TEST = $(BIN)/test_runner

MAIN_SRC = $(SRC)/main.c
LIB_SRCS = $(filter-out $(MAIN_SRC), $(wildcard $(SRC)/*.c))
TEST_SRCS = $(wildcard $(TEST)/*.c)

MAIN_OBJ = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(MAIN_SRC))
LIB_OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(LIB_SRCS))
TEST_OBJS = $(patsubst $(TEST)/%.c,$(OBJ)/test_%.o,$(TEST_SRCS))

DEPS = $(MAIN_OBJ:.o=.d) $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

all: $(EXEC)

# Link main executable
$(EXEC): $(MAIN_OBJ) $(LIB_OBJS) | $(BIN)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Link test executable
$(EXEC_TEST): $(TEST_OBJS) $(LIB_OBJS) | $(TEST)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile src/ files
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Compile tests/ files
$(OBJ)/test_%.o: $(TEST)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Create output directories if missing
$(BIN) $(OBJ):
	mkdir -p $@

run: $(EXEC)
	./$<

test: $(EXEC_TEST)
	./$<

clean:
	@rm -rf $(BIN) $(OBJ)

watch:
	find $(SRC) $(INC) $(TEST) -type f | entr -c make test

-include $(DEPS)

.PHONY: all run test watch clean
