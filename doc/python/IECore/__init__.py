
## \defgroup python Python functionality
#
# Some parts of the IECore library are defined purely in Python. These are shown below.

# We register our own IntrusivePtrToPython converter in the bindings based on RefCountedClass. This allows
# us to deal with object identity issues when pushing wrapped objects back into python. Boost python issues a warning
# about this as it has already registered a converter, so we ignore the warnings here.
import warnings
warnings.filterwarnings( "ignore", "to-Python converter for IECore::IntrusivePtr.*already registered.", RuntimeWarning )

from _IECore import *

# access by a shorter name for convenience
Msg = MessageHandler
from registerRunTimeTyped import registerRunTimeTyped


import IECore

## Registers a type id for an extension class. This makes TypeId.className
# available and also checks that no other type is trying to use the same id.
# It raises a RuntimeError if a conflicting type is already registered.
def __registerTypeId( typeId, typeName, baseTypeId ) :

	assert( type( typeId ) is IECore.TypeId )
	assert( type( typeName ) is str )
	assert( type( baseTypeId ) is IECore.TypeId )

	# check this type hasn't been registered already
	if hasattr( IECore.TypeId, typeName ):
		if getattr( IECore.TypeId, typeName ) != typeId:
			raise RuntimeError( "Type \"%s\" is already registered." % typeName )

		return

	if typeId in IECore.TypeId.values :
		raise RuntimeError( "TypeId \"%d\" is already registered as \"%s\"." % (typeId, IECore.TypeId.values[typeId] ) )

	# update the TypeId enum
	setattr( IECore.TypeId, typeName, typeId )
	IECore.TypeId.values[ int( typeId ) ] = typeId

	# register the new type id
	IECore.RunTimeTyped.registerType( typeId, typeName, baseTypeId )

__nextDynamicRunTimeTypedId = None

## This function adds the necessary function definitions to a python
# class for it to properly implement the RunTimeTyped interface. It should
# be called once for all python classes inheriting from RunTimeTyped. It also
# calls registerTypeId() for you.
# baseClass is deprecated.
# \todo Remove deprecation warning and baseClass parameter on Cortex 6.
# typId is optional and if not defined, this function will associate a dynamic Id
#       in the range FirstDynamicTypeId and LastDynamicTypeId from TypeIds.h.
#       It's necessary to specify type Id for Object derived class or anything that
#       is serializable.
def registerRunTimeTyped( typ, typId = None, baseClass = None ) :

	if not baseClass is None :
		IECore.warning( "%s: Passing base class is deprecated in registerRunTimeTyped." % typ )

	typeName = typ.__name__
	runTypedBaseClass = filter( lambda c: issubclass( c, IECore.RunTimeTyped ), typ.__bases__ )[0]

	# constants below are the same as in TypeIds.h
	FirstDynamicTypeId = 300000
	LastDynamicTypeId = 399999

	# check if overwritting registration.
	if not hasattr( IECore.TypeId, typeName ) :

		if typId is None :

			global __nextDynamicRunTimeTypedId

			if __nextDynamicRunTimeTypedId is None :
				__nextDynamicRunTimeTypedId = FirstDynamicTypeId
			elif __nextDynamicRunTimeTypedId > LastDynamicTypeId:
				raise Exception, "Too many dynamic RunTimeTyped registered classes! You must change TypeIds.h and rebuild Cortex."

			typId = __nextDynamicRunTimeTypedId

			__nextDynamicRunTimeTypedId += 1

		__registerTypeId( IECore.TypeId( typId ), typeName, IECore.TypeId( runTypedBaseClass.staticTypeId() ) )

	else :
		# check if the new type Id is compatible with the previously registered one.
		prevTypId = getattr( IECore.TypeId, typeName )
		if prevTypId in xrange( FirstDynamicTypeId, LastDynamicTypeId+1 ) :
			if not typId is None :
				raise Exception, "Trying to set a type ID for %s previously registered as a dynamic type Id!" % typeName
		else :
			if typId is None :
				raise Exception, "Trying to re-register type %s as dynamic type Id!" % typeName
			elif typId != prevTypId :
				raise Exception, "Trying to re-register %s under different type Id: %s != %s" % ( typeName, str(typId), prevTypId )
		# necessary when the typeid is defined in IECore/TypeIds.h and bound in TypeIdBinding.cpp, but then
		# the class for that typeid is implemented in python (currently ClassParameter does this).
		if IECore.RunTimeTyped.typeNameFromTypeId( prevTypId )=="" :
			IECore.RunTimeTyped.registerType( prevTypId, typeName, IECore.TypeId( runTypedBaseClass.staticTypeId() ) )

	# Retrieve the correct value from the enum
	tId = getattr( IECore.TypeId, typeName )

	# add the typeId and typeName method overrides
	typ.typeId = lambda x : tId
	typ.typeName = lambda x: typeName

	# add the staticTypeId, staticTypeName, baseTypeId, and baseTypeName overrides
	typ.staticTypeId = staticmethod( lambda : tId )
	typ.staticTypeName = staticmethod( lambda : typeName )
	typ.baseTypeId = staticmethod( lambda : runTypedBaseClass.staticTypeId() )
	typ.baseTypeName = staticmethod( lambda : runTypedBaseClass.staticTypeName() )

	# add the inheritsFrom method override
	def inheritsFrom( t, baseClass ) :

		if type( t ) is str :
			if type( baseClass ) is list :
				for base in baseClass :
					if base.staticTypeName() == t :
						return True
			else:
				if baseClass.staticTypeName() == t :
					return True
		elif type(t) is IECore.TypeId :
			if type( baseClass ) is list :
				for base in baseClass :
					if base.staticTypeId() == t :
						return True
			else:
				if baseClass.staticTypeId() == t :
					return True
		else:
			raise TypeError( "Invalid type specifier ( %s )" % str( t ) )

		if type( baseClass ) is list :
			for base in baseClass:
				if base.inheritsFrom( t ):
					return True
		else:
			return baseClass.inheritsFrom( t )

		return False

	typ.inheritsFrom = staticmethod( lambda t : inheritsFrom( t, runTypedBaseClass ) )


	# add the isInstanceOf method override
	def isInstanceOf( self, t, baseClass ) :

		if type( t ) is str :
			if self.staticTypeName() == t :
				return True
		elif type( t ) is IECore.TypeId :
			if self.staticTypeId() == t :
				return True
		else :
			raise TypeError( "Invalid type specifier ( %s )" % str( t ) )

		return inheritsFrom( t, baseClass )

	typ.isInstanceOf = lambda self, t : isInstanceOf( self, t, runTypedBaseClass )

from registerObject import registerObject


import IECore

def registerObject( typ, typId, baseClass = None ) :

	if not issubclass( typ, IECore.Object ) :
		raise TypeError, "registerObject called with non-Object class: %s" % typ

	IECore.registerRunTimeTyped( typ, typId, baseClass )
	IECore.Object.registerType( typ.staticTypeId(), typ.staticTypeName(), typ )

from Log import *


import os, sys, traceback
import inspect, string
import warnings
from IECore import *

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

	setLogLevel( MessageHandler.stringAsLevel( levelName ) )

## Set the environment variable and the current LevelFilteredMessageHandler.
# Parameters:
# level: MessageHandler.Level value.
#
# This function sets the $IECORE_LOG_LEVEL environment variable, so child processes will inherit the log level.
# If the current message handler is also a LevelFilteredMessageHandler, this function pushes
# it from the stack and register the new one.
## \ingroup python
def setLogLevel( level ):

	assert( isinstance( level, MessageHandler.Level ) and level!=MessageHandler.Level.Invalid )

	os.environ["IECORE_LOG_LEVEL"] = MessageHandler.levelAsString( level )

	current = MessageHandler.currentHandler()
	if not isinstance( current, LevelFilteredMessageHandler ) :
		msg( Msg.Level.Warning, "IECore.setLogLevel", "Failed to set log level - current handler is not a LevelFilteredMessageHandler" )
		return

	current.setLevel( level )
	
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
## \ingroup python
def showCallStack():

	f = inspect.currentframe().f_back.f_back
	index = 0
	callstack = "Callstack:\n"
	while not f is None:
		callstack += "> " + str(index) + ": " + f.f_globals["__name__"] + " #" + str(f.f_lineno) + "\n"
		f = f.f_back
		index += 1
	Msg.output(Msg.Level.Debug, __getCallContext( withLineNumber = True ), callstack )

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
	Msg.output(Msg.Level.Debug, __getCallContext(  withLineNumber = True  ), "[EXCEPTION CAPTURED] " + stdStr + "\n> Exception traceback:\n" + exceptInfo + exceptionType)

## Sends debug messages to the current message handler.
# Every message include information about the module and line number from where this function was called.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def debug(*args):

	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Debug, __getCallContext( withLineNumber = True ), stdStr )

# Sends warning messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def warning(*args):

	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Warning, __getCallContext(), stdStr )

# Sends info messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def info(*args):

	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Info, __getCallContext(), stdStr )

# Sends error messages to the current message handler.
# Parameters:
# Any string or object. They are converted to string and separated by space.
## \ingroup python
def error(*args):

	stdStr = string.join(map(str, args), " ")
	Msg.output(Msg.Level.Error, __getCallContext(), stdStr )

__all__ = [ "setLogLevelByName", "setLogLevel", "showCallStack",
		"exceptionInfo", "debugException", "debug", "warning", "info", "error",
]

from Formatter import Formatter


## The Formatter class defines an interface for specifying
# simple structured output without having to be concerned
# with exactly how that output is displayed.
#
# \ingroup python
class Formatter :

	def heading( self, name ) :

		raise NotImplementedError

	def paragraph( self, name ) :

		raise NotImplementedError

	def indent( self ) :

		raise NotImplementedError

	def unindent( self ) :

		raise NotImplementedError



from WrappedTextFormatter import WrappedTextFormatter


import Formatter
import StringUtil
import sys

## Implements the Formatter interface in a very simple form, just outputting
# text with word wrapping to a file object.
#
# \ingroup python
class WrappedTextFormatter( Formatter.Formatter ) :

	def __init__( self, outputFile, wrapWidth = 80 ) :

		self.__file = outputFile
		self.__wrapWidth = wrapWidth
		self.__indentation = 0
		self.__numNewLines = -1

	def heading( self, name ) :

		name = name.strip()

		self.__blankLine()
		self.__indent()
		self.__output( str( name ) + "\n" )
		self.__indent()
		self.__output( "".rjust( len( name ), "-" ) + "\n\n" )

		return self

	def paragraph( self, text ) :

		self.__blankLine()
		lines = StringUtil.wrap( str( text ).rstrip(), self.__wrapWidth ).split( "\n" )
		for line in lines :
			self.__indent()
			self.__output( line + "\n" )

	def indent( self ) :

		self.__indentation += 1
		return self

	def unindent( self ) :

		self.__indentation -= 1
		return self

	def __output( self, text ) :

		self.__file.write( text )
		self.__numNewLines = 0
		while self.__numNewLines < len( text ) :
			#sys.stdout.write( "("+text[(-1-self.__numNewLines)]+")" )
			if text[(-1-self.__numNewLines)]=='\n' :
				self.__numNewLines += 1
			else :
				break

		#sys.stdout.write( "OUTPUT " + str( self.__numNewLines ) + "\n" )

	def __indent( self ) :

		self.__output( "".rjust( self.__indentation * 4 ) )

	def __blankLine( self ) :

		if self.__numNewLines == -1 :
			return

		for i in range( self.__numNewLines, 2 ) :
			self.__file.write( "\n" )

import StringUtil
from DataTraits import *


import string
import datetime
import _IECore as IECore
from Log import *

## Utility function that recognizes objects that are simple types. That means,
# have one single value on it and are not IMath data types.
# VectorData, CompoundData and IMath-type Data are not simple.
# Returns a boolean indicating if the given object is a simple Data.
## \ingroup python
def isSimpleDataType(obj):

	if not isinstance(obj, IECore.Data):
		return False
	if hasattr(obj.__class__, "value"):
		objClass = type(obj)
		info = __dataTypesConversionDict[objClass]
		if info is None:
			return False
		# check if the element type a basic python type.
		if info[0] in [bool, int, long, str, float]:
			return True
	return False

##	Utility function that recognizes objects that are numeric simple types. That means,
#	have one single value on it, and it is numeric. For example, IntData. But not
#	IntVectorData, nor CharData which just accepts characters (up to now).
## \ingroup python
def isSimpleNumericDataType(obj):
	if not isinstance(obj, IECore.Data):
		return False
	if hasattr(obj.__class__, "value"):
		objClass = type(obj)
		info = __dataTypesConversionDict[objClass]
		if info is None:
			return False
		# check if the element type a basic python type.
		if info[0] in [int, long, float]:
			return True
	return False

## Utility function that recognizes objects that hold values as matrices.
# For example the IMath types: V2f, M33f, Color3f. But not vectors of those types.
## \ingroup python
def isMatrixDataType(obj):

	if not hasattr(obj.__class__, "value"):
		return False
	##\ todo: this attr doesn't guarantee a matrix or not. Quats and transformation matrices don't have dimension attrs
	if hasattr(obj.value.__class__, "dimensions"):
		# vectors, colors and matrices
		return True
	return False

## Utility function that returns ``True`` if a Data object obj could be created with a dict.
#
## \ingroup python
def isMappingDataType(obj):
	objClass = type(obj)
	info = __dataTypesConversionDict.get(objClass, None)
	if info is None:
		return False
	if info[0] is dict:
		return True
	return False

## Utility function that returns ``True`` if a Data object obj could be created from a list.
#
## \ingroup python
def isSequenceDataType(obj):
	objClass = type(obj)
	info = __dataTypesConversionDict.get(objClass, None)
	if info is None:
		return False
	if info[0] is list:
		return True
	return False

# map Data types => (element type, instantiate from element flag, [indexed element type])
__dataTypesConversionDict = {
	IECore.Data: None,
	IECore.BoolData: (bool, True),
	IECore.IntData: (int, True),
	IECore.UIntData: (int, False),
	IECore.CharData: (str, False),
	IECore.UCharData: (int, False),
	IECore.StringData: (str, True),
	IECore.FloatData: (float, False),
	IECore.DoubleData: (float, True),
	IECore.HalfData: (float, False),
	IECore.ShortData: (int, False),
	IECore.UShortData: (int, False),
	IECore.Int64Data: (int, False),
	IECore.UInt64Data: (int, False),


	IECore.V2fData: (IECore.V2f, True),
	IECore.V2dData: (IECore.V2d, True),
	IECore.V2iData: (IECore.V2i, True),
	IECore.V3iData: (IECore.V3i, True),
	IECore.V3fData: (IECore.V3f, True),
	IECore.V3dData: (IECore.V3d, True),
	IECore.QuatfData: (IECore.Quatf, True),
	IECore.QuatdData: (IECore.Quatd, True),
	IECore.Color3fData: (IECore.Color3f, True),
	IECore.Color3dData: (IECore.Color3d, True),
	IECore.Color4fData: (IECore.Color4f, True),
	IECore.Color4dData: (IECore.Color4d, True),
	IECore.Box2iData: (IECore.Box2i, True),
	IECore.Box3iData: (IECore.Box3i, True),
	IECore.Box2fData: (IECore.Box2f, True),
	IECore.Box2dData: (IECore.Box2d, True),
	IECore.Box3fData: (IECore.Box3f, True),
	IECore.Box3dData: (IECore.Box3d, True),
	IECore.M33fData: (IECore.M33f, True),
	IECore.M33dData: (IECore.M33d, True),
	IECore.M44fData: (IECore.M44f, True),
	IECore.M44dData: (IECore.M44d, True),

	IECore.BoolVectorData: ( list, False, bool),
	IECore.CharVectorData: (list, False, str),
	IECore.UCharVectorData: (list, False, int),
	IECore.IntVectorData: (list, False, int),
	IECore.UIntVectorData: (list, False, int),
	IECore.HalfVectorData: (list, False, float),
	IECore.FloatVectorData: (list, False, float),
	IECore.DoubleVectorData: (list, False, float),
	IECore.StringVectorData: (list, False, str),
	IECore.ShortVectorData: (list, False, int),
	IECore.UShortVectorData: (list, False, int),
	IECore.Int64VectorData: (list, False, int),
	IECore.UInt64VectorData: (list, False, int),
	IECore.V2fVectorData: (list, False, IECore.V2f),
	IECore.V2dVectorData: (list, False, IECore.V2d),
	IECore.V2iVectorData: (list, False, IECore.V2i),
	IECore.V3fVectorData: (list, False, IECore.V3f),
	IECore.V3dVectorData: (list, False, IECore.V3d),
	IECore.V3iVectorData: (list, False, IECore.V3i),
	IECore.QuatfVectorData: (list, False, IECore.Quatf),
	IECore.QuatdVectorData: (list, False, IECore.Quatd),
	IECore.Box2iVectorData: (list, False, IECore.Box2i),
	IECore.Box2fVectorData: (list, False, IECore.Box2f),
	IECore.Box2dVectorData: (list, False, IECore.Box2d),
	IECore.Box3iVectorData: (list, False, IECore.Box3i),
	IECore.Box3fVectorData: (list, False, IECore.Box3f),
	IECore.Box3dVectorData: (list, False, IECore.Box3d),
	IECore.M33fVectorData: (list, False, IECore.M33f),
	IECore.M33dVectorData: (list, False, IECore.M33d),
	IECore.M44fVectorData: (list, False, IECore.M44f),
	IECore.M44dVectorData: (list, False, IECore.M44d),
	IECore.Color3fVectorData: (list, False, IECore.Color3f),
	IECore.Color3dVectorData: (list, False, IECore.Color3d),
	IECore.Color4fVectorData: (list, False, IECore.Color4f),
	IECore.Color4dVectorData: (list, False, IECore.Color4d),

	IECore.CompoundData: (dict, True, None),
	IECore.CompoundDataBase: (dict, True, None),

	IECore.TransformationMatrixfData: ( IECore.TransformationMatrixf, True ),
	IECore.TransformationMatrixdData: ( IECore.TransformationMatrixd, True ),

	IECore.LineSegment3fData: ( IECore.LineSegment3f, True ),
	IECore.LineSegment3dData: ( IECore.LineSegment3d, True ),

	IECore.SplineffData: ( IECore.Splineff, True ),
	IECore.SplineddData: ( IECore.Splinedd, True ),
	IECore.SplinefColor3fData: ( IECore.SplinefColor3f, True ),
	IECore.SplinefColor4fData: ( IECore.SplinefColor4f, True ),

	IECore.CubeColorLookupfData: ( IECore.CubeColorLookupf, True ),
	IECore.CubeColorLookupdData: ( IECore.CubeColorLookupd, True ),

	IECore.DateTimeData: ( datetime.datetime, True ),
	IECore.TimeDurationData: ( datetime.timedelta, True ),
	IECore.TimePeriodData: ( IECore.TimePeriod, True ),

	IECore.SmoothSkinningData: None,
	
}

## Function that returns a list of Data derived classes.
## \ingroup python
def getDataDerivedTypes():

	dataTypesList = __dataTypesConversionDict.keys()
	dataTypesList.remove(IECore.Data)
	return dataTypesList

## Returns the type (class) for the element type hold on the instances of the given Data type. In fact, returns the class that is
# used to instantiate the Data type.
# For example, if the Data class returns true from isSequenceType(), then this function returns the type 'list'.
# See also: valueTypeFromSequenceType()
## \ingroup python
def elementTypeFromDataType(dataType):

	dataInfo = __dataTypesConversionDict[dataType]
	if dataInfo is None:
		raise TypeError, "This Data class can not be instantiated."
	return dataInfo[0]

## Returns the type (class) used on each indexed value on the given sequence type.
# For example: it returns 'int' for the IntVectorData class.
## \ingroup python
def valueTypeFromSequenceType(sequenceType):

	dataInfo = __dataTypesConversionDict[sequenceType]
	if dataInfo is None:
		raise TypeError, "This Data class can not be instantiated."
	if len(dataInfo) < 3:
		raise TypeError, "This Data class is not a sequence type!"
	return dataInfo[2]

## Returns the Data class that is instantiable given an element type.
## \ingroup python
def dataTypeFromElementType(elementType):

	for (dataType, value) in __dataTypesConversionDict.items():
		if value is None:
			continue
		if value[0] is elementType and value[1]:
			return dataType
	raise TypeError, "No Data type is compatible with the given element type: %s" % ( elementType )

## Returns the Data class that is instantiable given a element data object.
# It also instantiate container Data objects, like VectorData and CompoundData, given the proper list and dict.
## \ingroup python
def dataTypeFromElement(element):

	# threat the VectorData type exception...
	if isinstance(element, list) and len(element) > 0:
		# We have to check the list contents.
		elementValueType = type(element[0])
		for (dataType, value) in __dataTypesConversionDict.items():
			if value[0] is list and len(value) >= 2 and value[2] is elementValueType:
				return dataType

	return dataTypeFromElementType(type(element))

## Factory function for Data objects given its element data.
# Parameters:
# obj  -  any IECore simple structure objects (Color3f, V3f, ...) or python simple types (int, str, list, dict...)
# Returns:
# A Data object that holds the given element data object.
## \ingroup python
def dataFromElement(element):

	# An empty list or empty set is ambiguous - we don't know if it should be a StringVectorData, IntVectorData, or anything
	if element == [] or element == set() :

		raise RuntimeError, "Cannot determine Data type for ambiguous element: %s" % ( str( element ) )

	dataType = dataTypeFromElement(element)
	return dataType(element)

__all__ = [ "isSimpleDataType", "isSimpleNumericDataType", "isMatrixDataType", "isMappingDataType",
	"isSequenceDataType", "getDataDerivedTypes", "elementTypeFromDataType", "valueTypeFromSequenceType",
	"dataTypeFromElementType", "dataTypeFromElement", "dataFromElement",
]

from FileSequenceFunctions import *


import re
import os
import re
import glob
import shutil
import os.path
import _IECore as IECore

# This is here because we can't yet create a to_python converter for boost::regex
IECore.FileSequence.fileNameValidator = staticmethod( lambda : re.compile( "^([^#]*)(#+)([^#]*)$" ) )

## Moves the set of files specified by sequence1 to the set of files
# specified by sequence2, where sequence1 and sequence2 are
# FileSequence objects of equal length. This function is safe even if the
# files specified by each sequence overlap.
## \ingroup python
def mv( sequence1, sequence2 ) :

	if __sequencesClash( sequence1, sequence2 ) :
		sTmp = sequence1.copy()
		sTmp.setPrefix( os.path.join( os.path.dirname( sTmp.getPrefix() ), __tmpPrefix() ) )
		for src, dst in sequence1.mapTo( sTmp, True ) :
			shutil.move( src, dst )
		for src, dst in sTmp.mapTo( sequence2, True ) :
			shutil.move( src, dst )
	else :
		for src, dst in sequence1.mapTo( sequence2, True ) :
			shutil.move( src, dst )

## Copies the set of files specified by sequence1 to the set of files
# specified by sequence2, where sequence1 and sequence2 are
# FileSequence objects of equal length. This function is safe even if the
# files specified by each sequence overlap.
## \ingroup python
def cp( sequence1, sequence2 ) :

	if __sequencesClash( sequence1, sequence2 ) :
		raise RuntimeError( "Attempt to copy sequences with common filenames." )

	for src, dst in sequence1.mapTo( sequence2, True ) :
		shutil.copy( src, dst )

