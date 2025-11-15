SOURCES = opt.cpp cfg.cpp
OBJECTS = $(SOURCES:.cpp=.o)

CXXFLAGS = -g -MMD -MP

test: simple reassign

simple: opt
	bril2json < ../../examples/test/tdce/simple.bril | ./opt | brili -p

reassign: opt
	bril2json < ../../examples/test/tdce/reassign.bril | ./opt | brili -p

opt: $(OBJECTS)
	g++  $(OBJECTS) -o $@

clean:
	rm -f opt $(OBJECTS)

-include $(SOURCES:.cpp=.d)
