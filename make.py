#!python

import sys, os

scriptDir = os.path.dirname (os.path.abspath (sys.argv[0]))
os.chdir (scriptDir)

sys.path.append("build")
import makefile

goal = "all"
if len (sys.argv) > 1 :
  goal = sys.argv [1]

make = makefile.Make (goal, False)

uname = os.uname()[0]

sourceList = ["Heater.cpp", "Logger.cpp", "main.cpp", "mqtt.cpp", "Profiles.cpp", "TimeStamp.cpp"]
objectList = []
for source in sourceList:
  object = "objects/" + source + ".o"
  depObject = object + ".dep"
  objectList.append (object)
  rule = makefile.Rule ([object], "Compiling " + source)
  rule.deleteTargetFileOnClean()
  rule.mDependences.append (source)
  rule.mCommand.append ("c++")
  rule.mCommand += ["-std=c++17"]
  if uname == "Darwin":
    rule.mCommand += ["-I/opt/homebrew/opt/mosquitto/include", "-I/opt/homebrew/opt/nlohmann-json/include"]
  elif uname == "Linux":
    rule.mCommand += ["-Wno-psabi"]
  rule.mCommand += ["-c", source]
  rule.mCommand += ["-o", object]
  rule.mCommand += ["-MD", "-MP", "-MF", depObject]
  rule.enterSecondaryDependanceFile (depObject, make)
  make.addRule(rule)

linkOptions = []
if uname == "Darwin":
  linkOptions += ["-L/opt/homebrew/opt/mosquitto/lib"]
linkLib = ["-lmosquittopp"]
if uname == "Linux":
  linkLib += ["-lpthread"]

product = "ctrlheaters"
rule = makefile.Rule([product], "Linking " + product) # Release 2
rule.mDeleteTargetOnError = True
rule.deleteTargetFileOnClean()
rule.mDependences += objectList
rule.mCommand += ["c++"]
rule.mCommand += ["-o", product]
rule.mCommand += objectList
rule.mCommand += linkOptions
rule.mCommand += linkLib

make.addRule (rule)

# make.printRules()

make.addGoal ("all", [product], "Building all")
make.addGoal ("compile", objectList, "Compile C++ files")

# make.printGoals ()

make.runGoal(0,False)