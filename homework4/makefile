# Source and target
SRCS = main.cpp
TARGET_BASE = raylib_app

# Add the CS3113 library if it exists
ifeq ($(wildcard CS3113/cs3113.cpp),CS3113/cs3113.cpp)
    SRCS += CS3113/cs3113.cpp
endif

# Add the Entity library if it exists
ifeq ($(wildcard CS3113/Entity.cpp),CS3113/Entity.cpp)
    SRCS += CS3113/Entity.cpp
endif

# Add the Map library if it exists
ifeq ($(wildcard CS3113/Entity.cpp),CS3113/Entity.cpp)
    SRCS += CS3113/Map.cpp
endif

# Default compiler settings
CXX = g++
CXXFLAGS = -std=c++11

# Platform-safe remove command (Windows cmd vs POSIX)
ifeq ($(OS),Windows_NT)
    RM = del /Q
else
    RM = rm -f
endif

# Default exec name (overridden per-OS below)
TARGET = $(TARGET_BASE)

ifeq ($(OS),Windows_NT)
    # Windows (MinGW) - custom raylib in C:/raylib
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET = $(TARGET_BASE).exe
    EXEC = .\$(TARGET)
else ifeq ($(shell uname -s),Darwin)
    # macOS
    RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
    RAYLIB_LIBS   = $(shell pkg-config --libs raylib)
    CXXFLAGS += -arch arm64 $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = ./$(TARGET)
else
    # Linux / WSL
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC = ./$(TARGET)
endif

# Build rule
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Clean rule (delete exactly the actual output)
clean:
	-$(RM) $(TARGET)

# Run rule
run: $(TARGET)
	$(EXEC)
