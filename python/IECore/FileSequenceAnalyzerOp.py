##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

# \ingroup python

import os, copy
from IECore import *

# Base abstract class useful for analyzing file sequences.
# It checks file size changes, missing files and incomplete images.
class FileSequenceAnalyzerOp( Op ):

	def __init__( self, name, description, resultParameter, extensions = [] ):

		Op.__init__( self, name, description, resultParameter )

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

		if args.frameList.value == "":
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
		if args.checkFiles.value and len( nonMissingFrames ):

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
		if args.expectedSizeReduction.value != 100:
			minSizeRatio = 1. - ( args.expectedSizeReduction.value / 100.0 )
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


makeRunTimeTyped( FileSequenceAnalyzerOp, 100019, Op )
