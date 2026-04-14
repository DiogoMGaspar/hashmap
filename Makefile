CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 \
         -I./src -I./tests/deps/unity/src \
         -MMD -MP

SRC_DIR = src
TEST_DIR = tests
DEPS_DIR = $(TEST_DIR)/deps
UNITY_DIR = $(DEPS_DIR)/unity
BUILD_DIR = test_build

SRC = $(SRC_DIR)/hashmap.c
TEST = $(TEST_DIR)/tests.c
UNITY_SRC = $(UNITY_DIR)/src/unity.c

OBJS = \
	$(BUILD_DIR)/hashmap.o \
	$(BUILD_DIR)/tests.o \
	$(BUILD_DIR)/unity.o

TARGET = $(BUILD_DIR)/test_runner

DEPS = $(OBJS:.o=.d)
-include $(DEPS)

all: $(TARGET)

$(UNITY_DIR):
	mkdir -p $(DEPS_DIR)
	git clone https://github.com/ThrowTheSwitch/Unity.git $(UNITY_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/hashmap.o: $(SRC_DIR)/hashmap.c | $(BUILD_DIR)
$(BUILD_DIR)/tests.o: $(TEST_DIR)/tests.c | $(BUILD_DIR)
$(BUILD_DIR)/unity.o: $(UNITY_SRC) | $(BUILD_DIR)

$(BUILD_DIR)/%.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(UNITY_DIR) $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -rf $(DEPS_DIR)

re: fclean all

.PHONY: all test clean fclean re