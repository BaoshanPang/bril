SOURCES = opt.cpp cfg.cpp instruction.cpp
OBJECTS = $(SOURCES:.cpp=.o)

CXXFLAGS = -g -MMD -MP -DLVN

test: simple reassign

simple: opt
	bril2json < ../../examples/test/tdce/simple.bril | ./opt | sed "/\/\/.*/d" | brili -p

reassign: opt
	bril2json < ../../examples/test/tdce/reassign.bril | ./opt | sed "/\/\/.*/d" | brili -p

opt: $(OBJECTS)
	g++  $(OBJECTS) -o $@

clean:
	rm -f opt $(OBJECTS)

-include $(SOURCES:.cpp=.d)
