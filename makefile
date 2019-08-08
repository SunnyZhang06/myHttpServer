CC := g++
CXXFLAGS := -std=c++11 -g -Wall -D_REENTRANT
INCLUDE := -I./
LIBS := -lpthread -lstdc++ 
TARGET := HttpServer

SOURCES := $(wildcard *.cpp)
OBJ := $(PATSUBST %.cpp, %.o, $(SOURCES))
# 把$(SOURCES)中变量后缀是cpp的全部替换成.o

$(TARGET):$(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

# 所有.cpp文件生成.o文件,$@表示所有目标集 	
%.o:%.cpp   
	$(CC) -c $(CXXFLAGS) $(INCLUDE) $< -o $@ 

.PHONY:clean

clean:
	rm -rf $(OBJ) $(TARGET)

	
 

  

