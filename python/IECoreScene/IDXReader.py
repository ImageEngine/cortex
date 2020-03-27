##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

import re
import imath

import IECore
import IECoreScene

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
			raise ValueError( "No points in file..." )

		# Extract any annotations in the points database
		annotations = self.__extractRows( annotationMatch.group(1), annotationMatch.group(2), "PointNo" )

		# Find out a little about our project
		projInfo = self.__extractFields( projMatch.group(1) )

		# We're going to return  a group of PointsPrimitives, one for each station.
		g = IECoreScene.Group()

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

			for k in members.keys() :

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

				p.append( imath.V3f( x, y, z ) )

				nos.append( int(points[k]["PointNo"]) )

				ids.append( points[k]["PointID"] if "PointID" in points[k] else "" )
				dates.append( points[k]["Date"] if "Date" in points[k] else "" )
				codes.append( points[k]["Code"] if "Code" in points[k] else "" )

				if k in annotations :
					annotations.append( annotations[k]["Annotation"] )
				else :
					annotations.append( "" )

			primitive = IECoreScene.PointsPrimitive( p )

			# Extract any available station info from the SETUP block
			stnInfo = self.__extractFields( s.group(1) )
			for d in stnInfo.keys() :
				primitive.blindData()[d] = IECore.StringData( stnInfo[d] )

			# Store our station information on the primitive
			if "STN_NO" in stnInfo:

				stnNo = stnInfo["STN_NO"]

				if stnNo in points :
					tx = float( points[ stnNo ]["East"] )
					ty = float( points[ stnNo ]["Elevation"] )
					tz = -float( points[ stnNo ]["North"] ) # Handedness...
					primitive.blindData()["STN_POSITION"] = IECore.V3fData( imath.V3f( tx, ty, tz ) )

				if stnNo in annotations:
					primitive.blindData()["STN_ANNOTATION"] = IECore.StringData( annotations[stnNo]["Annotation"] )

			# copy Project Information to the primitive
			for f in projInfo:
				primitive.blindData()["PROJECT_%s" % f] = IECore.StringData( projInfo[f] )

			primitive["PointID"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, ids )
			primitive["PointNo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, nos )
			primitive["Date"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, dates )
			primitive["Code"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, codes )
			primitive["Annotation"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, annotations )

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
			raise ValueError( "Unable to find the requested key column '%s' (%s)" % ( keyColumn, columnNames ) )

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
