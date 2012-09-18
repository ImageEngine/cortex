
## \file IECore/StringUtil.py
#
# This file defines useful string related functions.
#
# \ingroup python

from urllib import quote, unquote
import shlex

## Returns a new string which is the old string with word wrapping
# performed so that no lines are longer than width.
def wrap( text, width ) :

	# i got this off the internet. i suspect it might not be very fast
	# for a lot of text.
	return reduce(lambda line, word, width=width: '%s%s%s' %
                  (line,
                   ' \n'[(len(line)-line.rfind('\n')-1
                         + len(word.split('\n',1)[0]
                              ) >= width)],
                   word),
                  text.split(' ')
                 )

## Encodes shell command line arguments treating special characters.
def quoteCmdLineArg( arg ):

	# tread empty string first
	if len(arg) == 0:
		return "''"

	# eliminate characters we know that works on shell command lines
	exceptions = [ ":", " ", "#" ]
	simpleArg = arg
	for exception in exceptions :
		simpleArg = simpleArg.replace( exception, "" )

	quotedTxt = quote( simpleArg )
	if quotedTxt != simpleArg :
		# we still need quoting...
		return quote( arg )

	# no spaces, no single quotes
	if arg.find(" ") == -1:
		return arg

	# add single quote
	return "'%s'" % arg

## Reverts the transformations from quoteCmdLineArg
def unquoteCmdLineArg( arg ):

	# eliminate single quotes
	if arg.startswith("'") and arg.endswith("'"):
		return arg[1:-1]

	return unquote( arg )

## Applies special quoting on the given argument list
# Uses quoteCmdLineArg() on each of the arguments
def quoteCmdLineArgs( args ):
	return map(lambda x: quoteCmdLineArg(x), args)

## Builds a single command line string from the given argument list.
# You can use this function together with ParameterParser.serialise when building a command line.
# It is better than pipes.quote() because it is recursive in nature gives more 
# compact representation for single quote characters and does not break with unbalanced quotes (example: """ 'a" """ ).
# Just make sure to call unquoteCmdLineArgs or unquoteCmdLine when parsing the arguments.
def quotedCmdLine( args ):
	return " ".join( quoteCmdLineArgs( args ) )

## Reverts any encoding applied to special characters in the given argument list.
# Uses unquoteCmdLineArg() on each list item.
def unquoteCmdLineArgs( args ):
	return map( lambda x: unquoteCmdLineArg(x), args )

## From a given command line, splits the parameters and unquote them.
def unquoteCmdLine( cmdLine ):
	return unquoteCmdLineArgs( shlex.split( cmdLine ) )

__all__ = [ 'wrap', 'quoteCmdLineArg', 'unquoteCmdLineArg', 'quotedCmdLine', 'quoteCmdLineArgs', 'unquoteCmdLineArgs' ]