## Removes all the files specified by the sequence.
## \ingroup python
def rm( sequence ) :

	for f in sequence.fileNames() :

		os.remove( f )

## Concetenates all the files specified by the sequence to stdout
# \todo Allow destination file to be specified
## \ingroup python
def cat( sequence ) :

	for f in sequence.fileNames() :

		ret = os.system( 'cat "%s"' % ( f ) )

# private utility functions

def __sequencesClash( sequence1, sequence2 ) :

	s = set()
	for f in sequence1.fileNames() :
		s.add( f )

	for f in sequence2.fileNames() :
		if f in s :
			return True

	return False

def __tmpPrefix() :

	"""Returns a hopefully unique string suitable for use as the temporary
	sequence prefix when moving a sequence."""

	import hashlib
	import platform
	import time

	h = hashlib.md5()
	h.update( platform.node() ) # computer name
	h.update( str( os.getpid() ) )
	h.update( str( time.time() ) )
	return "ieSequenceTmp" + h.hexdigest() + "."

__all__ = [ "mv", "cp", "rm", "cat" ]

from ClassLoader import ClassLoader


import os
import imp
import glob
import re
import os.path
from fnmatch import fnmatch
from IECore import Msg, msg, SearchPath, warning

## This class defines methods for creating instances of classes
# defined in python modules on disk. We could just use the standard
# import mechanism for this but this gives us queries over what is
# available and versioning and suchlike, and uses a different set
# of searchpaths to the standard python paths. It's intended for
# loading classes derived from Op, ParameterisedProcedural and similar
# extension classes, and allows us to create small versioned units
# of functionality for use all over the place - the ieCore "do" script
# uses the ClassLoader to find operations it can perform for instance.
# This class will find files with the following template path:
# <any path>/<className>/<className>-<version>.py
# Where [] represents optional field.
# And for performance sake, it will not explore directories which 
# contain files that match this:
# <any path>/<className>/<className>*.*
class ClassLoader :

	## Creates a ClassLoader which will load
	# classes found on the SearchPath object passed
	# in.
	def __init__( self, searchPaths ) :

		self.__searchPaths = searchPaths
		self.__defaultVersions = {}
		self.refresh()

	## Returns an alphabetically sorted list
	# of all the classes found
	# on the searchpaths. The optional matchString
	# performs glob style matching to narrow down
	# the set of names returned.
	def classNames( self, matchString = "*" ) :

		self.__findAllClasses()
		### \todo Support re, and allow exclusions, etc...
		n = [ x for x in self.__classes.keys() if fnmatch( x, matchString ) ]
		n.sort()
		return n

	## Returns the available versions of the specified
	# class as a list of ints, with the latest version last. 
	# If the class doesn't exist returns an empty list.
	def versions( self, name ) :

		try :
			c = self.__findClass( name )
			return c["versions"]
		except :
			return []

	## Sets the default	version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method.
	def setDefaultVersion( self, name, version ) :

		self.__validateVersion( version )

		c = self.__findClass( name )

		if not version in c["versions"] :
			raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )

		self.__defaultVersions[name] = version

	## Returns the default version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method. If it has not
	# been set explicitly with setDefaultVersion() then
	# it defaults to the highest available version.
	def getDefaultVersion( self, name ) :

		c = self.__findClass( name )

		v = self.__defaultVersions.get( name, c["versions"][-1] )
		if not v in c["versions"] :
			msg( Msg.Level.Warning, "ClassLoader.getDefaultVersion", "Version %d doesn't exist, reverting to version %d." % ( v, c["versions"][-1] ) )
			v = c["versions"][-1]
			self.__defaultVersions[name] = v

		return v

	## Loads the specified version of the named class.
	# Version defaults to getDefaultVersion( name ) if
	# not specified. Note that this returns the actual class
	# object itself rather than an instance of that class.
	# It also adds two class attributes named "path" and "version"
	# with the info necessary to reload the Op from ClassLoader.
	def load( self, name, version = None ) :

		c = self.__findClass( name )

		if not version :
			version = self.getDefaultVersion( name )

		if not version in self.versions( name ) :
			raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )

		if version in c["imports"] :
			return c["imports"][version]

		nameTail = os.path.basename( name )
		fileName = os.path.join( name, nameTail + "-" + str(version) + ".py" )
		fileName = self.__searchPaths.find( fileName )
		if fileName=="" :
			raise IOError( "Unable to find implementation file for class \"%s\" version %d." % (name, version) )

		fileForLoad = open( fileName, "r" )
		try :
			module = imp.load_module( "IECoreClassLoader" + name.replace( ".", "_" ) + str( version ), fileForLoad, fileName, ( ".py", "r", imp.PY_SOURCE ) )
		finally :
			fileForLoad.close()

		if not getattr( module, nameTail, None ) :
			raise IOError( "File \"%s\" does not define a class named \"%s\"." % ( fileName, nameTail ) )

		result = getattr( module, nameTail )
		
		if getattr( result, 'staticTypeName', None ) == getattr( result.__bases__[0], 'staticTypeName', None ) :
			warning( "Class \"%s\" has the same staticTypeName as its Base Class. Perhaps you should call registerRunTimeTyped." % name )
		
		result.path = name
		result.version = version

		c["imports"][version] = result

		return result

	## The ClassLoader uses a caching mechanism to speed
	# up frequent reloads of the same class. This method
	# can be used to force an update of the cache to
	# reflect changes on the filesystem.
	def refresh( self ) :

		# __classes is a dictionary mapping from a class name
		# to information for that class in the following form
		# {
		#		"versions" : [], # a list containing all the available versions for that class
		#		"imports" : {}, # a dictionary mapping from version numbers to the actual class definition
		#						# this is filled in lazily by load()
		# }
		# this will be filled in lazily by __findClass and __findAllClasses
		self.__classes = {}
		self.__foundAllClasses = False

	__defaultLoaders = {}
	## Returns a ClassLoader configured to load from the paths defined by the
	# specified environment variable. The same object is returned each time,
	# allowing one loader to be shared by many callers.
	@classmethod
	def defaultLoader( cls, envVar ) :

		loader = cls.__defaultLoaders.get( envVar, None )
		if loader :
			return loader

		sp = ""
		if envVar in os.environ :
			sp = os.environ[envVar]
		else :
			msg( Msg.Level.Warning, "ClassLoader.defaultLoader", "Environment variable %s not set." % envVar )

		loader = cls( SearchPath( os.path.expandvars( sp ), ":" ) )
		cls.__defaultLoaders[envVar] = loader

		return loader

	## Returns a ClassLoader configured to load from the
	# paths defined by the IECORE_OP_PATHS environment variable. The
	# same object is returned each time, allowing one loader to be
	# shared by many callers.
	@classmethod
	def defaultOpLoader( cls ) :

		return cls.defaultLoader( "IECORE_OP_PATHS" )

	## Returns a ClassLoader configured to load from the
	# paths defined by the IECORE_PROCEDURAL_PATHS environment variable. The
	# same object is returned each time, allowing one loader to be
	# shared by many callers.
	@classmethod
	def defaultProceduralLoader( cls ) :

		return cls.defaultLoader( "IECORE_PROCEDURAL_PATHS" )

	def __updateClassFromSearchPath( self, searchPath, name ) :

		pattern = re.compile( ".*-(\d+).py$" )
		pruneDir = False
		nameTail = os.path.split( name )[-1]

		# globbing for any extension rather than .py to avoid exploring shader 
		# directories without Python files. Function returns true on those cases.
		gf = glob.glob( os.path.join( searchPath, name, nameTail + "*.*" ) )
		for f in gf :

			pruneDir = True

			m = re.match( pattern, f )
			try :
				version = int( m.group( 1 ) )
			except :
				continue

			c = self.__classes.setdefault( name, { "versions" : [], "imports" : {} } )

			if not version in c["versions"]:
				c["versions"].append( version )
				c["versions"].sort()

		return pruneDir

	def __findClass( self, name ) :

		if not name in self.__classes and not self.__foundAllClasses :
			for path in self.__searchPaths.paths :
				self.__updateClassFromSearchPath( path, name )

		if name in self.__classes :
			return self.__classes[name]
		else :
			raise RuntimeError( "Class \"%s\" doesn't exist." % name )

	def __findAllClasses( self ) :

		if self.__foundAllClasses :
			return

		self.__classes = {}
		for path in self.__searchPaths.paths :

			for root, dirs, files in os.walk( path ) :

				if path.endswith( '/' ) :
					nameBase = root[len(path):]
				else :
					nameBase = root[len(path)+1:]

				dirsToPrune = set()
				for d in dirs :

					if self.__updateClassFromSearchPath( path, os.path.join( nameBase, d ) ) :
						dirsToPrune.add( d )

				for d in dirsToPrune :
					dirs.remove( d )

		self.__foundAllClasses = True

	# throws an exception if the version is no good
	@staticmethod
	def __validateVersion( version ) :

		if not type( version ) is int :
			raise TypeError( "Version must be an integer" )


from RemovePrimitiveVariables import RemovePrimitiveVariables


from IECore import *
from fnmatch import fnmatchcase

class RemovePrimitiveVariables( PrimitiveOp ) :

	def __init__( self ) :

		PrimitiveOp.__init__( self, "Removes variables from primitives" )

		self.parameters().addParameters(
			[
				StringParameter(
					name = "mode",
					description = """This chooses whether or not the names parameter specifies the names of
						variables to keep or the names of variables to remove.""",
					defaultValue = "remove",
					presets = (
						( "keep", "keep" ),
						( "remove", "remove" )
					),
					presetsOnly = True
				),
				StringVectorParameter(
					name = "names",
					description = "The names of variables. These can include * or ? characters to match many names.",
					defaultValue = StringVectorData()
				)
			]
		)

	def modifyPrimitive( self, primitive, args ) :

		keep = args["mode"].value == "keep"

		for key in primitive.keys() :

			for n in args["names"] :

				m = fnmatchcase( key, n )
				if (m and not keep) or (not m and keep) :
					del primitive[key]

registerRunTimeTyped( RemovePrimitiveVariables )

from RenamePrimitiveVariables import RenamePrimitiveVariables


from IECore import *
from fnmatch import fnmatchcase

class RenamePrimitiveVariables( PrimitiveOp ) :

	def __init__( self ) :

		PrimitiveOp.__init__( self, "Renames primitive variables" )

		self.parameters().addParameters(
			[
				StringVectorParameter(
					name = "names",
					description = "The names of variables and their new names, separated by spaces.",
					defaultValue = StringVectorData()
				)
			]
		)

	def modifyPrimitive( self, primitive, args ) :

		for name in args["names"] :

			ns = name.split()
			if len(ns)!=2 :
				raise Exception( "\"%s\" should be of the form \"oldName newName\"" )

			primitive[ns[1]] = primitive[ns[0]]
			del primitive[ns[0]]

registerRunTimeTyped( RenamePrimitiveVariables )

from SequenceCpOp import SequenceCpOp


from IECore import *

class SequenceCpOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Copies file sequences.",
			FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustNotExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				)
			]
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()
		dst = self.parameters()["dst"].getFileSequenceValue()
		# if no frame list is specified on the dst parameter, then we use the same as src parameter.
		if isinstance( dst.frameList, EmptyFrameList ):
			dst.frameList = src.frameList

		cp(	src, dst )

		return StringData( str(dst) )

registerRunTimeTyped( SequenceCpOp )

from SequenceLsOp import SequenceLsOp


from IECore import *
import os
import os.path
import datetime

class SequenceLsOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Lists file sequences.",
			Parameter(
				name = "result",
				description = "A list of matching sequences.",
				defaultValue = StringVectorData()
			)
		)

		self.userData()["UI"] = CompoundObject(
			{
				"showResult": BoolData( True ),
			}
		)

		self.parameters().addParameters(
			[
				DirNameParameter(
					name = "dir",
					description = "The directory to look for sequences in.",
					defaultValue = "./",
					check = DirNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				BoolParameter(
					name = "recurse",
					description = "When on, recursively searches all subdirectories for sequences.",
					defaultValue = False,
				),
				BoolParameter(
					name = "followLinks",
					description = "When on, follow symbolic links during directory traversal.",
					defaultValue = False,
				),
				IntParameter(
					name = "maxDepth",
					description = "The maximum depth to recursion - this can be used to prevent accidental traversing of huge hierarchies.",
					defaultValue = 1000,
					minValue = 1,
				),
				IntParameter(
					name = "minSequenceSize",
					description = "The minimum number of files to be considered a sequence",
					defaultValue = 2,
					minValue = 1,
				),
				StringParameter(
					name = "type",
					description = "The file types of the sequences to classify.",
					defaultValue = "any",
					presets = (
						( "files", "files" ),
						( "directories", "directories" ),
						( "any", "any" )
					),
					presetsOnly = True,
				),
				StringParameter(
					name = "resultType",
					description = "The type of the result returned.",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				),
				BoolParameter(
					name = "contiguousSequencesOnly",
					description = "When on, only sequences without missing frames are returned.",
					defaultValue = False,
				),
				StringParameter(
					name = "format",
					description = "The format of the result. This can be used to return strings suitable for viewing a sequence in a particular package.",
					defaultValue = "<PREFIX><#PADDING><SUFFIX> <FRAMES>",
					presets = (
						( "shake", "shake -t <FRAMES> <PREFIX>#<SUFFIX>" ),
						( "nuke", "nuke -v <PREFIX>%0<PADDINGSIZE>d<SUFFIX> <FIRST>,<LAST>" ),
						( "fcheck", "fcheck -n <FIRST> <LAST> <STEP> <PREFIX><@PADDING><SUFFIX>" ),
						( "frameCycler", "framecycler <PREFIX><#PADDING><SUFFIX>" ),
						( "rv", "rv <PREFIX><#PADDING><SUFFIX>" ),
					)
				),
				StringVectorParameter(
					name = "extensions",
					description = "A list of file extensions which the sequences must have if they are to be listed. An empty list"
						"means that any sequence will be listed. The . character should be omitted from the extension.",
					defaultValue = StringVectorData(),
					presets = (
						( "images", StringVectorData( [ "tif", "tiff", "jpg", "jpeg", "exr", "cin", "dpx", "ppm", "png", "gif", "iff", "raw", "cr2" ] ) ),
					)
				),
				CompoundParameter(
					name = "advanced",
					description = "Advanced paramaters for filtering results based on various criteria",
					members = [

						CompoundParameter(
							name = "modificationTime",
							description = "Controls for filtering results based on modification time",

							members = [
								BoolParameter(
									name = "enabled",
									description = "Enable filtering based on modification time",
									defaultValue = False
								),
								StringParameter(
									name = "mode",
									description = "Changes the mode of modified time operation, e.g. before or after",
									defaultValue = "before",
									presets = (
										( "Before Start", "before" ),
										( "After End", "after" ),
										( "Between Start/End", "between" ),
										( "Outside Start/End", "outside" )
									),
									presetsOnly = True
								),

								# \todo Use a TimePeriodParameter here instead of seaprate start/end times
								DateTimeParameter(
									name = "startTime",
									description = "The (local) start time at which to make modification time comparisons against",
									defaultValue = datetime.datetime.now()
								),
								DateTimeParameter(
									name = "endTime",
									description = "The (local) end time at which to make modification time comparisons against",
									defaultValue = datetime.datetime.now()
								),

							]
						)
					]
				)
			]
		)

	@staticmethod
	def __walk(top, topdown=True, followlinks=False):

		from os.path import join, isdir, islink
		from os import listdir, error

		try:
			names = listdir(top)
		except error :
		        return

		dirs, nondirs = [], []
		for name in names:
			if isdir(join(top, name)):
				dirs.append(name)
			else:
				nondirs.append(name)

		if topdown:
			yield top, dirs, nondirs

		for name in dirs:
			path = join(top, name)
			if followlinks or not islink(path):
				for x in SequenceLsOp.__walk(path, topdown, followlinks):
					yield x

		if not topdown:
			yield top, dirs, nondirs


	def doOperation( self, operands ) :

		# recursively find sequences
		baseDirectory = operands["dir"].value
		if baseDirectory != "/" and baseDirectory[-1] == '/' :
			baseDirectory = baseDirectory[:-1]

		sequences = ls( baseDirectory, operands["minSequenceSize"].value )

		# If we've passed in a directory which isn't the current one it is convenient to get that included in the returned sequence names
		relDir = os.path.normpath( baseDirectory ) != "."

		if relDir :
			for s in sequences :
				s.fileName = os.path.join( baseDirectory, s.fileName )

		if operands["recurse"].value :
			# \todo Can safely use os.walk here after Python 2.6, which introduced the followlinks parameter
			for root, dirs, files in SequenceLsOp.__walk( baseDirectory, topdown = True, followlinks = operands["followLinks"].value ) :
				relRoot = root[len(baseDirectory)+1:]
				if relRoot!="" :
					depth = len( relRoot.split( "/" ) )
				else :
					depth = 0

				if depth>=operands["maxDepth"].value :
					dirs[:] = []

				for d in dirs :
					ss = ls( os.path.join( root, d ), operands["minSequenceSize"].value )
					if ss :
						for s in ss :

							if relDir :
								s.fileName = os.path.join( baseDirectory, relRoot, d, s.fileName )
							else :
								s.fileName = os.path.join( relRoot, d, s.fileName )
							sequences.append( s )


		# \todo This Op would benefit considerably from dynamic parameters
		# NB. Ordering of filters could have considerable impact on execution time. The most expensive filters should be specified last.
		filters = []

		# filter sequences based on type
		if operands["type"].value != "any" :

			if operands["type"].value == "files" :
				fileTypeTest = os.path.isfile
			else :
				assert( operands["type"].value == "directories" )
				fileTypeTest = os.path.isdir

			def matchType( sequence ) :
				for sequenceFile in sequence.fileNames() :
					if not fileTypeTest( sequenceFile ) :
						return False

				return True

			filters.append( matchType )

		# filter sequences based on extension
		if operands["extensions"].size() :

			extensions = set( ["." + e for e in operands["extensions"]] )

			def matchExt( sequence ) :
				return os.path.splitext( sequence.fileName )[1] in extensions

			filters.append( matchExt )

		# filter sequences which aren't contiguous
		if operands["contiguousSequencesOnly"].value :

			def isContiguous( sequence ):

				return len( sequence.frameList.asList() ) == max( sequence.frameList.asList() ) - min( sequence.frameList.asList() ) + 1

			filters.append( isContiguous )

		# advanced filters
		if operands["advanced"]["modificationTime"]["enabled"].value :

			filteredSequences = []

			mode = operands["advanced"]["modificationTime"]["mode"].value
			startTime = operands["advanced"]["modificationTime"]["startTime"].value
			endTime = operands["advanced"]["modificationTime"]["endTime"].value

			matchFn = None
			if mode == "before" :
				matchFn = lambda x : x < startTime

			elif mode == "after" :
				matchFn = lambda x : x > startTime

			elif mode == "between" :
				matchFn = lambda x : x > startTime and x < endTime

			else :
				assert( mode == "outside" )
				matchFn = lambda x : x < startTime or x > endTime

			assert( matchFn )

			def matchModificationTime( sequence ) :

				# If any file in the sequence matches, we have a match.
				for sequenceFile in sequence.fileNames() :

					st = os.stat( sequenceFile )
					modifiedTime = datetime.datetime.fromtimestamp( st.st_mtime )
					if matchFn( modifiedTime ) :
						return True

				return False

			filters.append( matchModificationTime )

		def matchAllFilters( sequence ) :

			for f in filters :
				if not f( sequence ) : return False

			return True

		def matchAnyFilter( sequence ) :

			for f in filters :
				if f( sequence ) : return True

			return False

		# \todo Allow matching of any filter, optionally
		sequences = filter( matchAllFilters, sequences )

		# reformat the sequences into strings as requested

		for i in xrange( 0, len( sequences ) ) :

			s = operands["format"].value
			s = s.replace( "<PREFIX>", sequences[i].getPrefix() )

			pi = s.find( "PADDING>" )
			if pi > 1 and s[pi-2]=='<' :
				s = s[:pi-2] + "".ljust( sequences[i].getPadding(), s[pi-1] ) + s[pi+8:]

			s = s.replace( "<PADDINGSIZE>", str( sequences[i].getPadding() ) )
			s = s.replace( "<SUFFIX>", sequences[i].getSuffix() )
			s = s.replace( "<FRAMES>", str( sequences[i].frameList ) )

			frames = sequences[i].frameList.asList()
			frames.sort()
			s = s.replace( "<FIRST>", str( frames[0] ) )
			s = s.replace( "<LAST>", str( frames[-1] ) )
			if s.find( "<STEP>" )!=-1 :
				stepCounts = {}
				for j in xrange( 1, len( frames ) ) :
					step = frames[j] - frames[j-1]
					stepCounts[step] = stepCounts.setdefault( step, 0 ) + 1
				m = 0
				step = 1
				for k, v in stepCounts.items() :
					if v > m :
						step = k
						m = v
				s = s.replace( "<STEP>", str( step ) )

			sequences[i] = s

		# return the result as the requested type
		if operands["resultType"].value == "string" :
			return StringData( "\n".join( sequences ) )
		else :
			return StringVectorData( sequences )

registerRunTimeTyped( SequenceLsOp )

from SequenceMvOp import SequenceMvOp


from IECore import *

class SequenceMvOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Moves file sequences.",
			FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
				minSequenceSize = 1,
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustNotExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				)
			]
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()
		dst = self.parameters()["dst"].getFileSequenceValue()
		# if no frame list is specified on the dst parameter, then we use the same as src parameter.
		if isinstance( dst.frameList, EmptyFrameList ):
			dst.frameList = src.frameList

		mv( src, dst )

		return StringData( str(dst) )

registerRunTimeTyped( SequenceMvOp )

from SequenceRmOp import SequenceRmOp


from IECore import *

class SequenceRmOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Removes file sequences.",
			FileSequenceParameter(
				name = "result",
				description = "The removed file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
				minSequenceSize = 1,
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "seq",
					description = "The file sequence to remove.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				)
			]
		)

	def doOperation( self, operands ) :

		rm( self.parameters()["seq"].getFileSequenceValue() )

		return StringData( operands["seq"].value )

registerRunTimeTyped( SequenceRmOp )

from SequenceCatOp import SequenceCatOp


from IECore import *

class SequenceCatOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Concatenates file sequences to stdout.",
			IntParameter(
				name = "result",
				description = "The number of files .",
				defaultValue = 0,
			)
		)

		self.parameters().addParameter(
			FileSequenceParameter(
				name = "src",
				description = "The source file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.MustExist,
				allowEmptyString = False,
				minSequenceSize = 1,
			)
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()

		cat( src )

		return IntData( len( src.fileNames() ) )

registerRunTimeTyped( SequenceCatOp )

from SequenceRenumberOp import SequenceRenumberOp


from IECore import *

class SequenceRenumberOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Renumbers file sequences.",
			FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
				minSequenceSize = 1,
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence. This may be left blank, in which case the source sequence is renumbered in place.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.DontCare,
					allowEmptyString = True,
					minSequenceSize = 1,
				),
				IntParameter(
					name = "multiply",
					description = "A number multiplied with each frame number before offsetting.",
					defaultValue = 1,
				),
				IntParameter(
					name = "offset",
					description = "A number added to each frame number after multiplication.",
					defaultValue = 0,
				),
			]
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()
		dst = src.copy()
		if operands["dst"].value!="" :
			dst.fileName = operands["dst"].value

		frames = [ x * operands["multiply"].value + operands["offset"].value for x in src.frameList.asList() ]

		dst.frameList = frameListFromList( frames )

		mv( src, dst )

		return StringData( dst.fileName )

