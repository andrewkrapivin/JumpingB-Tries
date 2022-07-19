CXX = g++
#CXXFLAGS = -MMD -MP -pthread -march=native -std=c++20 -O0 -ggdb -Wall -W
CXXFLAGS = -MMD -MP -pthread -march=native -std=c++20 -Ofast -Wall -W
 
SRCDIR = src
OBJDIR = bin
TESTDIR = test
TARGET = $(OBJDIR)/test


src = $(wildcard $(SRCDIR)/*.cpp)
srch = $(wildcard $(SRCDIR)/*.hpp)
obj = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(src))
srctest = $(wildcard $(TESTDIR)/*.cpp)
objtest = $(patsubst $(TESTDIR)/%.cpp, $(TESTDIR)/%.o, $(srctest))
dep = $(patsubst %.cpp,%.d,$(src))

all:
	mkdir -p bin \
	
	make $(TARGET)

test:
	mkdir -p bin \
	
	make $(TARGET)

$(TARGET): test/test.o $(obj) $(srch)
	$(CXX) $(CXXFLAGS) $^ -o $@

test/test.o: test/test.cpp $(srch)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp Makefile $(srch)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(dep) 

.PHONY: clean prints
clean:
	rm -r -f bin \
	
	rm -f $(obj) $(TARGET) $(dep) $(objtest)

prints:
	echo '$(OBJDIR)'
	echo '$(src) $(obj)'
	echo '$(objtest)'

