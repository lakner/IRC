# tool macros
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g3 -O0

# path macros
OBJ_PATH := obj
SRC_PATH := src
INC_PATH := include

# compile macros
TARGET := ircserv

# src files & obj files
SRC := $(shell find $(SRC_PATH) -name *.cpp)
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
CLEAN_LIST := $(TARGET) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

# non-phony targets
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(TARGET)
	@echo "make: *** [" $(TARGET) "] Success"

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_PATH) -c $< -o $@

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(OBJ_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@rm -f $(CLEAN_LIST)
	@echo "make: *** [" $(TARGET) "] Cleaned" $(CLEAN_LIST)
	@rm -rf $(OBJ_PATH)

.PHONY: fclean
fclean: clean
	@rm -f $(TARGET)


.PHONY: re
re: fclean makedir all

print-%: 
	@echo $* = $($*)