registerRunTimeTyped( SequenceRenumberOp )

from SequenceConvertOp import SequenceConvertOp


from IECore import *

class SequenceConvertOp( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"This Op converts file sequences from one format to another. "
			"It supports all input formats for which a reader is available "
			"(" + " ".join( Reader.supportedExtensions() ) + ") and all output "
			"formats for which a writer is available (" + " ".join( Reader.supportedExtensions() ) + "). "
			"Because of it's general nature it doesn't support any additional options such as "
			"compression types for image formats. Also please note that not all combinations are "
			"possible - for instance you cannot convert an OBJ to a JPEG."
			,
			FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
				minSequenceSize = 1,
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = Reader.supportedExtensions(),
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustNotExist,
					allowEmptyString = False,
					extensions = Writer.supportedExtensions(),
					minSequenceSize = 1,
				)
			]
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()
		dst = self.parameters()["dst"].getFileSequenceValue()
		# if no frame list is specified on the dst parameter, then we use the same as src parameter.
		if isinstance( dst.frameList, EmptyFrameList ):
			dst.frameList = src.frameList

		# compare extensions, if extensions match, simply copy
		if src.fileName.split('.')[-1] == dst.fileName.split('.')[-1]:
			cpOp = SequenceCpOp()
			cpOp['src'] = operands["src"]
			cpOp['dst'] = operands["dst"]
			cpOp()
		else:
			# if extensions don't match, read and write
			for (sf, df) in zip(src.fileNames(), dst.fileNames()):
				img = Reader.create(sf).read()
				Writer.create(img, df).write()

		return StringData(str(dst))

registerRunTimeTyped( SequenceConvertOp )

from FormattedParameterHelp import formatParameterHelp


import IECore

## This function formats helpful descriptions of parameters, using
# a Formatter object.
## \ingroup python
def formatParameterHelp( parm, formatter ) :

	fn = __formatters.get( type( parm ), __formatParameter )
	fn( parm, formatter )

def __formatNumericParameter( parm, formatter ) :

	## \todo Could this be put somewhere generally useful?
	def formatFloat( f ) :

		f = "%.10f" % f
		f = f.rstrip( "0" )
		f = f.rstrip( "." )
		return f

	__formatParameter( parm, formatter )

	formatter.indent()

	if not parm.presetsOnly :

		if isinstance( parm, IECore.IntData ) :
			minValue = str( parm.minValue )
			maxValue = str( parm.maxValue )
		else :
			minValue = formatFloat( parm.minValue )
			maxValue = formatFloat( parm.maxValue )

		if parm.hasMinValue() and parm.hasMaxValue() :
			formatter.paragraph( "Range : %s - %s" % ( minValue, maxValue ) )
		elif parm.hasMinValue() :
			formatter.paragraph( "Min : %s" % minValue )
		elif parm.hasMaxValue() :
			formatter.paragraph( "Max : %s" % maxValue )

		formatter.unindent()

def __formatFileNameParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()

	e = parm.extensions
	if len( e ) :
		formatter.paragraph( "Valid extensions : " + " ".join( e ) )

	if parm.mustExist :
		formatter.paragraph( "File must exist" )

	if parm.mustNotExist :
		formatter.paragraph( "File must not exist" )

	if not parm.allowEmptyString :
		formatter.paragraph( "File must be specified" )

	formatter.unindent()

def __formatDirNameParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()

	if parm.mustExist :
		formatter.paragraph( "Directory must exist" )

	if parm.mustNotExist :
		formatter.paragraph( "Directory must not exist" )

	if not parm.allowEmptyString :
		formatter.paragraph( "Directory must be specified" )

	formatter.unindent()

def __formatFileSequenceParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()

	if parm.mustExist :
		formatter.paragraph( "Sequence must exist" )

	if parm.mustNotExist :
		formatter.paragraph( "Sequence must not exist" )

	if not parm.allowEmptyString :
		formatter.paragraph( "Sequence must be specified" )

	formatter.paragraph( "Values must be of the form \"something.###.ext\"" )

	formatter.unindent()

def __formatCompoundParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.heading( "Members" )
	formatter.indent()
	for p in parm.values() :
		formatParameterHelp( p, formatter )
	formatter.unindent()

def __formatParameter( parm, formatter ) :

	formatter.heading( parm.name + " (" + parm.__class__.__name__.replace( "Parameter", "" ) + ")" )
	formatter.paragraph( parm.description )

	formatter.indent()

	d = parm.defaultValue
	defaultPresetName = None
	for k, v in parm.presets().items() :
		if d == v :
			defaultPresetName = k
			break

	if defaultPresetName :
		formatter.paragraph( "Default : " + defaultPresetName )
	elif isinstance( d, IECore.Data ) and hasattr( d, "value" ) :
		formatter.paragraph( "Default : " + str( d.value ) )

	if len( parm.presetNames() ) :
		formatter.paragraph( "Presets : " )
		formatter.indent()
		formatter.paragraph( "\n".join( parm.presetNames() ) )
		formatter.unindent()

	formatter.unindent()

__formatters = {
	IECore.CompoundParameter : __formatCompoundParameter,
	IECore.FloatParameter : __formatNumericParameter,
	IECore.DoubleParameter : __formatNumericParameter,
	IECore.IntParameter : __formatNumericParameter,
	IECore.FileNameParameter : __formatFileNameParameter,
	IECore.DirNameParameter : __formatDirNameParameter,
	IECore.FileSequenceParameter : __formatFileSequenceParameter,
}

from ReadProcedural import ReadProcedural


from IECore import *
import math

class ReadProcedural( ParameterisedProcedural ) :

	def __init__( self ) :

		ParameterisedProcedural.__init__( self )

		self.parameters().addParameters(

			[
				CompoundParameter(
					name = "files",
					description = "Parameters controlling what files are rendered.",
					members = [
						FileNameParameter(
							name = "name",
							description = "The filename. This can include a series of one or more # characters to specify a file sequence. It may also include a @ character which will be substituted with each of the file numbers specified in the parameter below.",
							defaultValue = "",
							allowEmptyString = True,
							check = PathParameter.CheckType.DontCare,
						),
						StringParameter(
							name = "numbers",
							description = "A list of numbers to be substituted for the @ in the name above, to specify multiple files to render. This uses the same syntax as for frame ranges - e.g 1-10, 13, 15, 20-40x2.",
							defaultValue = "",
						),
						IntParameter(
							name = "frame",
							description = "The frame number of the files to render. This is substituted for any # characters in the filename.",
							defaultValue = 1,
							userData = CompoundObject( {
								"maya" : {
									"defaultConnection" : StringData( "time1.outTime" ),
								}
							} ),
						)
					]
				),

				CompoundParameter(
					name = "bounds",
					description = "Parameters controlling how the bounds are calculated for the particles.",
					members = [
						StringParameter(
							name = "mode",
							description = "How the bounds are calculated.",
							defaultValue = "calculated",
							presets = (
								( "Calculated", "calculated" ),
								( "Specified", "specified" ),
							),
							presetsOnly = True,
						),
						Box3fParameter(
							name = "specified",
							description = "The bounding box to use when bounding box mode is set to 'specified'.",
							defaultValue = Box3f( V3f( -1 ), V3f( 1 ) ),
						)
					]
				),
				CompoundParameter(
					name = "motion",
					description = "Parameters controlling motion.",
					members = [
						BoolParameter(
							name = "blur",
							description = "When this is on, motion blur is applied by loading two files per frame.",
							defaultValue = True
						)
					]
				),
			]

		)

	def doBound( self, args ) :

		if args["bounds"]["mode"].value=="specified" :

			return args["bounds"]["specified"].value

		else :

			bound = Box3f()
			fileNames = self.__allFileNames( args )
			for fileName in fileNames :

				if type( fileName ) is str :

					o = self.__readFile( fileName )
					if o :
						bound.extendBy( o.bound() )

				else :

					o = self.__readFile( fileName[0] )
					if o :
						bound.extendBy( o.bound() )

					o = self.__readFile( fileName[1] )
					if o :
						bound.extendBy( o.bound() )

			return bound

	def doRenderState( self, renderer, args ) :
	
		pass

	def doRender( self, renderer, args ) :

		fileNames = self.__allFileNames( args )
		shutter = renderer.getOption( "shutter" ).value

		for fileName in fileNames :

			if type( fileName ) is str :

				o = self.__readFile( fileName )
				if o :
					o.render( renderer )

			else :

				if shutter[0]==shutter[1] :
					o = self.__readFile( fileName[0] )
					if o :
						o.render( renderer )
				else :

					o0 = self.__readFile( fileName[0] )
					o1 = self.__readFile( fileName[1] )

					if o0 and o1 :

						m = MotionPrimitive()
						m[shutter[0]] = o0
						m[shutter[0]+1] = o1

						m.render( renderer )

	# returns either a list of filenames (no motion blur) or a list of tuples of filenames (for motion blur)
	def __allFileNames( self, args ) :

		if args["files"]["name"].value=="" :
			return []

		frame = args["files"]["frame"].value

		result = []
		if FileSequence.fileNameValidator().match( args["files"]["name"].value ) :
			sequence = FileSequence( args["files"]["name"].value, FrameRange( frame, frame ) )
			if args["motion"]["blur"].value :
				result.append( ( sequence.fileNameForFrame( frame ), sequence.fileNameForFrame( frame + 1 ) ) )
			else :
				result.append( sequence.fileNameForFrame( frame ) )
		else :
			result.append( args["files"]["name"].value )

		if "@" in args["files"]["name"].value :

			numbers = FrameList.parse( args["files"]["numbers"].value ).asList()

			newResult = []
			for f in result :

				for n in numbers :

					if type( f ) is str :

						newResult.append( f.replace( "@", str( n ) ) )

					else :

						f0 = f[0].replace( "@", str( n ) )
						f1 = f[1].replace( "@", str( n ) )
						newResult.append( ( f0, f1 ) )

		return result

	def __readFile( self, fileName ) :

		reader = Reader.create( fileName )
		if not reader :
			msg( Msg.Level.Error, "Read procedural", "Unable to create a Reader for '%s'." % fileName )
			return None

		o = reader.read()
		if not o or not o.isInstanceOf( "VisibleRenderable" ) :
			msg( Msg.Level.Error, "Read procedural", "Failed to load an object of type VisibleRenderable for '%s'." % fileName )
			return None

		return o

registerObject( ReadProcedural, 100026 )

from ClassLsOp import ClassLsOp


from IECore import *
import os
import os.path

class ClassLsOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Lists installed classes which can be loaded with IECore.ClassLoader.",
			Parameter(
				name = "result",
				description = "A list of classes.",
				defaultValue = StringVectorData()
			)
		)

		self.parameters().addParameters(
			[
				StringParameter(
					name = "type",
					description = "The type of class to list.",
					defaultValue = "procedural",
					presets = (
						( "Procedural", "procedural" ),
						( "Op", "op" ),
						( "Other", "other" ),
					),
					presetsOnly = True,
				),
				StringParameter(
					name = "match",
					description = "A glob style match string used to list only a subset of classes.",
					defaultValue = "*",
				),
				StringParameter(
					name = "searchPath",
					description = "When type is set to \"other\", this specifies a colon separated list of paths to search for classes on.",
					defaultValue = "",
				),
				StringParameter(
					name = "searchPathEnvVar",
					description = 	"When type is set to \"other\", this specifies an environment variable "
									"specifying a list of paths to search for classes on.",
					defaultValue = "",
				),
				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				)
			]
		)

	def doOperation( self, operands ) :

		t = operands["type"].value
		if t=="procedural" :
			loader = ClassLoader.defaultProceduralLoader()
		elif t=="op" :
			loader = ClassLoader.defaultOpLoader()
		else :
			if operands["searchPath"].value and operands["searchPathEnvVar"].value :
				raise RuntimeError( "Cannot specify both searchPath and searchPathEnvVar." )
			if not operands["searchPath"].value and not operands["searchPathEnvVar"].value :
				raise RuntimeError( "Must specify either searchPath or searchPathEnvVar." )

			if operands["searchPath"].value :
				sp = SearchPath( operands["searchPath"].value, ":" )
			else :
				sp = SearchPath( os.path.expandvars( os.environ[operands["searchPathEnvVar"].value] ), ":" )

			loader = ClassLoader( sp )

		classes = loader.classNames( operands["match"].value )

		if operands["resultType"].value == "string" :
			return StringData( "\n".join( classes ) )
		else :
			return StringVectorData( classes )

registerRunTimeTyped( ClassLsOp )

from OptionalCompoundParameter import OptionalCompoundParameter


import _IECore as IECore
from IECore import registerRunTimeTyped

## This class implements a CompoundParameter that do not validate optional parameters if they are undefined.
# This CompoundParameter derived class allows one to set a group of obligatory parameters that should always
# be validated. The non listed parameters are treated as optional. Optional parameters are not validated if their
# value is NullObject.
# \ingroup python
class OptionalCompoundParameter( IECore.CompoundParameter ):

	## Constructor
	# Uses the same parameters as CompoundParameter
	def __init__( self, *args, **argv ):
		IECore.CompoundParameter.__init__( self, *args, **argv )
		self.__obligatoryParameterNames = None

	## Defines a list of parameter names that must be validated.
	# The non listed parameters are treated as
	# optional. That means they can be set to NullObject. The obligatoryParameterNames can also be None.
	# In that case the validation used is from CompoundParameter
	def setObligatoryParameterNames( self, obligatoryParameterNames = None ):
		self.__obligatoryParameterNames = set( obligatoryParameterNames )

	## Returns the list of obligatory parameters or None if this OptionalCompoundParameter is working as a regular CompoundParameter.
	def getObligatoryParameterNames( self ):
		return self.__obligatoryParameterNames

	## Undefines the given parameter.
	def setParameterUndefined( self, paramName ):
		self[ paramName ] = IECore.NullObject()

	## Returns True if the given attribute is undefined and False otherwise.
	def getParameterUndefined( self, paramName ):
		return isinstance( self[ paramName ].getValue(), IECore.NullObject )

	## Overwrites default validation method from CompoundParameter.
	# This function does not validate undefined parameters ( values equal to NullObject ) not listed in the
	# obligatory parameter list.
	def valueValid( self, value ) :

		if self.__obligatoryParameterNames is None:

			return IECore.CompoundParameter.valueValid( self, value )

		else:

			if not isinstance( value, IECore.CompoundObject ):
				return ( False, "Not a CompoundObject!" )

			missing = self.__obligatoryParameterNames.difference( value.keys() )
			if len( missing ) > 0:
				return ( False, "Keys missing in CompoundObject: " + ", ".join( missing ) + "." )

			for name in value.keys():

				param = self[ name ]
				paramValue = value[ name ]

				if name in self.__obligatoryParameterNames or not isinstance( paramValue, IECore.NullObject ):
					( valid, msg ) = param.valueValid( paramValue )
					if not valid:
						return ( False, ("Error in parameter %s: " % name) + msg )

		return (True, "")

	## Smart getattr
	# Tries to use original CompoundParameter.__getattr__. If it fails, then try the local object dictionary.
	def __getattr__( self, attrName ):
		try:
			return IECore.CompoundParameter.__getattr__( self, attrName )
		except:
			return self.__dict__[ attrName ]

	## Smart setattr
	# Tries to use original CompoundParameter.__getattr__. If it fails, then try the local object dictionary.
	def __setattr__( self, attrName, attrValue ):
		try:
			parameter = IECore.CompoundParameter.__getattr__( self, attrName )
		except:
			self.__dict__[ attrName ] = attrValue
		else:
			parameter.smartSetValue( attrValue )

registerRunTimeTyped( OptionalCompoundParameter )

from FileExaminer import FileExaminer


import os.path
from IECore import ls

## The FileExaminer class is an abstract base class for classes which
# can perform some query on a file.
class FileExaminer :

	## Accepts a single argument, the name of the file to be
	# examined.
	def __init__( self, fileName ) :

		if type( fileName )!=str :
			raise TypeError( "FileName parameter must be a string." )

		self.__fileName = fileName

	## Sets the name of the file to be examined.
	def setFileName( self, fileName ) :

		self.__fileName = fileName

	## Returns the name of the file to be examined.
	def getFileName( self ) :

		return self.__fileName

	## Returns a set of dependencies for the file being examined.
	# The set can contain both fileName strings and also strings
	# specifying FileSequence objects in the form "sequence.#.ext frameRange".
	def dependencies( self ) :

		return set()

	## Recursively declares dependencies for all
	# files starting with the specified file, returning a set of
	# strings.
	@staticmethod
	def allDependencies( fileName ) :

		examiner = FileExaminer.create( fileName )
		if not examiner :
			return set()
		else :
			dependencies = examiner.dependencies()
			result = dependencies
			for dependency in dependencies :
				if IECore.FileSequence.fileNameValidator().match( dependency ) :
					ext = os.path.splitext( dependency )
					if ext!="" :
						ext = ext[1:]
						if ext in FileExaminer.__examiners :
							sequence = ls( dependency )
							if sequence :
								for f in sequence.fileNames() :
									result.update( FileExaminer.__allDependencies( f ) )
				else :
					result.update( allDependencies( dependency ) )

			return result

	## Creates an appropriate FileExaminer subclass for
	# the given fileName, returning None if no such
	# implementation exists.
	@staticmethod
	def create( fileName ) :

		ext = os.path.splitext( fileName )[1]
		if ext=="" :
			return None
		else :
			# strip the dot
			ext = ext[1:]

		if not ext in FileExaminer.__examiners :
			return None

		return FileExaminer.__examiners[ext]( fileName )

	## Returns a list of extensions for which FileExaminer
	# implementations have been registered
	@staticmethod
	def supportedExtensions() :

		return FileExaminer.__examiners.keys()

	## Registers a class which implements the FileExaminer
	# interface for files specified in the list of extensions.
	@staticmethod
	def registerExaminer( extensions, examinerClass ) :

		for ext in extensions :

			FileExaminer.__examiners[ext] = examinerClass

	__examiners = {}

from NukeFileExaminer import NukeFileExaminer


import _IECore as IECore
from FileExaminer import FileExaminer
import re
import shlex

## The NukeFileExaminer class implements the FileExaminer interface for
# nuke script files.
class NukeFileExaminer( FileExaminer ) :

	def __init__( self, fileName ) :

		FileExaminer.__init__( self, fileName )

	def dependencies( self ) :

		## I'd rather this was implemented as some sort of batch script
		# in nuke so it wasn't so text munging based, but it seems that
		# would mean taking a nuke gui license, which seems a bit excessive.
		# Hopefully this will do for now - it's really susceptible to changes
		# in the formatting of scripts.

		lines = []
		f = open( self.getFileName() )
		try :
			lines = f.readlines()
		finally :
			f.close()

		# find the frame range for the script
		rootNodes = self.__findNodes( "Root", lines )
		if len( rootNodes )==1 :
			scriptFirstFrame = int( self.__knobValue( "first_frame", rootNodes[0], "1" ) )
			scriptLastFrame = int( self.__knobValue( "last_frame", rootNodes[0], "100" ) )
		elif len( rootNodes )==0 :
			raise Exception( "No Root node found." )
		else :
			raise Exception( "More than one root node." )

		# find all the file references
		##################################################

		result = set()

		# first find read nodes
		readNodes = self.__findNodes( "Read", lines )
		for readNode in readNodes :

			fileName = self.__knobValue( "file", readNode, "" )
			proxyFileName = self.__knobValue( "proxy", readNode, "" )
			firstFrame = int( self.__knobValue( "first", readNode, "1" ) )
			lastFrame = int( self.__knobValue( "last", readNode, "1" ) )
			firstFrame = max( scriptFirstFrame, firstFrame )
			lastFrame = min( scriptLastFrame, lastFrame )

			if fileName!="" :
				result.add( self.__convertFileName( fileName, firstFrame, lastFrame ) )
			if proxyFileName!="" :
				result.add( self.__convertFileName( proxyFileName, firstFrame, lastFrame ) )

		# now find read geo nodes
		readGeoNodes = self.__findNodes( "ReadGeo", lines )
		for readGeoNode in readGeoNodes :

			fileName = self.__knobValue( "file", readGeoNode, "" )
			if fileName!="" :
				result.add( self.__convertFileName( fileName, scriptFirstFrame, scriptLastFrame ) )

		# now find grain nodes
		grainNodes = self.__findNodes( "ScannedGrain", lines )
		for grainNode in grainNodes :

			fileName = self.__knobValue( "fullGrain", grainNode, "" )
			if fileName!="" :
				firstFrame = int( self.__knobValue( "fullGrain.first_frame", grainNode, "1" ) )
				lastFrame = int( self.__knobValue( "fullGrain.last_frame", grainNode, "50" ) )
				result.add( self.__convertFileName( fileName, firstFrame, lastFrame ) )

		return result

	def __findNodes( self, nodeType, lines ) :

		result = []
		startIndex = 0
		while startIndex < len( lines ) :
			r = self.__findNode( nodeType, lines, startIndex )
			if r :
				result.append( r[0] )
				startIndex = r[1]
			else :
				break

		return result

	def __findNode( self, nodeType, lines, startIndex ) :

		for i in range( startIndex, len( lines ) ) :

			words = lines[i].split()

			if len( words ) == 2 and words[0]==nodeType and words[1]=="{" :

				node = []
				for j in range( i, len( lines ) ) :

					node.append( lines[j] )
					if lines[j].strip() == "}" :
						return node, j + 1

		return None

	def __knobValue( self, knobName, node, default ) :

		for line in node[1:-1] :

			words = shlex.split( line )
			if len( words )==2 and words[0]==knobName :
				return words[1]

		return default

	def __convertFileName( self, fileName, firstFrame, lastFrame ) :

		m = re.compile( "^(.*)%([0-9]*)d(.*)$" ).match( fileName )
		if m :

			padding = 1
			padder = m.group( 2 )
			if len( padder ) :

				if padder[0]=="0" :
					padding = int( padder[1:] )
				else :
					# if the padding doesn't begin with 0 then
					# nuke seems to pad with spaces. we won't accept
					# spaces in a filename
					raise Exception( "Filename \"%s\" is padded with spaces." % fileName )
			
			fileName = m.group( 1 ) + "#" * padding + m.group( 3 )
		
		if "#" in fileName :
			return fileName + " " + str( IECore.FrameRange( min( firstFrame, lastFrame ), max( firstFrame, lastFrame ) ) )
		else :
			return fileName

FileExaminer.registerExaminer( [ "nk" ], NukeFileExaminer )

from RIBFileExaminer import RIBFileExaminer


from FileExaminer import FileExaminer
from IECore import findSequences
import os

