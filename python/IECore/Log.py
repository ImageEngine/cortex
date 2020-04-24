##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

import os, sys, traceback
import inspect, string
import warnings
import IECore

## Set the environment variable and the current LevelFilteredMessageHandler.
# Parameters:
# level: a string with the name of the log level as defined in MessageHandler.Level.
#
# This function sets the $IECORE_LOG_LEVEL environment variable, so child processes will inherit the log level.
# If the current message handler is also a LevelFilteredMessageHandler, this function pushes
# it from the stack and register the new one.
#
## \ingroup python
def setLogLevelByName( levelName ):

	IECore.setLogLevel( IECore.MessageHandler.stringAsLevel( levelName ) )

## Set the environment variable and the current LevelFilteredMessageHandler.
# Parameters:
# level: MessageHandler.Level value.
#
# This function sets the $IECORE_LOG_LEVEL environment variable, so child processes will inherit the log level.
# If the current message handler is also a LevelFilteredMessageHandler, this function pushes
# it from the stack and register the new one.
## \ingroup python
def setLogLevel( level ):

	assert( isinstance( level, IECore.MessageHandler.Level ) and level!=IECore.MessageHandler.Level.Invalid )

	os.environ["IECORE_LOG_LEVEL"] = IECore.MessageHandler.levelAsString( level )

	current = IECore.MessageHandler.currentHandler()
	if not isinstance( current, IECore.LevelFilteredMessageHandler ) :
		IECore.msg( IECore.Msg.Level.Warning, "IECore.setLogLevel", "Failed to set log level - current handler is not a LevelFilteredMessageHandler" )
		return

	current.setLevel( level )

	IECore.debug("setLogLevel(", level, ")")

def __getCallStr(frame):
	return frame.f_globals.get("__name__", frame.f_globals.get("__file__", "N/A"))

def __getCallContext(frame = None, withLineNumber = False):
	if frame is None:
		f = inspect.currentframe().f_back.f_back
	else:
		f = frame
	callStr = __getCallStr(f)
	if withLineNumber:
		callStr += " #" + str(f.f_lineno)
	return callStr

## Help function to track dificult errors.
# It prints the callstack giving the module name and the line number.
## \ingroup python
def showCallStack():

	f = inspect.currentframe().f_back.f_back
	index = 0
	callstack = "Callstack:\n"
	while not f is None:
		callstack += "> " + str(index) + ": " + __getCallStr(f) + " #" + str(f.f_lineno) + "\n"
		f = f.f_back
		index += 1
	IECore.Msg.output(IECore.Msg.Level.Debug, __getCallContext( withLineNumber = True ), callstack )

## Use this function to get information about the context where the exception happened.
# Returns a tuple of strings (location, stack trace) for the captured exception.
## \ingroup python
def exceptionInfo():
	(exceptionType, exception, trace) = sys.exc_info()
	etb = traceback.extract_tb(trace)
	exceptionType = str(exceptionType.__name__) + ": " + str(exception)
	exceptInfo = ""
	for (module, line, function, location) in etb:
		exceptInfo += " File " + str(module) + ", line " + str(line) + ", in " + str(function) + "\n>    " + str(location) + "\n"
	return ( __getCallContext(  withLineNumber = True  ), "Exception traceback:\n" + exceptInfo + exceptionType)

## Sends debug messages to the current message handler and appends a full description of the catched exception.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def debugException(*args):

	# same as debug
	stdStr = string.join(map(str, args), " ")

	(exceptionType, exception, trace) = sys.exc_info()
	etb = traceback.extract_tb(trace)
	exceptionType = "> " + str(exceptionType.__name__) + ": " + str(exception)
	exceptInfo = ""
	for (module, line, function, location) in etb:
		exceptInfo += ">  File " + str(module) + ", line " + str(line) + ", in " + str(function) + "\n>    " + str(location) + "\n"
	IECore.Msg.output(IECore.Msg.Level.Debug, __getCallContext(  withLineNumber = True  ), "[EXCEPTION CAPTURED] " + stdStr + "\n> Exception traceback:\n" + exceptInfo + exceptionType)

## Sends debug messages to the current message handler.
# Every message include information about the module and line number from where this function was called.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def debug(*args):

	stdStr = " ".join(map(str, args))
	IECore.Msg.output(IECore.Msg.Level.Debug, __getCallContext( withLineNumber = True ), stdStr )

# Sends warning messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def warning(*args):

	stdStr = string.join(map(str, args), " ")
	IECore.Msg.output(IECore.Msg.Level.Warning, __getCallContext(), stdStr )

# Sends info messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def info(*args):

	stdStr = string.join(map(str, args), " ")
	IECore.Msg.output(IECore.Msg.Level.Info, __getCallContext(), stdStr )

# Sends error messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def error(*args):

	stdStr = string.join(map(str, args), " ")
	IECore.Msg.output(IECore.Msg.Level.Error, __getCallContext(), stdStr )

__all__ = [ "setLogLevelByName", "setLogLevel", "showCallStack",
		"exceptionInfo", "debugException", "debug", "warning", "info", "error",
]
