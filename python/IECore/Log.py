##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

## \file Log.py
#
# Defines useful functions for logging of messages.
#
# \ingroup python

import os, sys, traceback
import inspect, string
from _IECore import *

## Creates and activates a LevelFilteredMessageHandler to filter log messages.
# Parameters:
# level: string with the log level. Valid values are: "Debug", "Info", "Warning" and "Error".
#        If the given level parameter is None, then it will try to get from the env. variable $IE_CORE_LOG_LEVEL.
#
# This function is called on IECore initialization.
# 
def initializeLog(level = None):
	if level is None:
		if os.environ.has_key("IE_CORE_LOG_LEVEL"):
			level = os.environ["IE_CORE_LOG_LEVEL"]
		else:
			# by default, log is disabled.
			level = "Warning"
	setLogLevelByName( level )

## Set the environment variable and the current LevelFilteredMessageHandler.
# Parameters:
# level: a string with the name of the log level as defined in MessageHandler.Level.
#
# This function sets the $IE_CORE_LOG_LEVEL environment variable, so child processes will inherit the log level.
# If the current message handler is also a LevelFilteredMessageHandler, this function pushes 
# it from the stack and register the new one.
# 
def setLogLevelByName( levelName ):
	msgLevel = None
	levels = Msg.Level.values.values()
	for levelType in levels:
		if levelName.lower() == levelType.name.lower():
			msgLevel = levelType
	
	if msgLevel is None:
		raise Exception, "Unrecognized Log level: " + str(levelName)

	setLogLevel( msgLevel )

## Set the environment variable and the current LevelFilteredMessageHandler.
# Parameters:
# level: MessageHandler.Level value.
#
# This function sets the $IE_CORE_LOG_LEVEL environment variable, so child processes will inherit the log level.
# If the current message handler is also a LevelFilteredMessageHandler, this function pushes 
# it from the stack and register the new one.
# 
def setLogLevel( level ):

	if os.environ.get("IE_CORE_LOG_LEVEL", "").lower() != level.name.lower():
		os.environ["IE_CORE_LOG_LEVEL"] = level.name

	current = MessageHandler.popHandler()
	# remove previous Level filters
	while isinstance(current, LevelFilteredMessageHandler):
		current = MessageHandler.popHandler()

	newFilter = LevelFilteredMessageHandler( current, level )
	MessageHandler.pushHandler( newFilter )
	debug("setLogLevel(", level, ")")

def __getCallContext(frame = None, withLineNumber = False):
	if frame is None:
		f = inspect.currentframe().f_back.f_back
	else:
		f = frame
	callStr = f.f_globals["__name__"]
	if withLineNumber:
		callStr += " #" + str(f.f_lineno)
	return callStr  

## Help function to track dificult errors.
# It prints the callstack giving the module name and the line number.
def showCallStack():

	f = inspect.currentframe().f_back.f_back
	index = 0
	callstack = "Callstack:\n"
	while not f is None:
		callstack += "> " + str(index) + ": " + f.f_globals["__name__"] + " #" + str(f.f_lineno) + "\n"
		f = f.f_back
		index += 1
	Msg.output(Msg.Level.Debug, __getCallContext( withLineNumber = True ), callstack )

## Sends debug messages to the current message handler and appends a full description of the catched exception.
# Parameters:
# Any string or object. They are converted to string and separated by space.
def debugException(*args):
	
	# same as debug
	stdStr = string.join(map(str, args), " ")

	(exceptionType, exception, trace) = sys.exc_info()
	etb = traceback.extract_tb(trace)
	exceptionType = "> " + str(exceptionType.__name__) + ": " + str(exception)
	exceptInfo = ""
	for (module, line, function, location) in etb:
		exceptInfo += ">  File " + str(module) + ", line " + str(line) + ", in " + str(function) + "\n>    " + str(location) + "\n"
	Msg.output(Msg.Level.Debug, __getCallContext(  withLineNumber = True  ), "[EXCEPTION CAPTURED] " + stdStr + "\n> Exception traceback:\n" + exceptInfo + exceptionType)

## Sends debug messages to the current message handler.
# Every message include information about the module and line number from where this function was called.
# Parameters:
# Any string or object. They are converted to string and separated by space.
def debug(*args):
	
	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Debug, __getCallContext( withLineNumber = True ), stdStr )
	
# Sends warning messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
def warning(*args):
	
	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Warning, __getCallContext(), stdStr )

# Sends info messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
def info(*args):
	
	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Info, __getCallContext(), stdStr )

# Sends error messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
def error(*args):
	
	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Error, __getCallContext(), stdStr )

__all__ = [ "initializeLog", "setLogLevelByName", "setLogLevel", "showCallStack",
		"debugException", "debug", "warning", "info", "error",
]
