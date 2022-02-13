# ControleRadiateurs

MQTT client written in C++17 and designed to control a set of heaters driven by [FirmwareRadiateur](https://github.com/Koryphon/FirmwareRadiateur) running on dedicated boards.

## Used libraries

The libraries used are the following:

- ```libmosquitto```. Installed with mosquitto, the MQTT broker. To be installed with Homebrew : ```brew install mosquitto```.
- ```libmosquittopp```. C++ wrapper for libmosquitto. Also installed with mosquitto.
- ```nlohmann/json```. To be installed via Homebrew : ```brew install nlohmann-json```.

## Compilation

Python 2 or 3 is needed.

```sh
./make.py
```

## How it works

The policy to be applied for each heater is described in the file ``config.json``. At the root, we find the following keys:

- ```"location"```. The value is of type string and identifies the place of deployment. It will be displayed for information purposes when the application starts.
- ```"profiles"```. The value is a list of heating profiles over 24 hours. A heating profile key is of the form ```"hh:mm"``` and specifies the time at which a temperature is applied. The value is either a temperature in floating point number and in °C or a unique object with a single field having as key the duration and as value an array of 2 elements. The first element is the start temperature and the second the end temperature. The setpoint temperature will be a slope from the start temperature to the end temperature for the specified duration. After the interval, the end temperature is applied.
A profile can also be a reference to another profile (alias).
- ```"heaters"```. The value is a list of heaters with a key equal to their MQTT topic. [FirmwareRadiateur](https://github.com/Koryphon/FirmwareRadiateur) names the heaters ```heaterX``` where ```X``` is a number from 0 to 63 as given by the dip switch positions. The value of each heater can be the string ```"off"```, in which case the heater is not taken into account. Or it can be an object containing 
	- A key ```"profile"``` and a value referencing one of the profiles in the list ```"profiles"```.
	- A key ```"date"``` having for value an object containing a series of pairs (key, value) of type string where the key is a date ```yyyy-mm-dd hh:mm``` and the value the profile to apply from this date.
	- A key ```"offset"``` indicating the calibration offset of the heater sensor.
	- A key ```"power"``` indicating the power of the heater in Watts.
	- A key ```"location"``` giving the name of the room where the heater is installed.

### Sample profiles

The following piece of json:

```json
"profiles": {
    "room": {
        "00:00": 16.0,
        "05:00": {
        	"2:00": [ 16.0, 19.0 ]
        },
        "7:30": {
           "1:30": [ 19.0, 18.0 ]
        }
    }
}
```

defines a heating profile called ```"room"``` and fixes a temperature set point of 16°C from midnight to 5 am, then a slope going from 16°C to 19°C in two hours. From 7 am to 7:30 am, the temperature remains at 19°C. Then, from 7:30 am, a slope from 19°C to 18°C is applied. The temperature then remains at 18°C until midnight.

The following piece of json:

```json
"profiles": {
    "room": {
        "00:00": 16.0,
        "05:00": {
        	"2:00": [ 16.0, 19.0 ]
        },
        "7:30": {
           "1:30": [ 19.0, 18.0 ]
        }
    },
    "kitchen": "room"
}
```

adds a heating profile called ```"kitchen"``` which is an alias of the ```"room"``` profile.

### Examples of heater definitions

The following piece of json:

```json
"heaters": {
    "heater0": {
        "profile": "kitchen",
        "date": {
            "2022-02-11 11:10": "unused",
            "2022-02-15 04:00": "kitchen"
        }
    },
    "heater1": {
        "profile": "room"
    }
}
```

defines 2 heaters, ```"heater0"``` and ```"heater1"```. The first one uses the profile ```"kitchen"``` and the second one the profile ```"room"```. Between February 11, 2022 at 11:10 am and February 15, 2022 at 4 am, the profile of ```"heater0"``` becomes  ```"unused"``` (it corresponds to a setpoint temperature of 14°C). After February 15, 2022 at 4:00 a.m., the heater returns to the profile ```"kitchen"```.