## The RIBFileExaminer class implements the FileExaminer interface for
# RIB files. It uses the ribdepends utility distributed with 3delight
# to do the work.
class RIBFileExaminer( FileExaminer ) :

	def __init__( self, fileName ) :

		FileExaminer.__init__( self, fileName )

	def dependencies( self ) :

		pipe = os.popen( "ribdepends \"%s\"" % self.getFileName(), 'r' )
		lines = pipe.readlines()
		status = pipe.close()
		if status :
			raise RuntimeError( "Error running ribdepends" )

		goodIdentifiers = [ 's', 't', 'x', 'u', 'c' ]
		files = []
		for line in lines :
			if len( line ) > 4 :
				if line[0]=='[' and line[2:4]=="] " and line[1] in goodIdentifiers :

					files.append( line[4:].strip() )

		result = set()
		for f in files :
			result.add( f )

		return result

FileExaminer.registerExaminer( [ "rib" ], RIBFileExaminer )

from FileDependenciesOp import FileDependenciesOp


from IECore import *
import os
import os.path

class FileDependenciesOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Lists the dependencies of a file.",
			Parameter(
				name = "result",
				description = "A list of required files and file sequences.",
				defaultValue = StringVectorData()
			)
		)

		self.parameters().addParameters(
			[
				FileNameParameter(
					name = "file",
					description = "The file to list dependencies for.",
					defaultValue = "",
					check = DirNameParameter.CheckType.MustExist,
					extensions = " ".join( FileExaminer.supportedExtensions() ),
					allowEmptyString = False,
				),
				BoolParameter(
					name = "recurse",
					description = "When on, recursively searches the file dependency tree and lists all results.",
					defaultValue = False,
				),
				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				)
			]
		)

	def doOperation( self, operands ) :

		files = set()
		if operands["recurse"].value :

			files = FileExaminer.allDependencies( operands["file"].value )

		else :

			files = FileExaminer.create( operands["file"].value ).dependencies()

		if operands["resultType"].value == "string" :
			return StringData( "\n".join( [str(s) for s in files] ) )
		else :
			return StringVectorData( [str(s) for s in files] )

registerRunTimeTyped( FileDependenciesOp )

from CheckFileDependenciesOp import CheckFileDependenciesOp


from IECore import *
import os
import os.path

class CheckFileDependenciesOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Checks that the dependencies of a file exist.",
			Parameter(
				name = "result",
				description = "A list of missing files and file sequences.",
				defaultValue = StringVectorData()
			)
		)

		self.parameters().addParameters(
			[
				FileNameParameter(
					name = "file",
					description = "The file to check dependencies for.",
					defaultValue = "",
					check = DirNameParameter.CheckType.MustExist,
					extensions = " ".join( FileExaminer.supportedExtensions() ),
					allowEmptyString = False,
				),
				BoolParameter(
					name = "recurse",
					description = "When on, recursively searches the file dependency tree and checks all dependencies.",
					defaultValue = False,
				),
				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				)
			]
		)

	def doOperation( self, operands ) :

		dependencies = FileDependenciesOp()( file = operands["file"], recurse = operands["recurse"], resultType = "stringVector" )

		missingFiles = []
		for dependency in dependencies :

			if FileSequence.fileNameValidator().match( dependency ) :

				s = dependency.split()
				sequence = FileSequence( s[0], FrameList.parse( s[1] ) )

				frames = sequence.frameList.asList()
				frames.sort()
				missingFrames = []
				for frame in frames :
					if not os.path.exists( sequence.fileNameForFrame( frame ) ) :
						missingFrames.append( frame )

				if len( missingFrames ) :
					missingFiles.append( FileSequence( s[0], frameListFromList( missingFrames ) ) )

			else :

				if not os.path.exists( dependency ) :
					missingFiles.append( dependency )

		if operands["resultType"].value == "string" :
			return StringData( "\n".join( [str(s) for s in missingFiles] ) )
		else :
			return StringVectorData( [str(s) for s in missingFiles] )

registerRunTimeTyped( CheckFileDependenciesOp )

from PointsExpressionOp import PointsExpressionOp


from IECore import *

class PointsExpressionOp( ModifyOp ) :

	def __init__( self ) :

		ModifyOp.__init__( self, "Modifies the primitive variables of a PointsPrimitive using a python expression.",
			ObjectParameter(
				name = "result",
				description = "The modified points primitive.",
				defaultValue = PointsPrimitive( 0 ),
				type = PointsPrimitive.staticTypeId(),
			),
			ObjectParameter(
				name = "input",
				description = "The points primitive to modify.",
				defaultValue = PointsPrimitive( 0 ),
				type = PointsPrimitive.staticTypeId(),
			)
		)

		self.parameters().addParameters(

			[
				StringParameter(
					name = "expression",
					description = "A python expression applied on a per point basis. This may read from or assign to any of the per point"
						"primitive variables, and also assign any True value to the variable \"remove\" to have the point removed. The variable \"i\""
						"holds the index for the current point.",
					defaultValue = "",
				)
			]
		)

	def modify( self, pointsPrim, operands ) :

		# this dictionary derived class provides the locals for
		# the expressions. it overrides the item accessors to
		# provide access into the point data
		class LocalsDict( dict ) :

			def __init__( self, p ) :

				self.__numPoints = p.numPoints
				self.__vectors = {}
				for k in p.keys() :
					try :
						if len( p[k].data ) == p.numPoints :
							self.__vectors[k] = p[k].data
					except :
						pass

				self.__vectors["remove"] = BoolVectorData( p.numPoints )
				self.__haveRemovals = False

			def __getitem__( self, n ) :

				vector = self.__vectors.get( n, None )
				if vector is None :
					return dict.__getitem__( self, n )
				else :
					return vector[self["i"]]

			def __setitem__( self, n, v ) :

				vector = self.__vectors.get( n, None )
				if vector is None :
					dict.__setitem__( self, n, v )
				else :
					vector[self["i"]] = v
					if n=="remove" and v :
						self.__haveRemovals = True

			def removals( self ) :

				if self.__haveRemovals :
					return self.__vectors["remove"]
				else :
					return None

		# get globals and locals for expressions
		g = globals()
		l = LocalsDict( pointsPrim )

		# run the expression for each point
		e = compile( operands["expression"].value, "expression", "exec" )
		for i in range( 0, pointsPrim.numPoints ) :

			l["i"] = i
			exec e in g, l

		# filter out any points if requested
		removals = l.removals()
		if removals :

			newNumPoints = pointsPrim.numPoints
			for k in pointsPrim.keys() :

				try :
					primVar = pointsPrim[k]
					if len( primVar.data )==pointsPrim.numPoints :
						primVar.data = VectorDataFilterOp()( input = primVar.data, filter = removals, invert=True )
						pointsPrim[k] = primVar
						newNumPoints = primVar.data.size()
				except :
					# we'll get exceptions for data types which don't define len()
					pass

			pointsPrim.numPoints = newNumPoints

registerRunTimeTyped( PointsExpressionOp )

from Struct import Struct


## The Struct class provides an incredibly simple container with
# attribute access defined. It's useful for testing and provides
# a handy alternative to a dictionary at times.
class Struct:

	def __init__( self, **kwargs ) :

		for k, v in kwargs.items() :
			setattr( self, k, v )

import Enum
from LsHeaderOp import LsHeaderOp


from IECore import *

class LsHeaderOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Lists the contents of file headers using reader classes such as AttributeCache, HierarchicalCache and Reader.",
			Parameter(
				name = "result",
				description = "A list of meta-data contained in the file header.",
				defaultValue = StringVectorData()
			)
		)

		self.parameters().addParameters(

			[
				FileNameParameter(
					name = "file",
					description = "The file to list the header from.",
					defaultValue = "",
					check = FileNameParameter.CheckType.MustExist,
					extensions = " ".join( IndexedIOInterface.supportedExtensions() + Reader.supportedExtensions() ),
					allowEmptyString = False,
				),

				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				)
			]
		)

		self.userData()["UI"] = CompoundObject(
									{
										"showResult": BoolData( True ),
										"closeAfterExecution": BoolData( True ),
									}
								)

	def doOperation( self, operands ) :

		# \todo Do the exception handling properly!
		headers = None
		try:
			cache = HierarchicalCache( operands["file"].value, IndexedIOOpenMode.Read )
			headers = cache.readHeader()
		except:
			debugException( "Error reading header as HierarchicalCache." )
			try:
				cache = AttributeCache( operands["file"].value, IndexedIOOpenMode.Read )
				headers = cache.readHeader()
			except:
				debugException( "Error reading header as AttributeCache." )
				try:
					reader = Reader.create( operands["file"].value )
					headers = reader.readHeader()
				except:
					debugException( "Error reading header as Reader." )
					headers = None

		if headers is None:
			raise Exception, ("Could not get header from file " + operands["file"].value)

		def formatCompound( compound, lines, level = 0 ):
			levelStr = "";
			i = 0
			while( i < level ):
				levelStr += "     "
				i += 1

			for key in compound.keys():
				value = compound[ key ]
				if isinstance( value, CompoundObject ) or isinstance( value, CompoundData ):
					lines.append( levelStr + key + ": " )
					formatCompound( value, lines, level + 1 )
				elif isSimpleDataType( value ):
					lines.append( levelStr + key + ": " + str(value.value) )
				elif isSequenceDataType( value ):
					lines.append( levelStr + key + ": " + ", ".join( map( str, value ) ) )
				else:
					lines.append( levelStr + key + ": " + str(value) )

		headerLines = []
		formatCompound( headers, headerLines )

		if operands.resultType.value == "string" :
			return StringData( "\n".join( headerLines ) )
		else :
			return StringVectorData( headerLines )

registerRunTimeTyped( LsHeaderOp )

from curry import curry


## Curries a function and a bunch of arguments - that is it creates a
# new function with the arguments baked in, so they don't need to be
# passed when the function is called.
def curry( *bakeArgs, **bakeKwds ) :

	func = bakeArgs[0]
	bakeArgs = bakeArgs[1:]

	def curriedFunction( *callTimeArgs, **callTimeKwds ) :
		args = bakeArgs + callTimeArgs
		kwds = bakeKwds.copy()
		kwds.update( callTimeKwds )

		return func( *args, **kwds )

	return curriedFunction

from MenuItemDefinition import MenuItemDefinition


## The MenuItemDefinition class defines the contents of a menu item for use
# with the MenuDefinition class. It does nothing towards actually implementing
# a user interface, but instead defines content for a user interface
# implementation to realise. This allows menus to be defined in a UI agnostic
# way and then used with different toolkits.
#
# The MenuItemDefinition has the following attributes :
#
# command : a callable object invoked when the user selects the menu item
#
# secondaryCommand : a callable object invoked when the user selects the menu item
# in some other way. this is toolkit dependent - for instance in maya this command
# would be used when the option box is selected.
#
# divider : True if the menu item is a divider, False otherwise.
#
# active : if False then the menu item is unselectable. may also be a callable
# object which returns a boolean value to allow dynamic activation
#
# description : a string with help for the menu item
#
# subMenu : a callable object which returns a MenuDefinition, to produce
# a dynamically generated submenu.
#
# checkBox : A callable item to return True or False for checkBox state, or None
# for no checkBox at all. When checkBox is not None, the callable specified by the
# command attribute will be called whenever the checkBox is toggled.
#
# \todo Validation of attribute values, so for instance divider and command
# can't both be set at the same time.
# \ingroup python
class MenuItemDefinition :

	__slots__ = [ "command", "secondaryCommand", "divider", "active", "description", "subMenu", "checkBox", "blindData" ]

	def __init__( self, dictionary = None, **kwArgs ) :

		self.command = None
		self.secondaryCommand = None
		self.divider = False
		self.active = True
		self.description = ""
		self.subMenu = None
		self.checkBox = None
		self.blindData = {}

		if dictionary :
			for k, v in dictionary.items() :
				setattr( self, k, v )

		for k, v in kwArgs.items() :
			setattr( self, k, v )

	def __repr__( self ) :

		d = {}
		for s in self.__slots__ :
			d[s] = getattr( self, s )

		return "MenuItemDefinition( " + repr( d ) + " )"

from MenuDefinition import MenuDefinition


import re
from MenuItemDefinition import MenuItemDefinition

