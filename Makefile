UNAME := $(shell uname)
LIBS := -lmosquittopp
CFLAGS := -std=c++17

ifeq ($(UNAME), Linux)
  CCFLAGS += -Wno-psabi
  LIBS += -lpthread
endif
ifeq ($(UNAME), Darwin)
  CCFLAGS += -I/opt/homebrew/opt/mosquitto/include -I/opt/homebrew/opt/nlohmann-json/include
  LDFLAGS += -L/opt/homebrew/opt/mosquitto/lib
endif

%.o: %.c Heater.h Cfg.h Logger.h Profiles.h TimeStamp.h Util.h mqtt.h
	c++ -c -o $@ $(CCFLAGS) $<

ctrlheaters: Heater.o mqtt.o Profiles.o TimeStamp.o Logger.o main.o
	c++ -o ctrlheaters $(LDFLAGS) Heater.o mqtt.o Profiles.o TimeStamp.o Logger.o main.o $(LIBS)
