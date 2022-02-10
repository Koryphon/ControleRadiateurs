UNAME := $(shell uname)
LIBS := -lmosquittopp

ifeq ($(UNAME), Linux)
  CCFLAGS += -Wno-psabi
  LIBS += -lpthread
endif
ifeq ($(UNAME), Darwin)
  CCFLAGS += -I/opt/homebrew/opt/mosquitto/include -I/opt/homebrew/opt/nlohmann-json/include -L/opt/homebrew/opt/mosquitto/lib
endif

ctrlheaters: Heater.cpp mqtt.cpp Profiles.cpp TimeStamp.cpp Logger.cpp main.cpp
	c++ -o ctrlheaters -std=c++17 $(CCFLAGS) Heater.cpp mqtt.cpp Profiles.cpp TimeStamp.cpp Logger.cpp main.cpp $(LIBS)