## The MenuDefinition class defines the contents of a hierarchical menu
# containing MenuItemDefinition instances. It does nothing towards actually
# creating a user interface, but instead defines content for a user
# interface implementation to realise. This allows menus to be defined in a
# UI agnostic way and then used with different toolkits.
# \ingroup python
class MenuDefinition :

	def __init__( self, items = [] ) :

		self.__items = []

		for path, item in items :

			self.append( path, item )

	## Prepends a menu item to the menu. The item will
	# appear before all other items in its respective
	# submenu.
	def prepend( self, path, item ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		self.__items.insert( 0, ( path, item ) )

	## Appends a menu item at the end. The item will
	# appear after all other items in its respective
	# submenu.
	def append( self, path, item ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		self.__items.append( ( path, item ) )

	## Insert a menu item before the specified menu item.
	def insertBefore( self, path, item, beforePath ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		i = self.__pathIndex( beforePath )
		self.__items.insert( i, ( path, item ) )

	## Insert a menu item after the specified menu item.
	def insertAfter( self, path, item, afterPath ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		i = self.__pathIndex( afterPath )
		self.__items.insert( i+1, ( path, item ) )

	## Removes the named menu item. Raises a KeyError if
	# no such item exists and raiseIfMissing is True.
	def remove( self, path, raiseIfMissing=True ) :

		index = None
		for i in range( 0, len( self.__items ) ) :
			if self.__items[i][0]==path :
				index = i
				break

		if not index is None :
			del self.__items[index]
		else :
			if raiseIfMissing :
				raise KeyError( path )

	## Removes all items whose paths match the given
	# regular expression.
	def removeMatching( self, regEx ) :

		if type( regEx ) is str :
			regEx = re.compile( regEx )

		toRemove = []
		for i in range( 0, len( self.__items ) ) :
			if regEx.search( self.__items[i][0] ) :
				toRemove.append( i )

		toRemove.sort()
		toRemove.reverse()
		for i in toRemove :
			del self.__items[i]

	## Removes all menu items from the definition.
	def clear( self ) :

		del self.__items[:]

	## Returns a list of tuples of the form (path, MenuItemDefinition).
	# This can be used in realising the menu in a UI toolkit. This list
	# should be considered read-only - use the other methods to add and
	# remove items.
	def items( self ) :

		return self.__items

	## Returns a new MenuDefinition containing only the menu items
	# that reside below the specified root path. The paths in this
	# new definition are all adjusted to be relative to the requested
	# root.
	def reRooted( self, root ) :

		if not len( root ) :
			return MenuDefinition( [] )

		if root[-1]!="/" :
			root = root + "/"

		newItems = []
		for item in self.items() :

			if item[0].startswith( root ) :
				newItems.append( ( item[0][len(root)-1:], item[1] ) )

		return MenuDefinition( newItems )

	def __repr__( self ) :

		return "MenuDefinition( " + repr( self.items() ) + " )"

	def __pathIndex( self, path ) :

		for i in range( 0, len( self.__items ) ) :

			if self.__items[i][0]==path :
				return i

		raise KeyError( path )

from ParameterParser import ParameterParser


import IECore

## This class defines a means of parsing a list of string arguments with respect to a Parameter definition.
# It's main use is in providing values to be passed to Op instances in the do script. It now also provides
# the reverse operation - serialising parameter values into a form which can later be parsed.
# \ingroup python
class ParameterParser :

	__typesToParsers = {}
	__typesToSerialisers = {}

	## Parses the args to set the values of the parameters
	# held by a CompoundParameter object. args must be a list of strings.
	#
	# Parsing expects a series of entries of the form -parameterName value(s). It
	# is also possible to specify a series of child parameters which can be parsed without
	# their name being specified - this provides much less verbose command lines for commonly
	# used parameters, and the possibility of supporting the very common "command [options] filenames"
	# form of command. Flagless parameters are specified as userData in the parameters object passed -
	# this should be a StringVectorData named "flagless" in a CompoundData called "parser" -
	# these flagless parameters are parsed in the order they are specified in the data element.
	# If parsing fails at any time then a descriptive exception is raised.
	def parse(self, args, parameters):

		if not type( args ) is list :
			raise TypeError, "args must be a python list!"

		flagless = None
		flaglessIndex = 0
		if "parser" in parameters.userData() :
			if "flagless" in parameters.userData()["parser"] :
				flagless = parameters.userData()["parser"]["flagless"]
				if not flagless.isInstanceOf( IECore.StringVectorData.staticTypeId() ) :
					raise TypeError( "\"flagless\" parameters must be specified by a StringVectorData instance." )

		parmsFound = {}
		result = parameters.defaultValue
		args = args[:] # a copy we can mess around with
		while len(args):

			# get the name of the parameter being specified
			if len( args[0] ) and args[0][0]=="-" :
				name = args[0][1:]
				del args[0]
			elif flagless and flaglessIndex < flagless.size() :
				name = flagless[flaglessIndex]
				flaglessIndex += 1
			else :
				raise SyntaxError( "Expected a flag argument (beginning with \"-\")" )

			# find the parameter being specified. this might be the child of a compound
			# denoted by "." separated names.
			nameSplit = name.split( "." )

			# recurses down the parameters tree
			p = parameters

			if p.__class__.__name__ == 'CompoundParameter':
				for i in range(0, len(nameSplit)):
					if not nameSplit[i] in p:
						raise SyntaxError( "\"%s\" is not a parameter name." % name )
					p = p[nameSplit[i]]

			if len( args ) :

				# see if the argument being specified is a preset name, in which case we
				# don't need a special parser.
				if args[0] in p.presets() :

					p.setValue( args[0] )
					del args[0]
					continue

				# or if it's asking us to read a value from a file, in which case we
				# also don't need a special parser
				if args[0].startswith( "read:" ) :

					fileName = args[0][5:]
					r = IECore.Reader.create( fileName )
					if not r :
						raise RuntimeError( "Unable to create reader for file \"%s\"." % fileName )

					p.setValidatedValue( r.read() )
					del args[0]
					continue

				# or if it's asking us to evaluate a python string to create a value,
				# in which case we also don't need a special parser
				if args[0].startswith( "python:" ) :

					toEval = args[0][7:]
					r = eval( toEval )
					p.setValidatedValue( r )
					del args[0]
					continue

			# we're gonna need a specialised parser
			parser = None
			typeId = p.typeId()
			while typeId != IECore.TypeId.Invalid :
				if typeId in self.__typesToParsers :
					parser = self.__typesToParsers[typeId]
					break
				typeId = IECore.RunTimeTyped.baseTypeId( typeId )
			
			if parser is None :
				raise SyntaxError( "No parser available for parameter \"%s\"." % name )

			try :
				parser( args, p )
			except Exception, e :
				raise SyntaxError( "Problem parsing parameter \"%s\" : %s " % ( name, e ) )

	## Returns a list of strings representing the values contained within parameters.
	# This can later be passed to parse() to retrieve the values. May
	# throw an exception if the value held by any of the parameters is not valid.
	def serialise( self, parameters ) :

		return self.__serialiseWalk( parameters, "" )

	def __serialiseWalk(self, parameter, rootName):

		# Allow parameters to request not to be serialised.
		if "parser" in parameter.userData() and "serialise" in parameter.userData()["parser"]:
			if not parameter.userData()["parser"]["serialise"].value:
				return []

		# Try to find a serialiser
		serialiser = None
		typeId = parameter.typeId()
		while typeId != IECore.TypeId.Invalid :
			if typeId in self.__typesToSerialisers :
				serialiser = self.__typesToSerialisers[typeId]
				break
			if typeId != IECore.TypeId.CompoundParameter :
				# consider serialisers for base classes.
				typeId = IECore.RunTimeTyped.baseTypeId( typeId )
			else :
				# don't consider serialiser for base classes of CompoundParameter
				# so that it doesn't get an unecessary serialisation in the case
				# of a serialiser for Parameter being registered.
				break

		if serialiser is None and not isinstance( parameter, IECore.CompoundParameter ) :
			# bail if no serialiser available, unless it's a CompoundParameter in which case we'll content
			# ourselves with serialising the children if no serialiser is available.
			raise RuntimeError( "No serialiser available for parameter \"%s\"" % parameter.name )
		
		result = []
		if serialiser is not None :
			# we have a registered serialiser - use it
			try:
				s = serialiser( parameter )
				if not isinstance( s, list ) :
					raise RuntimeError( "Serialiser did not return a list." )
				for ss in s :
					if not isinstance( ss, str ) :
						raise RuntimeError( "Serialiser returned a list with an element which is not a string." )
			except Exception, e:
				raise RuntimeError("Problem serialising parameter \"%s\" : %s" % (parameter.name, e))

			# serialize
			result += [ '-' + rootName + parameter.name ] + s

		# recurse to children of CompoundParameters
		if parameter.isInstanceOf( IECore.CompoundParameter.staticTypeId() ) :
			path = rootName
			if parameter.name :
				path = path + parameter.name + '.'
			# recurse
			for childParm in parameter.values() :
				result += self.__serialiseWalk( childParm, path )
		
		return result
		
	@classmethod
	## Registers a parser and serialiser for a new Parameter type.
	def registerType( cls, typeId, parser, serialiser ) :

		cls.__typesToParsers[typeId] = parser
		cls.__typesToSerialisers[typeId] = serialiser

	@classmethod
	## Registers a default parser and serialiser for a Parameter type
	# for which repr( parameter.getValue() ) yields a valid python statement.
	def registerTypeWithRepr( cls, typeId ) :

		cls.__typesToParsers[typeId] = None
		cls.__typesToSerialisers[typeId] = _serialiseUsingRepr

###################################################################################################################
# parsers and serialisers for the built in parameter types
###################################################################################################################

def __parseBool( args, parameter ) :

	validValues = {
		"True" : True,
		"False" : False,
		"On" : True,
		"Off" : False,
		"1" : True,
		"0" : False,
		"true" : True,
		"false" : False,
		"on" : True,
		"off" : False,
	}

	if not len( args ) :
		raise SyntaxError( "Expected a boolean value." )

	if not args[0] in validValues :
		raise SyntaxError( "Expected one of %s" % ", ".join( validValues.keys() ) )

	parameter.setValidatedValue( IECore.BoolData( validValues[args[0]] ) )
	del args[0]

def __parseNumeric( dataType, integer, args, parameter ) :

	if not len( args ) :
		raise SyntaxError( "Expected a numeric value." )

	if integer :

		try :
			value = int( args[0] )
		except :
			raise SyntaxError( "Expected an integer value." )

	else :

		try :
			value = float( args[0] )
		except :
			raise SyntaxError( "Expected a float value." )

	parameter.setValidatedValue( dataType( value ) )
	del args[0]

def __parseNumericCompound( dataType, elementType, n, integer, args, parameter ) :

	if not len( args ) :
		raise SyntaxError( "Expected %d numeric values." % n )

	values = []
	for i in range( 0, n ) :

		if integer :

			try :
				values.append( int( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d integer values." % n )

		else :

			try :
				values.append( float( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d float values." % n )

	parameter.setValidatedValue( dataType( elementType( *values ) ) )

def __parseBoxOrLine( dataType, boxType, elementType, integer, args, parameter ) :

	n = elementType.dimensions() * 2

	if not len( args ) :
		raise SyntaxError( "Expected %d numeric values." % n )

	values = []
	for i in range( 0, n ) :

		if integer :

			try :
				values.append( int( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d integer values." % n )

		else :

			try :
				values.append( float( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d float values." % n )

	parameter.setValidatedValue( dataType( boxType( elementType( *values[:n/2] ), elementType( *values[n/2:] ) ) ) )

def __parseString( args, parameter ) :
	parameter.setValidatedValue( IECore.StringData( args[0] ) )
	del args[0]

def __parseStringArray( args, parameter ) :

	d = IECore.StringVectorData()
	foundFlag = False
	while len( args ) and not foundFlag :
		a = args[0]
		if len(a) and a[0] == "-" :
			foundFlag = True
		else :
			d.append( a )
			del args[0]

	parameter.setValidatedValue( d )

def __parseBoolArray( args, parameter ) :

	d = IECore.BoolVectorData()

	validValues = {
		"True" : True,
		"False" : False,
		"On" : True,
		"Off" : False,
		"1" : True,
		"0" : False,
		"true" : True,
		"false" : False,
		"on" : True,
		"off" : False,
	}

	done = False
	while len( args ) and not done :
		a = args[0]
		if len(a) and a[0] == "-" :
			done = True
		elif not args[0] in validValues :
			raise SyntaxError( "Expected one of %s" % ", ".join( validValues.keys() ) )
		else :
			try :
				d.append( validValues[args[0]] )
				del args[0]
			except :
				done = True

	parameter.setValidatedValue( d )

def __parseNumericArray( dataType, integer, args, parameter ) :

	d = dataType()

	done = False
	while not done and len( args ) :
		if integer :
			try :
				d.append( int( args[0] ) )
				del args[0]
			except :
				done = True
		else :
			try :
				d.append( float( args[0] ) )
				del args[0]
			except :
				done = True

	parameter.setValidatedValue( d )
	
def __parseTransformationMatrix( dataType, args, parameter ) :

	if not len(args) :
		raise SyntaxError( "Expected 29 values." )

	if dataType == IECore.TransformationMatrixfData :
		vecType = IECore.V3f
		angleType = IECore.Eulerf
		orientationType = IECore.Quatf
	else :
		vecType = IECore.V3d
		angleType = IECore.Eulerd
		orientationType = IECore.Quatd

	t = type(dataType().value)()

	t.translate = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	t.scale = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]	
	
	t.shear = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	t.rotate = angleType( float(args[0]), float(args[1]), float(args[2] ) )
	t.rotate.setOrder( getattr( angleType.Order, args[3] ) )
	del args[0:4]
	
	t.rotationOrientation = orientationType( float(args[0]), float(args[1]), float(args[2] ), float(args[3]) )
	del args[0:4]
	
	t.rotatePivot = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	t.rotatePivotTranslation = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	t.scalePivot = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	t.scalePivotTranslation = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]
	
	parameter.setValidatedValue( dataType(t) )

def __parseObject( args, parameter ) :

	v = args[0]
	v = IECore.hexToDecCharVector( v )
	mio = IECore.MemoryIndexedIO( v, "/", IECore.IndexedIOOpenMode.Read )
	v = IECore.Object.load( mio, "v" )
	parameter.setValidatedValue( v )
	del args[0]
	
def __serialiseString( parameter ) :

	return [ parameter.getTypedValue() ]

def __serialiseStringArray( parameter ) :

	return list( parameter.getValue() )

def __serialiseUsingStr( parameter ) :

	return [ str( parameter.getValidatedValue() ) ]
	
def __serialiseUsingSplitStr( parameter ) :

	return str( parameter.getValidatedValue() ).split()	

def _serialiseUsingRepr( parameter ) :

	return [ "python:" + repr( parameter.getValidatedValue() ) ]

def __serialiseTransformationMatrix( parameter ) :

	t = parameter.getValidatedValue().value
	retList = []
	retList.extend( str(t.translate).split() )
	retList.extend( str(t.scale).split() )
	retList.extend( str(t.shear).split() )
	retList.extend( str(t.rotate).split() )
	retList.append( str(t.rotate.order()) )
	retList.extend( str(t.rotationOrientation).split() )
	retList.extend( str(t.rotatePivot).split() )
	retList.extend( str(t.rotatePivotTranslation).split() )
	retList.extend( str(t.scalePivot).split() )
	retList.extend( str(t.scalePivotTranslation).split() )
	return retList

def __serialiseObject( parameter ) :

	v = parameter.getValidatedValue()
	mio = IECore.MemoryIndexedIO( IECore.CharVectorData(), "/", IECore.IndexedIOOpenMode.Write )
	v.save( mio, "v" )
	buf = mio.buffer()
	return [ IECore.decToHexCharVector( buf ) ]
	
ParameterParser.registerType( IECore.BoolParameter.staticTypeId(), __parseBool, __serialiseUsingStr )
ParameterParser.registerType( IECore.IntParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.IntData, True, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.FloatParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.FloatData, False, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.DoubleParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.DoubleData, False, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.StringParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.ValidatedStringParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.PathParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.PathVectorParameter.staticTypeId(), __parseStringArray, __serialiseStringArray )
ParameterParser.registerType( IECore.FileNameParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.DirNameParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.FileSequenceParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.FrameListParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.StringVectorParameter.staticTypeId(), __parseStringArray, __serialiseStringArray )
ParameterParser.registerType( IECore.BoolVectorParameter.staticTypeId(), ( lambda args, parameter : __parseBoolArray( args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.IntVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.IntVectorData, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.FloatVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.FloatVectorData, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.DoubleVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.DoubleVectorData, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V2iParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2iData, IECore.V2i, 2, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V3iParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3iData, IECore.V3i, 3, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V2fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2fData, IECore.V2f, 2, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V3fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3fData, IECore.V3f, 3, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V2dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2dData, IECore.V2d, 2, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V3dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3dData, IECore.V3d, 3, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Color3fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.Color3fData, IECore.Color3f, 3, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Color4fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.Color4fData, IECore.Color4f, 4, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.M44fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.M44fData, IECore.M44f, 16, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.M44dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.M44dData, IECore.M44d, 16, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box2fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2fData, IECore.Box2f, IECore.V2f, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box3fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3fData, IECore.Box3f, IECore.V3f, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box2dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2dData, IECore.Box2d, IECore.V2d, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box3dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3dData, IECore.Box3d, IECore.V3d, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box2iParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2iData, IECore.Box2i, IECore.V2i, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box3iParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3iData, IECore.Box3i, IECore.V3i, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.LineSegment3fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.LineSegment3fData, IECore.LineSegment3f, IECore.V3f, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.LineSegment3dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.LineSegment3dData, IECore.LineSegment3d, IECore.V3d, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.SplineffParameter.staticTypeId(), None, _serialiseUsingRepr )
ParameterParser.registerType( IECore.SplinefColor3fParameter.staticTypeId(), None, _serialiseUsingRepr )
ParameterParser.registerType( IECore.TransformationMatrixfParameter.staticTypeId(), ( lambda args, parameter : __parseTransformationMatrix( IECore.TransformationMatrixfData, args, parameter ) ), __serialiseTransformationMatrix )
ParameterParser.registerType( IECore.TransformationMatrixdParameter.staticTypeId(), ( lambda args, parameter : __parseTransformationMatrix( IECore.TransformationMatrixdData, args, parameter ) ), __serialiseTransformationMatrix )
ParameterParser.registerType( IECore.ObjectParameter.staticTypeId(), __parseObject, __serialiseObject )

__all__ = [ "ParameterParser" ]

from SearchReplaceOp import SearchReplaceOp


import shutil
import tempfile
import os
import re
from IECore import *

class SearchReplaceOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "Performs a search and replace on ASCII text files.",
			FileNameParameter(
				name = "result",
				description = "The resulting file. Maya be the same as the input file.",
				defaultValue = "",
				allowEmptyString = True,
			)
		)

		self.parameters().addParameters(
			[
				FileNameParameter(
					name = "source",
					description = "The source file.",
					defaultValue = "",
					extensions = "ma rib shk nk",
					check = FileNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				FileNameParameter(
					name = "destination",
					description = "The destination file.",
					defaultValue = "",
					allowEmptyString = False,
				),
				StringParameter(
					name = "searchFor",
					description = "The pattern to search for",
					defaultValue = "",
				),
				BoolParameter(
					name = "regexpSearch",
					description = "Enable to perform searching based on regular expressions",
					defaultValue = False
				),
				StringParameter(
					name = "replaceWith",
					description = "The string with which to replace patterns which match the search criteria",
					defaultValue = "",
				)
			]
		)

	def doOperation( self, operands ) :

		source = operands["source"].value
		destination = operands["destination"].value

		searchFor = operands["searchFor"].value
		if not operands["regexpSearch"] :
			searchFor = re.escape( searchFor )

		replaceWith = operands["replaceWith"].value

		inFileStat = os.stat( source ).st_mode

		inFile = open( source, "r" )

		if source == destination :

			inPlace = True
			fd = tempfile.mkstemp()

		else :

			inPlace = False
			fd = ( os.open( destination, os.O_WRONLY | os.O_TRUNC | os.O_CREAT ), destination )

		outFile = os.fdopen( fd[0], "w" )


		inLine = inFile.readline()
		while inLine :

			outLine = re.sub( searchFor, replaceWith, inLine )
			os.write( fd[0], outLine )

			inLine = inFile.readline()

		inFile.close()
		outFile.close()

		if inPlace :

			shutil.move( destination, destination + ".bak" )
			shutil.move( fd[1], destination )

		os.chmod( destination, inFileStat )

		return StringData( destination )

registerRunTimeTyped( SearchReplaceOp )

from CapturingMessageHandler import CapturingMessageHandler


from IECore import MessageHandler, Struct

## The CapturingMessageHandler simply stores all messages passed to it
# in an attribute called messages. It's useful for verifying expected
# message output during testing. Each message in the messages list is a Struct
# with "level", "context" and "message" attributes.
# \ingroup python
class CapturingMessageHandler( MessageHandler ) :

	def __init__( self ) :

		MessageHandler.__init__( self )

		self.messages = []

	def handle( self, level, context, message ) :

		s = Struct()
		s.level = level
		s.context = context
		s.message = message

		self.messages.append( s )


from FileSequenceAnalyzerOp import FileSequenceAnalyzerOp


# \ingroup python

import os, copy
from IECore import *

# Base abstract class useful for analyzing file sequences.
# It checks file size changes, missing files and incomplete images.
class FileSequenceAnalyzerOp( Op ):

	def __init__( self, description, resultParameter, extensions = [] ):

		Op.__init__( self, description, resultParameter )

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "fileSequence",
					description = "The input image sequence to be loaded.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = extensions,
				),
				FrameListParameter(
					name = 'frameList',
					description = 'Provide the frames this sequence is supposed to have. Leave empty to load only the existent frames.',
				),
				IntParameter(
					name = 'expectedSizeReduction',
					description = "Percentage number that sets what would be considered a normal file size reduction between the frames. It is enforced from the previous and next frames.\nFor example, 20% means that frame 1 and 3 on the sequence 1,2 and 3 cannot be smaller than 80% the size of frame 2.",
					defaultValue = 20,
					minValue = 0,
					maxValue = 100,
				),
				BoolParameter(
					name = 'checkFiles',
					description = 'Set this On if you want to check the file contents. It may take longer to compute.',
					defaultValue = True,
				),
			]
		)

		self.__lastParameterValue = CompoundObject()

	def __compute( self ):

		p = self.parameters()
		args = p.getValue()

		if self.__lastParameterValue == args:
			return

		fileSequence = p['fileSequence'].getFileSequenceValue()

		if args["frameList"].value == "":
			expectedFrameList = fileSequence.frameList
		else:
			expectedFrameList = p['frameList'].getFrameListValue()

		# check missing frames
		frameInfo = {}
		missing = []
		frames = expectedFrameList.asList()

		self.__frameNumbers = frames

		for f in frames:
			framePath = fileSequence.fileNameForFrame( f )
			try:
				ft = os.stat( framePath )
			except:
				missing.append( f )
				frameInfo[ f ] = { "path": framePath, "type": 'missing' }
			else:
				frameInfo[ f ] = { "path": framePath, "size": ft.st_size }

		nonMissingFrames = list( set( frames ).difference( missing ) )

		# check for corrupted files
		corrupted = []
		if args["checkFiles"].value and len( nonMissingFrames ):

			# currently we only have methods for checking image files. So we have to identify if it is an image sequence
			try:
				reader = Reader.create( frameInfo[0]['path'] )
			except Exception, e:
				# unrecognized extension?
				debugException("Disabling check for corrupted files because could not instantiate reader:", e)
			else:
				if isinstance( reader, ImageReader ):
					for f in nonMissingFrames:
						reader = Reader.create( frameInfo[f]['path'] )
						if not reader.isComplete():
							corrupted.append( f )
							frameInfo[ f ]['type'] = 'corrupted'

		nonCorruptedFrames = list( set(nonMissingFrames).difference( corrupted ) )
		nonCorruptedFrames.sort()

		# check for abrupt size changes on the sequence.
		suspicious = []
		if args["expectedSizeReduction"].value != 100:
			minSizeRatio = 1. - ( args["expectedSizeReduction"].value / 100.0 )
			previousSize = -1
			previousGoodFrame = -1
			for f in nonCorruptedFrames:
				currentSize = frameInfo[f]['size']
				if previousSize != -1:

					currentRatio = float(currentSize)/previousSize
					if currentRatio < minSizeRatio:
						# Current frame looks broken because it is strangely smaller then the previous good frame.
						suspicious.append( f )
						frameInfo[ f ]['type'] = 'suspicious'
						frameInfo[ f ]['reason'] = previousGoodFrame
						# 'continue' to skip setting the previousSize variable.
						continue

					if previousGoodFrame == (f - 1):
						previousRatio = previousSize/float(currentSize)
						if previousRatio < minSizeRatio:
							# Previous frame looks broken because it is strangely smaller then the current frame.
							suspicious.append( f )
							frameInfo[ previousGoodFrame ]['type'] = 'suspicious'
							frameInfo[ previousGoodFrame ]['reason'] = f
							# don't 'continue' because the problem was in a previous frame. this is good!

				previousSize = currentSize
				previousGoodFrame = f

		# set all the other frames as good
		for f in frameInfo.keys():
			if not frameInfo[f].has_key( 'type' ):
				frameInfo[f]['type'] = 'ok'

		self.__frameInfo = frameInfo
		self.__lastParameterValue = args.copy()

	# Returns the frame numbers analysed.
	def allFrames( self ):
		self.__compute()
		return list( self.__frameNumbers )

	# Returns the frames that had abrupt file size changes.
	def suspiciousFrames( self ):
		self.__compute()
		return filter( lambda x: self.__frameInfo[x]['type'] == 'suspicious', self.__frameInfo.keys() )

	# Returns the frames that correspond to corrupted files.
	def corruptedFrames( self ):
		self.__compute()
		return filter( lambda x: self.__frameInfo[x]['type'] == 'corrupted', self.__frameInfo.keys() )

	# Returns the frames that are missing on the file sequence.
	def missingFrames( self ):
		self.__compute()
		return filter( lambda x: self.__frameInfo[x]['type'] == 'missing', self.__frameInfo.keys() )

	# Returns a dictionary where the keys are the frame numbers from the input frameList parameter and the values are dict with "type", "path" and "size" items.
	# The "type" could be one of these: "ok", "missing", "corrupted" or "suspicious".
	# When there's a suspicious frame, it means there was an abrupt change on the file size.
	# So, there will be also another item called "reason" and it's value is the frame that leads to this conclusion.
	def frameInfos( self ):
		self.__compute()
		return copy.deepcopy( self.__frameInfo )


registerRunTimeTyped( FileSequenceAnalyzerOp )

from CheckImagesOp import CheckImagesOp


# \ingroup python

import os
from IECore import *

#This Op checks an image file sequence for corrupted and missing files. It also warns of abrupt file size changes.
# The Op will raise an error if there's any missing or corrupt file.
# Otherwise it will return the number of suspicious frames (strange file sizes).
# This Op is targeted for users only.
class CheckImagesOp( FileSequenceAnalyzerOp ) :

	def __init__( self ) :

		FileSequenceAnalyzerOp.__init__( self,
"""This Op checks an image file sequence for corrupted and missing files. It also warns of abrupt file size changes.
The Op will raise an error if there's any missing or corrupt file.
Otherwise it will return the number of suspicious frames (strange file sizes).""",
			IntParameter(
				name = "result",
				description = "Returns the number of suspicious frames.",
				defaultValue = 0,
			),
			extensions = "dpx exr cin tif tiff jpeg jpg"
		)

		self.userData()["UI"] = CompoundObject(
			{
				"infoMessages": BoolData( True ),
			}
		)

	def doOperation( self, args ) :

		suspicious = self.suspiciousFrames()
		suspicious.sort()
		corrupted =  self.corruptedFrames()
		corrupted.sort()
		missing = self.missingFrames()
		missing.sort()

		info( "Checking sequence:", args["fileSequence"].value, "..." )

		if len(missing):
			error( "Missing frames:", ','.join( map( str, missing ) ) )
		if len(corrupted):
			error( "Corrupted frames:", ','.join( map( str, corrupted ) ) )
		if len(suspicious):
			warning("Suspicious frames:", ','.join( map( str, suspicious ) ) )

		if len(missing) + len(corrupted) + len(suspicious) == 0:
			info( "File sequence is ok." )

		if (len(missing) + len(corrupted)) > 0:
			raise Exception, "The file sequence did not pass the test."

		return IntData( len(suspicious) )

registerRunTimeTyped( CheckImagesOp )

from FileSequenceGraphOp import FileSequenceGraphOp


# \ingroup python

import os, copy
from IECore import *

# Creates a bar graph object that represents the file sizes as bar height and file status as bar color.
class FileSequenceGraphOp( FileSequenceAnalyzerOp ):

	def __init__( self ):

		FileSequenceAnalyzerOp.__init__( self,
			description = 'Creates a bar graph object that represents the file sizes as bar height and file status as bar color.',
			resultParameter = ObjectParameter(
								name = 'barGraph',
								description = 'Resulting bar graph object that represents the input file sequence.',
								defaultValue = NullObject(),
								type = TypeId.Group,
			)
		)

		self.userData()["UI"] = CompoundObject(
			{
				"showResult": BoolData( True ),
				"farmExecutable": BoolData( False ),
			}
		)

	def doOperation( self, args ) :

		frameInfos = self.frameInfos()
		frames = self.allFrames()

		# get the max file size to normalize later.
		maxSize = -1
		for v in frameInfos.values():
			s = v.get('size', -1)
			if maxSize < s:
				maxSize = s

		# create a Mesh primitive as a litle square 1x1 aligned to plane X,Y
		mesh = MeshPrimitive.createPlane( Box2f( V2f(0,0), V2f(1,1) ) )

		# create the main group
		graph = Group()

		colors = {
			'ok': Color3f( 0.1, 0.7, 0.1 ),
			'missing': Color3f( 0.5, 0.3, 0.7 ),
			'corrupted': Color3f( 0.8, 0.3, 0.3 ),
			'suspicious': Color3f( 1, 1, 0.3 ),
		}

		# set a value that will be added to the height of every file. So even super small files will be visible on the graph.
		baseHeight = maxSize/6.

		# create one Group for each frame in the file sequence.
		for (fi, f) in enumerate(frames):

			info = frameInfos[f]

			frameGrp = Group()
			m = M44f()
			m.translate( V3f( fi, 0, 0 ) )
			m.scale( V3f( 1.0, info.get('size', maxSize) + baseHeight, 1.0 ) )
			frameGrp.setTransform( MatrixTransform( m ) )
			state = AttributeState()
			state.attributes['name'] = StringData( info['path'] )
			state.attributes['color'] = Color3fData( colors[ info['type'] ] )
			frameGrp.addState( state )
			frameGrp.addChild( mesh )

			graph.addChild( frameGrp )

		m = M44f()
		m.translate( V3f( -1./2., -1./10., 0 ) )
		m.scale( V3f( 1. / len( frames ), (1. / (maxSize +  + baseHeight)) / 5., 1. ) )
		graph.setTransform( MatrixTransform( m ) )
		# Indicate this Renderable object is meant to be a hyperlink to entities on the file system.
		graph.blindData()['hyperlink'] = StringData('FileSystem')
		return graph

registerRunTimeTyped( FileSequenceGraphOp )

from LayeredDict import LayeredDict


from IECore import CompoundObject, CompoundParameter

## This class takes a stack of dictionary like objects and provides
# read only access to them as if they were one, falling through to
# lower dictionaries if the requested item isn't in the ones above.
class LayeredDict :

	## Constructs a new layered dictionary :
	#
	# dicts : a list of dictionaries to layer. dicts[0] has
	# highest precedence and dicts[-1] lowest. after construction
	# this list can be accessed as LayeredDict.layers, and may be
	# modified in place to change the layering.
	#
	# dictClasses : a set of classes to consider as dictionary types.
	# this comes into play when layeredDict["key"] yields an object of
	# that type. in this case the object is not returned alone, but instead
	# a new LayeredDict of that object and any others with the same key
	# is returned - this allows the layering to continue in dictionaries held
	# within the topmost dictionary.
	def __init__( self, dicts, dictClasses = set( [ dict, CompoundObject, CompoundParameter ] ) ) :

		for d in dicts :
			assert( d.__class__ in dictClasses )

		self.layers = dicts

		assert( isinstance( dictClasses, set ) )
		self.__dictClasses = dictClasses

	def __getitem__( self, key ) :

		for i in range( 0, len( self.layers ) ) :

			if key in self.layers[i] :

				value = self.layers[i][key]
				if not value.__class__ in self.__dictClasses :

					return value

				else :

					# need to return a LayeredDict
					dicts = [ value ]
					for j in range( i + 1, len( self.layers ) ) :
						if key in self.layers[j] :
							dicts.append( self.layers[j][key] )

					return LayeredDict( dicts )

		raise KeyError( key )

	def __contains__( self, key ) :

		for i in range( 0, len( self.layers ) ) :

			if key in self.layers[i] :
				return True

		return False

	def keys( self ) :

		allKeys = set()
		for d in self.layers :

			allKeys.update( d.keys() )

		return list( allKeys )

	def get( self, key, defaultValue ) :

		try :
			return self.__getitem__( key )
		except KeyError :
			return defaultValue


from CompoundVectorParameter import CompoundVectorParameter


from IECore import *

## This class is a CompoundParameter that only accepts vector parameters with the same length.
# \ingroup python

class CompoundVectorParameter ( CompoundParameter ):

	def __testParameterType( self, parameter ):
		data = parameter.getValue()
		if not isSequenceDataType( data ):
			raise TypeError, "The parameter %s cannot be added because it does not hold vector data object." % parameter.name

	# overwrites base class definition just to limit the parameter types accepted.
	def addParameter( self, parameter ):
		self.__testParameterType( parameter )
		CompoundParameter.addParameter( self, parameter )

	# overwrites base class definition just to limit the parameter types accepted.
	def addParameters( self, parameters ):
		for parameter in parameters:
			self.__testParameterType( parameter )
		CompoundParameter.addParameters( self, parameters )

	# overwrites base class definition just to limit the parameter types accepted.
	def insertParameter( self, parameter, other ):
		self.__testParameterType( parameter )
		CompoundParameter.insertParameter( self, parameter, other )

	## Returns true only if all the vector parameters are of the same length and they also validate ok.
	def valueValid( self, value ) :

		res = CompoundParameter.valueValid( self, value )
		if not res[0]:
			return res

		size = None
		keys = value.keys()
		values = value.values()
		for i in range( 0, len( keys ) ) :

			thisSize = len( values[i] )
			if size is None:
				size = thisSize

			if size != thisSize :
				return ( False, ( "Parameter \"%s\" has wrong size ( expected %d but found %d )" % ( keys[i], size, thisSize ) ) )

		return ( True, "" )

registerRunTimeTyped( CompoundVectorParameter )

from AttributeBlock import AttributeBlock


## A context object intended for use with python's "with" syntax. It calls
# renderer.attributeBegin() in enter and renderer.attributeEnd() in exit.
class AttributeBlock :

	def __init__( self, renderer ) :

		self.__renderer = renderer

	def __enter__( self ) :

		self.__renderer.attributeBegin()

	def __exit__( self, type, value, traceBack ) :

		self.__renderer.attributeEnd()



from TransformBlock import TransformBlock


## A context object intended for use with python's "with" syntax. It calls
# renderer.transformBegin() in enter and renderer.transformEnd() in exit.
class TransformBlock :

	def __init__( self, renderer ) :

		self.__renderer = renderer

	def __enter__( self ) :

		self.__renderer.transformBegin()

	def __exit__( self, type, value, traceBack ) :

		self.__renderer.transformEnd()



from WorldBlock import WorldBlock


## A context object intended for use with python's "with" syntax. It calls
# renderer.worldBegin() in enter and renderer.worldEnd() in exit.
class WorldBlock :

	def __init__( self, renderer ) :

		self.__renderer = renderer

	def __enter__( self ) :

		self.__renderer.worldBegin()

	def __exit__( self, type, value, traceBack ) :

		self.__renderer.worldEnd()



from SequenceMergeOp import SequenceMergeOp


# \ingroup python

import os
from IECore import *

# The SequenceMergeOp is a base class for Ops which perform merging of two file sequences into a single file sequence.
class SequenceMergeOp( Op ) :

	def __init__( self, description, extensions = [] ) :

		assert( type( extensions ) is list )

		Op.__init__(
			self,
			description,
			StringVectorParameter(
				name = "result",
				description = "The names of the files created",
				defaultValue = StringVectorData([])
			)
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "fileSequence1",
					description = "The first input sequence",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = extensions,
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "fileSequence2",
					description = "The second input sequence",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = extensions,
					minSequenceSize = 1,
				),
				FileSequenceParameter(
					name = "outputFileSequence",
					description = "The output file sequence to generate. For each frame in this sequence, the corresponding inputs for that frame are merged",
					defaultValue = "",
					allowEmptyString = False,
					check = FileSequenceParameter.CheckType.MustNotExist,
					minSequenceSize = 1,
				),
			]
		)

	# Derived classes should override this method, and merge the files given in "fileName1" and "fileName2" into "outputFileName",
	# returning True on success or False on failure.
	def _merge( self, fileName1, fileName2, outputFileName ) :

		pass

	def doOperation( self, args ) :

		fileSequence1 = self.parameters()['fileSequence1'].getFileSequenceValue()
		fileSequence2 = self.parameters()['fileSequence2'].getFileSequenceValue()
		outputFileSequence = self.parameters()['outputFileSequence'].getFileSequenceValue()

		resultFiles = []

		for frame in outputFileSequence.frameList.asList() :

			fileName1 = fileSequence1.fileNameForFrame( frame )
			fileName2 = fileSequence2.fileNameForFrame( frame )
			outputFileName = outputFileSequence.fileNameForFrame( frame )

			if self._merge( fileName1, fileName2, outputFileName ) :

				resultFiles.append( outputFileName )

		return StringVectorData( resultFiles )

registerRunTimeTyped( SequenceMergeOp )

from ImageSequenceCompositeOp import ImageSequenceCompositeOp


# \ingroup python

import os
from IECore import *

# The ImageSequenceCompositeOp does a simple A-over-B composite of two input sequences of image files
class ImageSequenceCompositeOp( SequenceMergeOp ) :

	def __init__( self ) :

		SequenceMergeOp.__init__(
			self,
			"The ImageSequenceCompositeOp does a simple A-over-B composite of two input sequences of image files",
			extensions = [ "tif", "tiff", "exr", "cin", "dpx", "jpg" ]
		)

		self.parameters().addParameters(
			[
				IntParameter(
					name = "operation",
					description = "The compositing operation to apply",
					defaultValue = ImageCompositeOp.Operation.Over,
					presets = (
						( "Over", ImageCompositeOp.Operation.Over ),
						( "Min", ImageCompositeOp.Operation.Min ),
						( "Max", ImageCompositeOp.Operation.Max ),
					),
					presetsOnly = True
				),
			]
		)

	def _merge( self, fileName1, fileName2, outputFileName ) :

		image1 = Reader.create( fileName1 ).read()
		if not image1.isInstanceOf( "ImagePrimitive" ) :
			raise RuntimeError( "ImageSequenceCompositeOp: Could not load image from from '%s'" % ( fileName1 ) )

		image2 = Reader.create( fileName2 ).read()
		if not image2.isInstanceOf( "ImagePrimitive" ) :
			raise RuntimeError( "ImageSequenceCompositeOp: Could not load image from from '%s'" % ( fileName2 ) )

		op = ImageCompositeOp()

		resultImage = op(
			input = image2,
			imageA = image1,
			operation = self.parameters()["operation"].getValue().value,
			inputMode = ImageCompositeOp.InputMode.Unpremultiplied,
		)
		Writer.create( resultImage, outputFileName ).write()

		return True

registerRunTimeTyped( ImageSequenceCompositeOp )

from DateTimeParameterParser import *


import IECore

def __parseDateTime( args, parameter ) :

	import datetime

	if not len( args ) :
		raise SyntaxError( "Expected date/time." )


	dateStr = args[0]
	if not type( dateStr ) is str :
		raise SyntaxError( "Expected date/time." )

	for format in [ "%Y-%m-%d %H:%M:%S", "%Y-%m-%d %H:%M", "%Y-%m-%d" ] :

		try :
			date = datetime.datetime.strptime( dateStr, format )
			d = IECore.DateTimeData( date )
			parameter.setValidatedValue( d )
			del args[0]
			return
		except :
			pass

	# If no day specified, use today
	for format in [ "%H:%M:%S", "%H:%M" ] :

		try :
			date = datetime.datetime.strptime( dateStr, format )
			today = datetime.datetime.now()
			date = date.replace( day = today.day, month = today.month, year = today.year )
			d = IECore.DateTimeData( date )
			parameter.setValidatedValue( d )
			del args[0]
			return
		except :
			pass

	raise SyntaxError( "Not a valid date/time: '%s'" % ( dateStr ) )


def __serialiseDateTime( parameter ) :

	dt = parameter.getTypedValue()

	# strptime does not support fractional seconds
	dt = dt.replace( microsecond = 0 )

	return [ str( dt ) ]

IECore.ParameterParser.registerType( IECore.DateTimeParameter.staticTypeId(), __parseDateTime, __serialiseDateTime )

__all__ = []

from MotionBlock import MotionBlock


## A context object intended for use with python's "with" syntax. It calls
# renderer.motionBegin() in enter and renderer.motionEnd() in exit.
class MotionBlock :

	def __init__( self, renderer, times ) :

		self.__renderer = renderer
		self.__times = times

	def __enter__( self ) :

		self.__renderer.motionBegin( self.__times )

	def __exit__( self, type, value, traceBack ) :

		self.__renderer.motionEnd()



from SubstitutedDict import SubstitutedDict


import string
import IECore

## Acts like a dictionary, but performs substitutions on any string or StringData values
# retrieved from it. Substitutions are specified as a dictionary where keys are tokens to
# be substituted and values are the corresponding substitutions. Substitution is performed
# using string.Template().
class SubstitutedDict :

	def __init__( self, dict, substitutions, dictClasses = set( [ dict, IECore.CompoundObject, IECore.CompoundParameter, IECore.LayeredDict ] ) ) :
	
		self.__dict = dict
		self.__substitutions = substitutions
		self.__dictClasses = dictClasses
				
	def __getitem__( self, key ) :
	
		value = self.__dict[key]
				
		if value.__class__ in self.__dictClasses :
			return SubstitutedDict( value, self.__substitutions, self.__dictClasses )
			
		if isinstance( value, basestring ) :
			return string.Template( value ).safe_substitute( self.__substitutions )
		elif isinstance( value, IECore.StringData ) :
			return IECore.StringData( string.Template( value.value ).safe_substitute( self.__substitutions ) )
	
		return value
		
	def __contains__( self, key ) :
	
		return self.__dict.__contains__( key )
	
	def __eq__( self, other ) :
	
		if not isinstance( other, SubstitutedDict ) :
			return False
			
		return (	self.__dict == other.__dict and 
					self.__substitutions == other.__substitutions and
					self.__dictClasses == other.__dictClasses 	)
	
	def __ne__( self, other ) :
	
		return not self.__eq__( other )
				
	def keys( self ) :
	
		return self.__dict.keys()
		
	def values( self, substituted=True ) :
	
		if substituted :
			return [ self.get( k ) for k in self.__dict.keys() ]
		else :
			return self.__dict.values()

	def items( self, substituted=True ) :
	
		return zip( self.keys(), self.values() )

	def get( self, key, defaultValue=None, substituted=True ) :
	
		try :
			if substituted :
				return self.__getitem__( key )
			else :
				return self.__dict[key]
		except KeyError :
			return defaultValue

	def substitutions( self ) :
	
		return self.__substitutions

from VisualiserProcedural import VisualiserProcedural


import IECore

## An incredibly simple procedural which just renders an object
# passed to it via a Parameter. This is of most use for visualising
# things in Maya using the IECoreMaya.ProceduralHolder.
class VisualiserProcedural( IECore.ParameterisedProcedural ) :

	def __init__( self ) :

		IECore.ParameterisedProcedural.__init__( self )

		self.parameters().addParameters(

			[
				
				IECore.VisibleRenderableParameter(
					"renderable",
					"The object to visualise",
					IECore.Group()
				)
				
			]

		)

	def doBound( self, args ) :

		return args["renderable"].bound()

	def doRenderState( self, renderer, args ) :
	
		pass

	def doRender( self, renderer, args ) :

		args["renderable"].render( renderer )

IECore.registerObject( VisualiserProcedural, 100027 )

from IDXReader import IDXReader


import re

import IECore

class IDXReader( IECore.Reader ) :

	def __init__( self, fileName=None ) :
	
		IECore.Reader.__init__(
			self,
			"Reads Leica Geosystems IDX files"
		)

		if fileName is not None :
			self["fileName"].setTypedValue( fileName )

	@staticmethod
	def canRead( fileName ) :
	
		try :
			f = open( fileName, "r" )
			return f.read( 6 )=="HEADER"
		except :
			return False
		
	def doOperation( self, args ) :
		
		f = open( args["fileName"].value, "r" )
		
		l = "".join( f.readlines() )
			
		dbMatch = re.search( "^DATABASE(.*)END DATABASE", l, re.MULTILINE | re.DOTALL )
		if dbMatch is None :
			raise RuntimeError( "Unable to find database block in file \"%s\"" % args["fileName"].value )
	
		headerMatch = re.search( "^HEADER(.*)END HEADER", l, re.MULTILINE | re.DOTALL )
		if headerMatch is None :
			raise RuntimeError( "Unable to find header block in file \"%s\"" % args["fileName"].value )
		
		projMatch = re.search( "PROJECT(.*?)END PROJECT", headerMatch.group( 1 ), re.MULTILINE | re.DOTALL )
		if projMatch is None :
			raise RuntimeError( "Unable to find project block in file \"%s\"" % args["fileName"].value )
		
		theoMatch = re.search( "^THEODOLITE(.*?)END THEODOLITE", l, re.MULTILINE | re.DOTALL )
		if theoMatch is None :
			raise RuntimeError( "Unable to find theodolite block in file \"%s\"" % args["fileName"].value )
		
		pointsMatch = re.search( "POINTS\(([^)]*)\)(.*?)END POINTS", dbMatch.group( 1 ), re.MULTILINE | re.DOTALL )
		if pointsMatch is None :
			raise RuntimeError( "Unable to find points block in file \"%s\"" % args["fileName"].value )
		
		annotationMatch = re.search( "ANNOTATIONS\(([^)]*)\)(.*?)END ANNOTATIONS", dbMatch.group( 1 ), re.MULTILINE | re.DOTALL )
		if annotationMatch is None :
			raise RuntimeError( "Unable to find annotation block in file \"%s\"" % args["fileName"].value )
			
		setupSlopeMatch = re.finditer( "SETUP(.*?)END SETUP.*?SLOPE\(([^)]*)\)(.*?)END SLOPE", theoMatch.group( 1 ), re.MULTILINE | re.DOTALL )
		if setupSlopeMatch is None :
			raise RuntimeError( "Unable to setup/slope block in file \"%s\"" % args["fileName"].value )

		# Extract the Points from the database, these all have 'absolute' positions based on where the
		# Station was registered as being.
		points = self.__extractRows( pointsMatch.group(1), pointsMatch.group(2), "PointNo" )
		if not points :
			raise ValueError, "No points in file..."	
		
		# Extract any annotations in the points database
		annotations = self.__extractRows( annotationMatch.group(1), annotationMatch.group(2), "PointNo" )

		# Find out a little about our project
		projInfo = self.__extractFields( projMatch.group(1) )
		
		# We're going to return  a group of PointsPrimitives, one for each station.		
		g = IECore.Group()		
			
		for s in setupSlopeMatch:
						
			try:
				members = self.__extractRows( s.group(2), s.group(3), "TgtNo" )
			except ValueError :
				continue		
			
			p = IECore.V3fVectorData()
			ids = IECore.StringVectorData()
			nos = IECore.IntVectorData()
			dates = IECore.StringVectorData()
			codes = IECore.StringVectorData()
			annotations = IECore.StringVectorData()

			for k in members.iterkeys():
				
				if k not in points :
					continue
					
				if "East" not in points[k] or "North" not in points[k] or "Elevation" not in points[k]:
					# some rows seem to have missing data - not much we can do about that
					continue
				
				try :	
					x = float( points[k]["East"] )
					y = float( points[k]["Elevation"] )
					z = -float( points[k]["North"] ) # Handedness...
				except ValueError:
					continue;
					
				p.append( IECore.V3f( x, y, z ) )
				
				nos.append( int(points[k]["PointNo"]) )
				
				ids.append( points[k]["PointID"] if "PointID" in points[k] else "" )
				dates.append( points[k]["Date"] if "Date" in points[k] else "" )
				codes.append( points[k]["Code"] if "Code" in points[k] else "" )
				
				if k in annotations :
					annotations.append( annotations[k]["Annotation"] )
				else :
					annotations.append( "" )
							
			primitive = IECore.PointsPrimitive( p )
			
			# Extract any available station info from the SETUP block
			stnInfo = self.__extractFields( s.group(1) )
			for d in stnInfo.iterkeys() :
				primitive.blindData()[d] = IECore.StringData( stnInfo[d] )				
			
			# Store our station information on the primitive
			if "STN_NO" in stnInfo:
				
				stnNo = stnInfo["STN_NO"]
								
				if stnNo in points :				
					tx = float( points[ stnNo ]["East"] )
					ty = float( points[ stnNo ]["Elevation"] )
					tz = -float( points[ stnNo ]["North"] ) # Handedness...
					primitive.blindData()["STN_POSITION"] = IECore.V3fData( IECore.V3f( tx, ty, tz ) )
					
				if stnNo in annotations:
					primitive.blindData()["STN_ANNOTATION"] = IECore.StringData( annotations[stnNo]["Annotation"] )	
		
			# copy Project Information to the primitive			
			for f in projInfo:
				primitive.blindData()["PROJECT_%s" % f] = IECore.StringData( projInfo[f] )			
						
			primitive["PointID"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, ids )
			primitive["PointNo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, nos )
			primitive["Date"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, dates )
			primitive["Code"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, codes )
			primitive["Annotation"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, annotations )

			if primitive.numPoints > 0 :
				g.addChild( primitive )
			
		return g

	# Extracts a row into a dictionary, or list, depending on wether a keyColumns name is specified
	# \param columnString A comma separated list of column names
	# \param rows A string containing all the rows of data
	# \paran keyColumn (optional) The string name of a column to use as a key in the output. If specified
	# a dictionary will be returned, rather than an ordered list.
	def __extractRows( self, columnString, rows, keyColumn=None ) :
	
		columnNames = [ x.strip() for x in columnString.split( "," ) ]
		numColumns = len( columnNames )
				
		if keyColumn and keyColumn not in columnNames :
			raise ValueError, "Unable to find the requested key column '%s' (%s)" % ( keyColumn, columnNames )
	
		if keyColumn:
			keyIndex = columnNames.index( keyColumn )
			output = {}	
		else :
			output = []
				
		rows = rows.split( "\n" )
		for row in rows :
		
			thisRow = {}
		
			columns = [ x.strip( " \t\r;\"\'" ) for x in row.split( "," ) ]
			if len( columns ) != numColumns :
				continue

			try :
				for i in range(numColumns):
					thisRow[ columnNames[i] ] = columns[i]			
			except ValueError :
				# some rows seem to have missing data - not much we can do about that
				continue

			if keyColumn:
				output[ columns[keyIndex] ] = thisRow	
			else:
				output.append( thisRow )

		return output
	
	# Extracts fields from a simple NAME VALUE line.
	# \param data A string containing the field block
	# \prarm names A list of field NAMEs to extract. Any missing names will be omitted from the
	# returned dictionary.
	# \return A dictionary of NAME : VALUE pairs. All data remains as strings. 
	def __extractFields( self, data, names=None ) :
		
		fields = {}
	
		if names:
			for n in names:
				match = re.search( n+'[\s]+"{0,1}?([:\\/\-,.\w.]+)"{0,1}?', data, re.MULTILINE | re.DOTALL )
				if match:
					fields[n] = match.group(1)	
		else:
		
			lines = data.split( "\n" )
			for l in lines:	
				match = re.search( '([\w.]+)[\s]+"{0,1}?([:\\/\-,.\w. ]+)"{0,1}?', l.strip( " \t\r;\"\'" ), re.MULTILINE | re.DOTALL )
				if match:
					fields[match.group(1)] = match.group(2)

		return fields
	
IECore.registerRunTimeTyped( IDXReader )
IECore.Reader.registerReader( "idx", IDXReader.canRead, IDXReader, IDXReader.staticTypeId() )

from ClassParameter import ClassParameter


import IECore

## The ClassParameter is a specialised CompoundParameter which allows its
# children to be specified by another Parameterised class which is loaded
# using the ClassLoader. This allows one class to easily nest another while
# exposing the other's parameters publicly.
class ClassParameter( IECore.CompoundParameter ) :

	def __init__( self, name, description, searchPathEnvVar, className="", classVersion=0, userData=None ) :
	
		IECore.CompoundParameter.__init__( self, name, description, userData=userData )
		
		self.__classInstance = None
		self.__className = ""
		self.__classVersion = 0
		self.__searchPathEnvVar = searchPathEnvVar
		
		self.setClass( className, classVersion, searchPathEnvVar )
	
	## Return the class being held. If withClassLoaderArgs is True then a tuple is returned
	# in the following form : ( class, className, classVersion, searchPathEnvVar ).	
	def getClass( self, withClassLoaderArgs=False ) :
	
		if withClassLoaderArgs :
			return ( self.__classInstance, self.__className, self.__classVersion, self.__searchPathEnvVar )
		else :	
			return self.__classInstance
	
	## Sets the class being held. The specified class is loaded using a ClassLoader and
	# the class' parameters are added to this parameter as children.		
	def setClass( self, className, classVersion, searchPathEnvVar=None ) :
	
		if ( className, classVersion, searchPathEnvVar ) == ( self.__className, self.__classVersion, self.__searchPathEnvVar ) :
			return
			
		self.__classInstance = None
		self.clearParameters()
		
		searchPathToUse = searchPathEnvVar if searchPathEnvVar is not None else self.__searchPathEnvVar
		
		if className!="" :
		
			loader = IECore.ClassLoader.defaultLoader( searchPathToUse )
			
			self.__classInstance = loader.load( className, classVersion )()
		
			self.addParameters(
				self.__classInstance.parameters().values()
			)
		
		self.__className = className
		self.__classVersion = classVersion
		self.__searchPathEnvVar = searchPathToUse

	@staticmethod
	def _serialise( parameter ) :

		return [

			parameter.__className,
			str( parameter.__classVersion ),
			parameter.__searchPathEnvVar,

		]

	@staticmethod
	def _parse( args, parameter ) :

		parameter.setClass( args[0], int( args[1] ), args[2] )
		del args[0:3]
					
IECore.registerRunTimeTyped( ClassParameter, IECore.TypeId.ClassParameter )
			
IECore.ParameterParser.registerType( ClassParameter.staticTypeId(), ClassParameter._parse, ClassParameter._serialise )

from ClassVectorParameter import ClassVectorParameter


import IECore

## The ClassVectorParameter is similar to the ClassParameter but instead of holding
# a single class it holds many classes, each storing its parameters under a named
# child of the ClassVectorParameter.
class ClassVectorParameter( IECore.CompoundParameter ) :

	def __init__( self, name, description, searchPathEnvVar, classes=[], userData=None ) :
	
		IECore.CompoundParameter.__init__( self, name, description, userData=userData )
		
		self.__searchPathEnvVar = searchPathEnvVar
		self.__namesToInstances = {} # maps parameter names to [ classInstance, className, classVersion ] lists
		
		self.setClasses( classes )
	
	## Returns the name of the environment variable which defines paths to search for child classes on.
	def searchPathEnvVar( self ) :
		
		return self.__searchPathEnvVar
	
	## Returns a list of the classes held as children. These are returned in the same order
	# as the child parameters holding them. If withClassLoaderArgs is True then a list of
	# tuples is returned, with each tuple being of the form ( classInstance, parameterName, className, classVersion ).
	def getClasses( self, withClassLoaderArgs=False ) :
	
		result = []
		for k in self.keys() :
			if withClassLoaderArgs :
				instance = self.__namesToInstances[k]
				result.append( ( instance[0], k, instance[1], instance[2] ) )
			else :
				result.append( self.__namesToInstances[k][0] )
		
		return result
				
	## Sets the classes held as children. Classes must be a list of tuples of the form
	# ( parameterName, className, classVersion ). If any tuple in the list matches an existing
	# child class, then that class will be preserved rather than replaced with a new instance
	# of the same thing.
	def setClasses( self, classes ) :
	
		# validate arguments and figure out what child parameter names we need
	
		assert( isinstance( classes, list ) )
		
		neededNames = set()
		for c in classes :
			assert( isinstance( c, tuple ) )
			assert( len( c ) == 3 )
			assert( isinstance( c[0], str ) )
			assert( isinstance( c[1], str ) )
			assert( isinstance( c[2], int ) )
			if c[0] in neededNames :
				raise ValueError( "Duplicate parameter name \"%s\"" % c[0] )
			neededNames.add( c[0] )
			
		# first remove any existing parameters which we don't need
		
		for parameterName in self.keys() :
			if parameterName not in neededNames :
				self.removeClass( parameterName )
		
		# and then create any new ones we need and reload and reorder existing
		# ones as necessary
		
		for i in range( 0, len( classes ) ) :
		
			# modify or add a parameter for this class
			
			self.setClass( classes[i][0], classes[i][1], classes[i][2] )
			parameter = self[classes[i][0]]
			
			# make sure the parameter has the right order within the whole
			
			self.removeParameter( parameter )
			if len( self ) == i :
				self.addParameter( parameter )
			else :
				keys = self.keys()
				self.insertParameter( parameter, self[keys[i]] )

	## Returns the class instance that the named parameter represents,
	# or if withClassLoaderArgs is True, then returns a tuple of the
	# form ( classInstance, className, classVersion ).
	def getClass( self, parameterOrParameterName, withClassLoaderArgs=False ) :
	
		if isinstance( parameterOrParameterName, basestring ) :
			parameterName = parameterOrParameterName
		else :
			parameterName = parameterOrParameterName.name
		
		if withClassLoaderArgs :
			return tuple( self.__namesToInstances[parameterName] )
		else :
			return self.__namesToInstances[parameterName][0]
	
	## Sets the class held by the named parameter, if no such
	# parameter exists then one will be appended. To insert 
	# a parameter somewhere other than the end, use getClasses()
	# and setClasses().
	def setClass( self, parameterOrParameterName, className, classVersion ) :
	
		if isinstance( parameterOrParameterName, basestring ) :
			parameterName = parameterOrParameterName
			parameter = self.parameter( parameterOrParameterName )
			if not parameter :
				parameter = IECore.CompoundParameter( parameterName, "" )
				self.addParameter( parameter )
		else :
			parameter = parameterOrParameterName
			parameterName = parameter.name
		
		instance = self.__namesToInstances.setdefault( parameterName, [ None, "", 0 ] )

		if [ className, classVersion ] != instance[1:] :

			loader = IECore.ClassLoader.defaultLoader( self.__searchPathEnvVar )
			instance[0] = loader.load( className, classVersion )()
			instance[1] = className
			instance[2] = classVersion

			parameter.clearParameters()
			parameter.addParameters( instance[0].parameters().values() )
			
			# copy user data over:
			parameter.userData().copyFrom( instance[0].parameters().userData() )
	
	## Removes the class held by the named parameter.
	def removeClass( self, parameterName ) :
	
		self.removeParameter( parameterName )
		del self.__namesToInstances[parameterName]
	
	## Returns a good name for a new parameter. It's not compulsory to use this
	# function (any unique name is fine) but it can be useful to keep a consistent
	# naming convention, and it removes the need to come up with unique names some other
	# way.
	def newParameterName( self, prefix="p" ) :
	
		existingNames = set( self.keys() )
		for i in range( 0, len( existingNames ) + 1 ) :
			parameterName = "%s%d" % ( prefix, i )
			if parameterName not in existingNames :
				return parameterName

	@staticmethod
	def _serialise( parameter ) :

		result = [ parameter.__searchPathEnvVar ]
		
		classes = parameter.getClasses( True )
		
		result.append( str( len( classes ) ) )

		result += [ x[1] for x in classes ]
		result += [ x[2] for x in classes ]
		result += [ str( x[3] ) for x in classes ]

		return result
		
	@staticmethod
	def _parse( args, parameter ) :

		parameter.__searchPathEnvVar = args[0]
		del args[0]
		
		numClasses = int( args[0] )
		del args[0]
		
		parameterNames = args[:numClasses]
		del args[:numClasses]
		
		classNames = args[:numClasses]
		del args[:numClasses]
		
		classVersions = [ int( x ) for x in args[:numClasses] ]
		del args[:numClasses]

		parameter.setClasses( zip( parameterNames, classNames, classVersions ) )
					
IECore.registerRunTimeTyped( ClassVectorParameter, IECore.TypeId.ClassVectorParameter )
			
IECore.ParameterParser.registerType( ClassVectorParameter.staticTypeId(), ClassVectorParameter._parse, ClassVectorParameter._serialise )

from CompoundStream import CompoundStream


## A class which acts like a python file object but outputs to several
# underlying files. This is useful for test output as it allows results to
# be output to the terminal as well as a file.
class CompoundStream :

	def __init__( self, streams=() ) :

		self.__streams = tuple( streams )

	def write( self, l ) :

		for s in self.__streams :
			s.write( l )

	def flush( self ) :

		for s in self.__streams :
			s.flush()

from IgnoredExceptions import IgnoredExceptions


## A context object intended for use with python's "with" syntax. It is used
# to replace this idiom :
#
# try :
#	value = someThing["with"]["keys"]["that"]["may"]["not"]["exist"]
# except KeyError :
#	pass
#
# with something slightly more concise and expressive of what is happening :
#
# with IgnoredExceptions( KeyError ) :
#	value = someThing["with"]["keys"]["that"]["may"]["not"]["exist"]
#
class IgnoredExceptions :

	## Accepts a variable number of exception types - these will be silently
	# ignored if they are thrown from within the body of the block.
	def __init__( self, *args ) :

		self.__toIgnore = args

	def __enter__( self ) :

		pass

	def __exit__( self, type, value, traceBack ) :

		if isinstance( value, self.__toIgnore ) :
			return True
				
		if type is not None and issubclass( type, self.__toIgnore ) :
			return True

import ParameterAlgo

# importing internal utility modules and class overwrites
from ObjectOverwriting import *


import IECore

"""
Implement new methods to the Object class.
"""

def __object__deepcopy__(self, memo):
	# call IECore.Object deep copy operator on the object.
	return self.copy()

# add python copy.deepcopy() functionality to the Object class
IECore.Object.__deepcopy__ = __object__deepcopy__

__all__ = []

from OpOverwriting import *


import IECore

"""
Implement new methods to the Op class.
"""

def __opSmartOperator( self, **args ):
	"""
	Smart version of Op.operator function. It accepts python values, simple structures and Data objects
	as values for the Op parameters.
	"""
	for (paramName, paramValue) in args.items():
		if isinstance(paramValue, IECore.Object):
			self[ paramName ].setValue( paramValue )
		else:
			self[ paramName ].setTypedValue( paramValue )

	return self.operate( )

# redefine Op
IECore.Op.__call__ = __opSmartOperator

__all__ = []

from ParameterOverwriting import *


import IECore

"""
Implement new methods to the Parameter and CompoundParameter classes.
"""

def __parameterSmartSetValue( self, value ):
	"""
	Smart setValue operator for Parameter objects. Uses introspection on the given value to define
	how the value will be assigned to the Parameter object.
	"""
	if isinstance( value, IECore.Object ):
		self.setValue( value )

	elif hasattr( self, "setTypedValue" ):
		self.setTypedValue( value )

	else:
		raise TypeError, "Invalid parameter type"

def __compoundParameterSmartSetValue( self, value ):
	"""
	Smart setValue operator for CompoundParameter objects. Uses introspection on the given value to define
	how the value will be assigned to the CompoundParameter object.
	"""

	if isinstance( value, IECore.CompoundObject ):
		self.setValue( value )

	elif isinstance( value, dict ):
		for ( n, v ) in value.items():
			self[ n ].smartSetValue( v )

	else:
		raise TypeError, "Invalid parameter type"

def __compoundParameterSetItem( self, itemName, itemValue ):
	"""
	Smart __setitem__ operator.
	"""
	self[ itemName ].smartSetValue( itemValue )

## \todo Remove this function entirely when we're done testing everything against cortex 5.
def __compoundParameterSetAttr( self, attrName, attrValue ) :

	if attrName in self :
		raise RuntimeError( "Can no longer set child parameter values using attribute syntax." )
	
	IECore.Parameter.__setattr__( self, attrName, attrValue )

# expand class definitions
IECore.Parameter.smartSetValue = __parameterSmartSetValue
IECore.CompoundParameter.smartSetValue = __compoundParameterSmartSetValue
IECore.CompoundParameter.__setattr__ = __compoundParameterSetAttr
IECore.CompoundParameter.__setitem__ = __compoundParameterSetItem

__all__ = []

from ParameterisedOverwriting import *


import warnings

import IECore

"""
Implement new methods to the Parameterised class.
"""

def __parameterisedSetItemOp( self, attrName, attrValue ):
	"""
	Smart version of __setitem__ operator. Uses Parameter.smartSetValue() function for
	dynamic type conversion.
	"""
	try:
		parameters = IECore.Parameterised.parameters( self )
	except:
		# it's probably a derived class and the constructor did not initialized Parameterised.
		# So the attribute must be a class attribute and not a Parameter attribute.
		# \todo Might want to rethink this logic
		self.__dict__[ attrName ] = attrValue
	else:
		if parameters.has_key( attrName ):
			parameters[ attrName ].smartSetValue( attrValue )
		else:
			# allow assignment of other attributes to the object.
			self.__dict__[ attrName ] = attrValue

IECore.Parameterised.__setitem__ = __parameterisedSetItemOp

__all__ = []

from MessageHandlerOverwriting import *


import IECore

__all__ = []

def __enter( self ) :

	IECore.MessageHandler.pushHandler( self )
	
def __exit( self, type, value, traceBack ) :

	poppedHandler = IECore.MessageHandler.popHandler()
	assert( poppedHandler.isSame( self ) )
	
IECore.MessageHandler.__enter__ = __enter
IECore.MessageHandler.__exit__ = __exit


from ConfigLoader import loadConfig


import re
import os
import os.path
import sys
import traceback
import IECore

## This function provides an easy means of providing a flexible configuration
# mechanism for any software. It works by executing all .py files found on
# a series of searchpaths. It is expected that these files will then make appropriate
# calls to objects passed in via the specified contextDict.
# \ingroup python
def loadConfig( searchPaths, contextDict, raiseExceptions = False ) :

	paths = searchPaths.paths
	paths.reverse()
	for path in paths :
		# \todo Perhaps filter out filenames that begin with "~", also? This would
		# exclude certain types of auto-generated backup files.
		pyExtTest = re.compile( "^[^~].*\.py$" )
		for dirPath, dirNames, fileNames in os.walk( path ) :
			for fileName in filter( pyExtTest.search, fileNames ) :
				fullFileName = os.path.join( dirPath, fileName )
				if raiseExceptions:
					execfile( fullFileName, contextDict, contextDict )
				else:
					try :
						execfile( fullFileName, contextDict, contextDict )
					except Exception, m :
						stacktrace = traceback.format_exc()
						IECore.msg( IECore.Msg.Level.Error, "IECore.loadConfig", "Error executing file \"%s\" - \"%s\".\n %s" % ( fullFileName, m, stacktrace ) )

loadConfig( IECore.SearchPath( os.environ.get( "IECORE_CONFIG_PATHS", "" ), ":" ), { "IECore" : IECore } )

from Preset import Preset

import IECore


## The Preset class serves as a base class for the implementation of
## 'presets'. In a nutshell, they are callable classes that manipulate
## the parameters of a Parameterised object. \see BasicPreset for an
## implementation that provides parameter value loading an saving, with
## support for Class and ClassVector parameters.
##
## Presets themselves are Parametersied objects, to allow them to have
## their own parameters to control how they might be applied.
class Preset( IECore.Parameterised ) :
 	
	def __init__( self, description="" ) :
	
		IECore.Parameterised.__init__( self, description )
	
	## \return Presets may return a dictionary of arbitrary metadata
	## to describe their contents/function. The default implementation
	## simply sets "title" to the class name.
	def metadata( self ) :
	
		return { "title" : self.__class__ }
		
 	## \return True if the Preset can be applied to the given rootParameter
	## on the given parameterised object, otherwise False
	def applicableTo( self, parameterised, rootParameter ) :
		
		raise NotImplementedError
	
	## Applies the preset to the specified parameterised and 
	## root parameter.
	def __call__( self, parameterised, rootParameter ) :
		
		raise NotImplementedError

IECore.registerRunTimeTyped( Preset )


from BasicPreset import BasicPreset


import IECore

import os
import re

## Implements a Preset to permit values to be saved and restored 
## from a Parameterised object. BasicPresets can be created either
## as in-memory representations of the parameters, or saved to disk.
##
## The BasicPreset has a single parameter
##   "overwriteMatchingComponents" : Bool, which controls how 
##      ClassVector parameter are treated, when the preset is applied.
class BasicPreset( IECore.Preset ) :

	## The constructor is essentially in two forms:
	## 
	##    IECore.BasicPreset( parameterised, rootParameter=None, parameters=(), referenceData=False )
	##
	## This is the most common form, and should be used to create a new preset from the
	## given parameterised holding object.
	##
	##    IECore.BasicPreset( pathOrData )
	##  
	## This form is used to restore data into a preset for application, and should rarely
	## be used directly.
	##
	## \param pathOrData, this should be an absolute path to a CompoundObject on disk or a 
	## CompoundObject pointer itself. This object should contain the data structure for the preset.
	## \param parameterised, The Parameterised object holding the parameters to be saved.
	## \param rootParameter, IECore.Parameter, Where to start in the parmameter hierarchy. 
	## \param parameters, ( IECore.Parameter, ... ), A list of Parameters to include in the
	##              the preset. This allow certain values not to be included in the preset.
	## \param referenceData, bool, When enabled, this stops the preset mechanism from
	##              copying the value data from the parameters it encapsulates. This can save memory
	##              when the preset is to be written straight to disk. The default behaviour
	##				copies any parameter values so the preset is not dependent on the source
	##				parameters state at the time of application.
	def __init__( self, pathOrDataOrParameterised, rootParameter=None, parameters=(), referenceData=False ) :
		
		self._header = None
		self._data = None
		self._cob = None
		
		IECore.Preset.__init__( self )
				
		self.parameters().addParameters(
		
			[
				IECore.BoolParameter(
					name = "overwriteMatchingComponents",
					description = "When off, the preset will always append items to a " + \
					"ClassVectorParameter, otherwise, it will replace the existing " + \
					"classes with the same names, if they don't match the preset. " + \
					"This does not affect and parameter values, these are always set " + \
					"to match the preset.",
					defaultValue = False
				),
			]
		)
		
		if isinstance( pathOrDataOrParameterised, str ) or isinstance( pathOrDataOrParameterised, unicode ) :
			
			self._cob = pathOrDataOrParameterised
		
		elif isinstance( pathOrDataOrParameterised, IECore.CompoundObject ) :
		
			self._data = pathOrDataOrParameterised	
		
		elif hasattr( pathOrDataOrParameterised, "parameters" ):
			
			data = IECore.CompoundObject()
			
			if rootParameter is None:
				rootParameter = pathOrDataOrParameterised.parameters()
			
			BasicPreset._grabHierarchy( data, rootParameter, parameters )
			
			# We need to prune any class entries without parameters, so that
			# we don't meddle with classes the user asked us not to copy parameters for.
			BasicPreset._pruneHierarchy( data )
			
			if referenceData:
				self._data = data
			else:
				self._data = data.copy()
								
		else :		
			
			raise ValueError, "IECore.BasicPreset.__init__: Unsupported object passed: %s." % pathOrDataOrParameterised
		
	## \return a dictionary of metatdata about the preset. BasicPresets 
	## provide the following keys, when a preset has been saved to disk.
	## NOTE: Presets created by the 'Copy' method will not contain any
	## pertinent information in theses fields:
	##
	##   "title" : string, The user supplied name the preset.
	##   "description" : string, A multi-line string of arbitrary descriptive text.
	##   "categories" : ( string, .. ), A list of strings, one for each category
	##                  the preset is considered part of.
	def metadata( self ) :
		
		self._ensureHeader()
		
		h = self._header
		return {
			"title" : h["title"].value if "title" in h else self.__class__,
			"description" : h["description"].value if "description" in h else "",
			"categories" : list( h["categories"] ) if "categories" in h else (),
		}

	## \see IECore.Preset.applicableTo	
	def applicableTo( self, parameterised, rootParameter ) :
		
		self._ensureData()	
		return self._applicableTo( parameterised, rootParameter, self._data )

	## \see IECore.Preset.__call__
	# \param parameterList A list of Parameter pointers that the preset should apply to. 
	# \param parameterListExcludes A bool, which when True, will treat the parameterList as a
	# 'skip' list, rather than an 'application' list.
	# NOTE: When parameterListExcludes is False, all parent parameters of a desired leaf parameter 
	# must be in this list. Otherwise the preset will not consider the parent so will never
	# reach the child.
	def __call__( self, parameterised, rootParameter, parameterList=[], parameterListExcludes=False ) :
			
		self._ensureData()
		
		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "IECore.BasicPreset: Sorry, this preset is not applicable to that parameter."
		
		if parameterList and not parameterListExcludes :
			# Not much point getting out of bed if the root isn't in there...
			if rootParameter not in parameterList:
				# Copy the list so we don't modify the one we were given.
				parameterList = parameterList[:]
				parameterList.append( rootParameter )
						
		self._applyHierarchy( parameterised, rootParameter, self._data, parameterList, parameterListExcludes )
											
	## This method will save the specified parameters to disk in such a was
	## as can be loaded by the IECore.ClassLoader
	## \param path, string, The file system location the preset should be saved to
	##              note: this should be a directory name, not the desired preset name.
	## \param name, string, The name of the preset, the preset will be saved under this
	##              name inside of 'path'. This name is not sanitised, and it is the 
	##              responsibility of the caller to ensure that it is a legal file system name.
	## \param title, string, The title of the preset, no character restrictions.
	## \param description, string, A description of the preset, no character restrictions.
	## \param categories, ( string, ... ) A list of categories the preset should be tagged with
	## \param version, int, the version of the preset, this will default to 1, used when saving
	## for the ClassLoader.
	## \param classLoadable, bool, if True (default) then the preset will be saved in a way that
	## can be loaded by the ClassLoader, otherwise, just a cob file is written containing the
	## presets data.
	def save( self, path, name, title="", description="", categories=(), version=1, classLoadable=True ) :
	
		if not self._data:
			raise RuntimeError, "IECore.BasicPreset.save: Unable to save, preset has no data."
	
		baseDir = path
		cobName = "%s.cob" % ( name, )
		pyFile = None
			
		if classLoadable :
			baseDir = "%s/%s" % ( path, name )
			cobName = "%s-%i.cob" % ( name, version )
			pyFile = "%s/%s-%i.py" % ( baseDir, name, version )
		
		cobFile = "%s/%s" % ( baseDir, cobName )	
			
		if not os.path.isdir( baseDir ) :
			os.makedirs( baseDir )
		
		if not os.path.isdir( baseDir ) :
			raise RuntimeError, "IECore.BasicPreset.save: Unable to create the directory '%s'" % baseDir
		
		w = IECore.Writer.create( self._data, cobFile )

		w["header"].getValue()["title"] = IECore.StringData( title if title else name )
		w["header"].getValue()["description"] = IECore.StringData( description )
		w["header"].getValue()["categories"] = IECore.StringVectorData( categories )
		w["header"].getValue()["dataVersion"] = IECore.IntData( 1 )

		w.write()
		
		if pyFile :	
			BasicPreset._writePy( pyFile, cobName, name )		
	
	def _ensureData( self ) :
		
		if self._data != None:
			return			

		if self._cob is not None:

			data = IECore.Reader.create( self._cob ).read()
			if not isinstance( data, IECore.CompoundObject ) :
				raise RuntimeError, "IECore.BasicPreset: Unable to retrieve data from '%s'." % self._cob 
			self._data = data
		
		if not self._data:
		
			raise RuntimeError, "IECore.BasicPreset: No data in preset." 

	def _ensureHeader( self ) :
		
		if self._cob != None:
			self._header = IECore.Reader.create( self._cob ).readHeader()
		else:
			self._header = {}
				
	@staticmethod
	def _writePy( fileName, cob, className  ) :
				
		f = open( fileName, "w" )
		f.write(
		
"""import IECore
import os.path

class %s( IECore.BasicPreset ):
	
	def __init__( self ):
		dir = os.path.dirname( __file__ )
		IECore.BasicPreset.__init__( self, dir+"/%s"	)

IECore.registerRunTimeTyped( %s )
		
""" % (	className, cob, className )

		)
		
	@staticmethod
	def _grabHierarchy( data, parameter, parameterList=() ) :
				
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :

			for p in parameter.keys() :
			
				data[p] = IECore.CompoundObject()

				BasicPreset._grabHierarchy(
					data[p],
					parameter[p],
					parameterList, 
				 )
				 
		else :
			
			if isinstance( parameter, IECore.ClassParameter ) :
				
				BasicPreset._grabClassParameter( parameter, data, parameterList )
			
			elif isinstance( parameter, IECore.ClassVectorParameter ) :
				
				BasicPreset._grabClassVectorParameter( parameter, data, parameterList )
			
			else :	
						
				# Some parameter types end up with different python instance
				# due to a boost bug, so 'if p in parameterList' fails.
				if parameterList:
					for p in parameterList:
						if parameter.isSame( p ) :		
							BasicPreset._grabParameter( parameter, data )
							break
				else :
					BasicPreset._grabParameter( parameter, data )
	
	@staticmethod	
	def _grabParameter( parameter, data ) :
		
		data["_value_"] = parameter.getValue()
	
	@staticmethod	
	def _grabClassParameter( parameter, data, parameterList ) :
		
		c = parameter.getClass( True )
		
		data["_className_"] = IECore.StringData( c[1] )
		data["_classVersion_"] = IECore.IntData( c[2] )
		data["_classSearchPaths_"] = IECore.StringData( c[3] )

		classNameFilter = "*"
		try :
			classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		data["_classNameFilter_"] = IECore.StringData( classNameFilter )

		data["_classValue_"] = IECore.CompoundObject()

		if c[0] :
			
			# Some classes may have no parameters, if they have been
			# specifically included in the parameter list, then we
			# want to save their instance specification anyway.
			if len( c[0].parameters() ) :

				BasicPreset._grabHierarchy(
					data["_classValue_"],
					c[0].parameters(),
					parameterList, 
				)
			
			elif parameterList :
		
				for p in parameterList:
					if parameter.isSame( p ) :			
						data["_noPrune_"] = IECore.BoolData( True )
				
			else :
			
				data["_noPrune_"] = IECore.BoolData( True )
			
	
	@staticmethod		
	def _grabClassVectorParameter( parameter, data, parameterList ) :
		
		classes = parameter.getClasses( True )
				
		data["_classSearchPaths_"] = IECore.StringData( parameter.searchPathEnvVar() )

		classNameFilter = "*"
		try :
			classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		data["_classNameFilter_" ] = IECore.StringData( classNameFilter )

		data["_classNames_"] = IECore.StringVectorData()
		data["_classVersions_"] = IECore.IntVectorData()
		data["_classOrder_"] = IECore.StringVectorData()

		data["_values_"] = IECore.CompoundObject()

		for c in classes:

			data["_classOrder_"].append( c[1] )
			data["_classNames_"].append( c[2] )
			data["_classVersions_"].append( c[3] )

			v = IECore.CompoundObject()

			BasicPreset._grabHierarchy(
				v,
				c[0].parameters(),
				parameterList, 
			)			

			data["_values_"][c[1]] = v 		
		
	def _applyHierarchy( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
				
		if parameterList :
			if invertList : # its a 'skipList'
				if parameter in parameterList :
					return
			else :
				if parameter not in parameterList :
					return
		
		if "_className_" in data :
							
			self._applyClassParameter( parameterised, parameter, data, parameterList, invertList )
			
		elif "_classNames_" in data :
		
			self._applyClassVector( parameterised, parameter, data, parameterList, invertList )
		
		elif "_value_" in data :
				
			self._applyParameter( parameterised, parameter, data )
			
		else : # CompoundParameter
					
			for p in data.keys() :
			
				if p not in parameter :
					IECore.msg( 
						IECore.Msg.Level.Warning, 
						"IECore.BasicPreset", 
						"'%s' is missing from '%s' (%s)" % ( p, parameter.name, parameter ) 
					)
					continue
				
				self._applyHierarchy( parameterised, parameter[p], data[p], parameterList, invertList )
			
	def _applyParameter( self, parameterised, parameter, data ) :
		
		try:
			parameter.setValue( data["_value_"] )
		except Exception, e:
			IECore.msg( IECore.Msg.Level.Warning, "IECore.BasicPreset", str(e) )
		
	def _applyClassParameter( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
		
		if not isinstance( parameter, IECore.ClassParameter ) :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.BasicPreset", 
				"Unable to restore to '%s' (%s) as it isnt a ClassParameter"
					% ( parameter.name, parameter )
			)
			return
		
		c = parameter.getClass( True )
		
		className = data["_className_"].value
		classVersion = data["_classVersion_"].value
		classPaths = data["_classSearchPaths_"].value
					
		if c[1] != className or c[2] != classVersion or c[3] != classPaths:
			parameter.setClass( className, classVersion, classPaths )
			
		c = parameter.getClass( False )
		if c and "_classValue_" in data :	
			self._applyHierarchy( parameterised, c.parameters(), data["_classValue_"], parameterList, invertList )		
		
	def _applyClassVector( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
		
		if not isinstance( parameter, IECore.ClassVectorParameter ) :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.BasicPreset",
				"Unable to restore to '%s' (%s) as it isnt a ClassVectorParameter"
					% ( parameter.name, parameter )
			)
			return
		
		overwrite = self.parameters()["overwriteMatchingComponents"].getTypedValue()
				
		names = data["_classNames_"]
		versions = data["_classVersions_"]
		paramNames = data["_classOrder_"]
		
		for i in range( len( data["_classNames_"] ) ) :
	
			# We still have the class information, even if
			# there were no parameter values saved.
			if paramNames[i] not in data["_values_"] :
				continue
		
			if overwrite:
				
				c = None
				
				if paramNames[i] in parameter:			
					c = parameter.getClass( paramNames[i], True )
				
				if not c or c[1:] != ( paramNames[i], names[i], versions[i] ) :
					parameter.setClass( paramNames[i], names[i], versions[i] )
				
				c = parameter.getClass( paramNames[i], True )
			
			else:

				c = self._addClassToVector(
					parameter, 
					paramNames[i],
					names[i],
					versions[i],
				)
				
				
			self._applyHierarchy(
				parameterised,
				c[0].parameters(),
				data["_values_"][ paramNames[i] ],
				parameterList,
				invertList
			)
		
	def _addClassToVector( self, parameter, parameterName, className, classVersion ) :
			
		classes = parameter.getClasses( True )
		parameterNames = [ c[1] for c in classes ]
		if parameterName in parameterNames:
			parameterName = parameter.newParameterName()
		
		parameter.setClass( parameterName, className, classVersion )
		return parameter.getClass( parameterName, True )

	def _applicableTo( self, parameterised, parameter, data ) :
				
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
			
			if "_classValue_" in data or "_values_" in data:
				return False
			
			for k in data.keys():
				if k not in parameter:
					return False
			
		elif isinstance( parameter, IECore.ClassParameter ) :
			
			if "_className_" not in data:
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
								
		elif isinstance( parameter, IECore.ClassVectorParameter ) :
			
			if "_classNames_" not in data:
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
				
			if data["_classSearchPaths_"].value != parameter.searchPathEnvVar() :
				return False
			
		else :
			
			if "_value_" not in data:
				return False
								
			if not parameter.valueValid( data["_value_"] )[0]:
				return False		
		
		return True
		
	@staticmethod
	def _pruneHierarchy( data ) :
	
		returnVal = True
	
		for k in data.keys() :
		
			if k == "_value_" or k == "_noPrune_": 
			
				returnVal = False
				
			elif isinstance( data[k], IECore.CompoundObject ):
			
				if BasicPreset._pruneHierarchy( data[k] ) :
					del data[k]
				else :
					returnVal = False			
		
		return returnVal		

IECore.registerRunTimeTyped( BasicPreset )
	

from RelativePreset import RelativePreset


import IECore

import os
import re

## Implements a Preset that represents changes between two Parameter objects.
# The comparison on elements in a ClassVectorParameters takes in consideration both the parameter name and 
# the loaded class name in order to consider the "same" element. We do that do try to work around the fact
# that the parameter names ("p0", "p1", etc) are very simple and easy to reapper after a sequence of removal/addition
# operations in a ClassVectorParameter. The method is not 100% safe but should work for most cases.
# \todo Consider adding a protected member that is responsible for that comparison and enable derived classes to
# do other kinds of comparisons, for example, using additional parameters such as user labels.
#
class RelativePreset( IECore.Preset ) :

	## \param currParameter, IECore.Parameter, represents the parameter state after all changes have been made. 
	## \param oldParameter, IECore.Parameter, represents the parameter state before any changes. 
	def __init__( self, currParameter=None, oldParameter=None ) :
		
		IECore.Preset.__init__( self )

		self.__data = IECore.CompoundObject()

		# accepts no parameters at all.
		if currParameter is None and oldParameter is None :
			return

		if not isinstance( currParameter, IECore.Parameter ) :
			raise TypeError, "Parameter currParameter must be a IECore.Parameter object!"

		if not oldParameter is None :

			if not isinstance( oldParameter, IECore.Parameter ) :
				raise TypeError, "Parameter oldParameter must be a IECore.Parameter object!"

			if currParameter.typeId() != oldParameter.typeId() :
				raise TypeError, "Mismatching types for currParameter and oldParameter!"

		RelativePreset.__grabParameterChanges( currParameter, oldParameter, self.__data )
	
	## \see IECore.Preset.applicableTo	
	def applicableTo( self, parameterised, rootParameter ) :
		
		return RelativePreset.__applicableTo( rootParameter, self.__data )

	def getDiffData( self ):
		"""Returns a IECore.CompoundObject instance that contains the description of all the differences between the two parameters provided when creating this preset."""
		return self.__data.copy()

	def setDiffData( self, data ):
		"""Use this function to recreate a RelativePreset from data previously returned by getDiffData()."""
		if not isinstance( data, IECore.CompoundObject ):
			raise TypeError, "Invalid data type! Must be a IECore.CompoundObject"

		self.__data = data.copy()

	## \see IECore.Preset.__call__
	def __call__( self, parameterised, rootParameter ) :

		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "Sorry, this preset is not applicable to the given parameter."
		
		if len( self.__data ) :
			self.__applyParameterChanges( rootParameter, self.__data )

	@staticmethod
	def __grabParameterChanges( currParameter, oldParameter, data, paramPath = "" ) :

		if not oldParameter is None:

			if currParameter.staticTypeId() != oldParameter.staticTypeId() :
				raise Exception, "Incompatible parameter %s!" % paramPath

		if isinstance( currParameter, IECore.ClassParameter ) :

			RelativePreset.__grabClassParameterChanges( currParameter, oldParameter, data, paramPath )
			
		elif isinstance( currParameter, IECore.ClassVectorParameter ) :
			
			RelativePreset.__grabClassVectorParameterChanges( currParameter, oldParameter, data, paramPath )
			
		elif isinstance( currParameter, IECore.CompoundParameter ) :
			
			RelativePreset.__grabCompoundParameterChanges( currParameter, oldParameter, data, paramPath )

		else :

			RelativePreset.__grabSimpleParameterChanges( currParameter, oldParameter, data, paramPath )

	@staticmethod
	def __grabCompoundParameterChanges( currParameter, oldParameter, data, paramPath ) :

		for p in currParameter.keys() :
			
			newData = IECore.CompoundObject()
			childOldParam = None
			if not oldParameter is None :
				if p in oldParameter.keys() :
					childOldParam = oldParameter[p]

			RelativePreset.__grabParameterChanges(
				currParameter[p],
				childOldParam, 
				newData,
				paramPath + "." + p
			)

			if len(newData) :
				data[p] = newData

		if len(data):
			data["_type_"] = IECore.StringData( "CompoundParameter" )

	@staticmethod
	def __grabSimpleParameterChanges( currParameter, oldParameter, data, paramPath ) :

		if not oldParameter is None :

			if currParameter.getValue() == oldParameter.getValue() :
				return

		data["_type_"] = IECore.StringData( currParameter.typeName() )
		data["_value_"] = currParameter.getValue().copy()
	
	@staticmethod
	def __grabClassParameterChanges( currParameter, oldParameter, data, paramPath ) :
		
		c = currParameter.getClass( True )
		
		className = c[1]
		classVersion = c[2]
		classNameFilter = "*"
		try :
			classNameFilter = currParameter.userData()["UI"]["classNameFilter"].value
		except :
			pass

		oldClassName = None
		oldClassVersion = None
		childOldParam = None
		if not oldParameter is None :
			oldClass = oldParameter.getClass( True )
			oldClassName = oldClass[1]
			oldClassVersion = oldClass[2]
			if oldClass[0] :
				childOldParam = oldClass[0].parameters()

		classValue = IECore.CompoundObject()

		if c[0] :
	
			RelativePreset.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				classValue,
				paramPath
			)

		if len(classValue):
			data["_classValue_"] = classValue

		if len(data) or className != oldClassName or classVersion != oldClassVersion :
			data["_className_"] = IECore.StringData(className)
			data["_classVersion_"] = IECore.IntData(classVersion)
			data["_classNameFilter_"] = IECore.StringData(classNameFilter)
			data["_type_"] = IECore.StringData( "ClassParameter" )
	
	@staticmethod		
	def __grabClassVectorParameterChanges( currParameter, oldParameter, data, paramPath ) :
		
		classes = currParameter.getClasses( True )
				
		classNameFilter = "*"
		try :
			classNameFilter = currParameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		classNameFilter = IECore.StringData( classNameFilter )

		classNames = IECore.StringVectorData()
		classVersions = IECore.IntVectorData()
		classOrder = IECore.StringVectorData()

		values = IECore.CompoundObject()

		for c in classes:

			pName = c[1]
			classOrder.append( pName )
			classNames.append( c[2] )
			classVersions.append( c[3] )

			v = IECore.CompoundObject()

			childOldParam = None
			if not oldParameter is None and pName in oldParameter.keys() :
				oldClass = oldParameter.getClass( pName )
				if oldClass :
					childOldParam = oldClass.parameters()

			RelativePreset.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				v,
				paramPath + "." + pName
			)

			if len(v) :
				values[c[1]] = v

		removedParams = []
		if not oldParameter is None :
			removedParams = list( set( oldParameter.keys() ).difference( classOrder ) )
			if removedParams :
				data["_removedParamNames_"] = IECore.StringVectorData( removedParams )
				data["_removedClassNames_"] = IECore.StringVectorData()
				for pName in removedParams :
					oldClass = oldParameter.getClass( pName, True )
					data["_removedClassNames_"].append( oldClass[1] )

		modifiedParams = IECore.StringVectorData()
		modifiedClassNames = IECore.StringVectorData()
		modifiedClassVersions = IECore.IntVectorData()
		addedParam = IECore.BoolVectorData()

		for i in xrange(0,len(classOrder)):
			pName = classOrder[i]
			cName = classNames[i]
			cVersion = classVersions[i]
			oldClassName = None
			oldClassVersion = None

			if not oldParameter is None :
				try:
					oldClass = oldParameter.getClass( pName, True )
					oldClassName = oldClass[1]
					oldClassVersion = oldClass[2]

				except Exception, e:
					# added parameter...
					pass

			if cName != oldClassName or cVersion != oldClassVersion :
				modifiedParams.append( pName )
				modifiedClassNames.append( cName )
				modifiedClassVersions.append( cVersion )
				added = (oldClassName is None)
				# if we are changing the class type, we have to mark as if we
				# were removing it too
				if cName != oldClassName and not oldClassName is None:
					if not "_removedParamNames_" in data :
						data["_removedParamNames_"] = IECore.StringVectorData()
						data["_removedClassNames_"] = IECore.StringVectorData()
					data["_removedParamNames_"].append(pName)
					data["_removedClassNames_"].append(oldClassName)
					removedParams.append(pName)
					added = True

				addedParam.append( added )
		
		if len(modifiedParams) :
			data["_modifiedParamsNames_"] = modifiedParams
			data["_modifiedClassNames_"] = modifiedClassNames
			data["_modifiedClassVersions_"] = modifiedClassVersions
			data["_addedParam_"] = addedParam

		# get all non-new parameters
		parameterOrder = filter( lambda n: not n in modifiedParams or not addedParam[ modifiedParams.index(n) ], classOrder )
		baseOrder = parameterOrder
		if not oldParameter is None :
			# get all non-deleted original parameters
			baseOrder = filter( lambda n: not n in removedParams, oldParameter.keys() )
			
		if baseOrder != parameterOrder :

			if len(baseOrder) != len(parameterOrder):
				raise Exception, "Unnexpected error. Unmatching parameter lists!"

			# clamp to the smallest list containing the differences
			for start in xrange(0,len(baseOrder)):
				if baseOrder[start] != parameterOrder[start] :
					break
			for endPos in xrange(len(baseOrder),0,-1):
				if baseOrder[endPos-1] != parameterOrder[endPos-1] :
					break

			data["_modifiedOrder_"] = IECore.StringVectorData( parameterOrder[start:endPos] )

		if len(values):
			# keep the original classes to which the parameters were edited
			for pName in values.keys() :
				values[pName]["_class_"] = IECore.StringData( classNames[classOrder.index(pName)] )
			data["_values_"] = values

		if len(data):
			data["_classNameFilter_" ] = classNameFilter
			data["_type_"] = IECore.StringData( "ClassVectorParameter" )
			data["_paramNames_"] = classOrder
			data["_classNames_"] = classNames

	@staticmethod
	def __applyParameterChanges( parameter, data, paramPath = "" ) :

		if isinstance( parameter, IECore.ClassParameter ) :

			RelativePreset.__applyClassParameterChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.ClassVectorParameter ) :

			RelativePreset.__applyClassVectorChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.CompoundParameter ) :

			RelativePreset.__applyCompoundParameterChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.Parameter ) :

			RelativePreset.__applySimpleParameterChanges( parameter, data, paramPath )

		else :
			IECore.msg( 
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unrecognized type (%s) for parameter %s. Not affected by preset." % ( parameter.typeName(), parameter.name )
			)

	@staticmethod		
	def __applyCompoundParameterChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "CompoundParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found CompoundParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		for p in data.keys() :
			if p in [ "_type_", "_class_" ] :
				continue

			if paramPath :
				newParamPath = paramPath + "." + p
			else :
				newParamPath = p

			if p not in parameter :
				IECore.msg( 
					IECore.Msg.Level.Warning, 
					"IECore.RelativePreset", 
					"Could not find parameter '%s'. Preset value ignored." % newParamPath
				)
				continue
				
			RelativePreset.__applyParameterChanges( parameter[p], data[p], newParamPath )
			
	@staticmethod		
	def __applySimpleParameterChanges( parameter, data, paramPath ) :

		if data["_type_"].value != parameter.typeName() :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found %s."
					% ( paramPath, data["_type_"].value, parameter.typeName() )
			)
			return
		
		try:
			parameter.setValue( data["_value_"] )
		except Exception, e:
			IECore.msg( IECore.Msg.Level.Warning, "IECore.RelativePreset", str(e) )
		
	@staticmethod		
	def __applyClassParameterChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "ClassParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found ClassParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		c = parameter.getClass( True )
		
		className = data["_className_"].value
		classVersion = data["_classVersion_"].value

		if c[1] != className or c[2] != classVersion :
			parameter.setClass( className, classVersion )
			
		c = parameter.getClass( False )
		if c :
			RelativePreset.__applyParameterChanges( c.parameters(), data["_classValue_"], paramPath )
		
	@staticmethod
	def __applyClassVectorChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "ClassVectorParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found ClassVectorParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		# remove parameters if they match in parameter name and class name
		if "_removedParamNames_" in data :
			for (i,pName) in enumerate( data["_removedParamNames_"] ):
				if pName in parameter.keys() :
					c = parameter.getClass( pName, True )
					if c and c[1] == data["_removedClassNames_"][i] :
						parameter.removeClass( pName )

		paramRemaps = {}

		if "_modifiedParamsNames_" in data :
			modifiedParams = data["_modifiedParamsNames_"]
			modifiedClassNames = data["_modifiedClassNames_"]
			modifiedClassVersions = data["_modifiedClassVersions_"]
			addedParam = data["_addedParam_"]
			addedCount = 0

			# first modify items
			for i in range( len( modifiedClassNames ) ) :
	
				if addedParam[i] :

					addedCount += 1
		
				else :

					# must find an existing matching parameter, no matter what
					if modifiedParams[i] in parameter:
						c = parameter.getClass( modifiedParams[i], True )
						if modifiedClassNames[i] == c[1] :
							if modifiedClassVersions[i] != c[2] :
								parameter.setClass( modifiedParams[i], modifiedClassNames[i], modifiedClassVersions[i] )
						else :
							IECore.msg(
								IECore.Msg.Level.Warning, 
								"IECore.RelativePreset", 
								"Parameter '%s.%s' has a different class. Expected %s but found %s. Ignoring class change on this parameter."
									% ( paramPath, modifiedParams[i], modifiedClassNames[i], c[1] )
							)
					else :
						IECore.msg(
							IECore.Msg.Level.Warning, 
							"IECore.RelativePreset", 
							"Unable to find parameter '%s.%s' in %s. Ignoring class change on this parameter."
								% ( paramPath, modifiedParams[i], parameter.name )
						)

		# get a list of classes before the addition of new items
		newOrder = False
		newClassList = map( lambda c: c[1:], parameter.getClasses( True ) )
		newParamList = map( lambda c: c[0], newClassList )
		# compare each class with whatever existed when we created the RelativePreset and see which ones are the same
		sameClasses = set()
		for c in newClassList :

			if '_modifiedParamsNames_' in data :
				# If the preset has added this parameter it should not match current parameters in the vector, no matter if the class matches. Is it always the case?
				if c[0] in data['_modifiedParamsNames_'] :
					if data['_addedParam_'][ data['_modifiedParamsNames_'].index(c[0]) ] :
						continue

			try :
				i = data['_paramNames_'].index(c[0])
			except :
				continue
			if c[1] == data['_classNames_'][i] :
				sameClasses.add( c[0] )

		if "_modifiedOrder_" in data :
			# there was some kind of change in the order of parameters as well...

			modifiedOrder = filter( lambda pName: pName in sameClasses, data["_modifiedOrder_"] )

			# find the range of parameters that lie between the reordered parameters in the current vector
			firstParam = None
			lastParam = None
			for (i,pName) in enumerate(newParamList) :
				if pName in modifiedOrder :
					if firstParam is None:
						firstParam = i
					lastParam = i
			
			if firstParam != lastParam :

				# adds one by one the unknown parameters that lied between the reordered parameters.
				for pName in newParamList[firstParam:lastParam+1] :
					if not pName in modifiedOrder :
						modifiedOrder.insert( modifiedOrder.index(baseParam)+1, pName )
					baseParam = pName

				def classOrder( c1, c2 ):
					# if both elements were on the original reordering operation we use their relationship
					if c1[0] in modifiedOrder and c2[0] in modifiedOrder:
						i1 = modifiedOrder.index( c1[0] )
						i2 = modifiedOrder.index( c2[0] )
						return cmp( i1, i2 )

					# otherwise we use the current order.
					i1 = newParamList.index( c1[0] )
					i2 = newParamList.index( c2[0] )
					return cmp( i1, i2 )

				newClassList.sort( classOrder )
				newParamList = map( lambda c: c[0], newClassList )
				newOrder = True

		if "_modifiedParamsNames_" in data :
			# now add items to the appropriate spot in the newClassList and newParamList
			if addedCount :

				newOrder = True
				prevActualParam = None
				lastActualParamInsertion = None
				currClasses = parameter.getClasses( True )

				for pName in data["_paramNames_"] :
					if pName in sameClasses :
						if pName in newParamList :
							prevActualParam = pName
							continue
					if pName in modifiedParams :
						i = modifiedParams.index(pName)
						if addedParam[ i ] :
							if prevActualParam is None :
								if lastActualParamInsertion is None :
									# Here we assume that the new parameter should
									# go to the top because its predecessors don't exist on the
									# new vector. Maybe it could also print a warning message..
									lastActualParamInsertion = 0
								else :
									lastActualParamInsertion += 1
							else :
								lastActualParamInsertion = newParamList.index( prevActualParam ) + 1
								prevActualParam = None

							if pName in parameter:

								newParamName = parameter.newParameterName()

								if not re.match("^p[0-9]+$", pName) :
									IECore.msg(
										IECore.Msg.Level.Warning, 
										"IECore.RelativePreset", 
										"Custom parameter %s.%s is being renamed to %s..."
											% ( paramPath, pName, newParamName )
									)		
								paramRemaps[ pName ] = newParamName
								pName = newParamName
							# add the parameter to the vector, so that next calls to parameter.newParameterName() will work.
							parameter.setClass( pName, modifiedClassNames[i], modifiedClassVersions[i] )
							# update our official new arrays
							newParamList.insert(lastActualParamInsertion, pName)
							newClassList.insert(lastActualParamInsertion, (pName,modifiedClassNames[i], modifiedClassVersions[i]) )


		# update parameters with new order
		if newOrder :
			parameter.setClasses( newClassList )

		if "_values_" in data :

			for paramName in data["_values_"].keys() :

				remapedParamName = paramRemaps.get( paramName, paramName )
				presetValue = data["_values_"][paramName]

				if remapedParamName in parameter.keys() :
					c = parameter.getClass( remapedParamName, True )
					if c[1] == presetValue["_class_"].value :

						RelativePreset.__applyParameterChanges(
							c[0].parameters(),
							presetValue,
							paramPath + "." + remapedParamName
						)
					else :
						IECore.msg(
							IECore.Msg.Level.Warning, 
							"IECore.RelativePreset", 
							"Ignoring preset values for parameter %s.%s. Expected class %s but found %s."
								% ( paramPath, remapedParamName, presetValue["_class_"].value, c[1] )
						)

				else :
					IECore.msg(
						IECore.Msg.Level.Warning, 
						"IECore.RelativePreset", 
						"Unable to find parameter '%s.%s' in %s. Ignoring this preset changes."
							% ( paramPath, remapedParamName, parameter.name )
					)
		
	@staticmethod
	def __applicableTo( parameter, data ) :
		
		if len(data) == 0 :
			return True

		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
			
			if data["_type_"].value != "CompoundParameter":
				return False
			
		elif isinstance( parameter, IECore.ClassParameter ) :
			
			if data["_type_"].value != "ClassParameter":
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False

		elif isinstance( parameter, IECore.ClassVectorParameter ) :
			
			if data["_type_"].value != "ClassVectorParameter":
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
				
		else :
			
			if data["_type_"].value != parameter.typeName():
				return False

			if not parameter.valueValid( data["_value_"] )[0]:
				return False		
		
		return True


IECore.registerRunTimeTyped( RelativePreset )
	


