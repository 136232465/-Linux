CC = g++
prom = server
deps = $(shell find ./ -name "*.h")
src = $(shell find ./ -name "*.cpp")
obj = $(src:%.cpp=%.o)

$(prom): $(obj)
	$(CC) -o $(prom) $(obj)

%.o:%.cpp $(deps)
	$(CC) -c $< -o $@



clean:
	rm -rf $(prom) $(obj)
