import os
from IECore import *

class CheckImagesOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "CheckImagesOp",
			"""This Op checks an image file sequence for corrupted and missing files. It also warns of abrupt file size changes.
There will be error messages for each frame that is corrupt or missing and warning messages for each frame that has abrupt size changes. The Op will raise an error if there's one or more error messages.""",
			IntParameter(
				name = "result",
				description = "Returns the number of warning messages.",
				defaultValue = 0,
			)
		)
		
		self.userData()["UI"] = CompoundObject(
			{
				"infoMessages": BoolData( True ),
			}
		)

		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "input",
					description = "The image sequence to be tested.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = "dpx exr cin tif tiff jpeg jpg",
				),
				BoolParameter(
					name = 'checkMissingFrames',
					description = "If True the Op will make sure there are no missing frames in the file sequence.\nIt will assume your file sequence should have continous frame numbers. Notice that if the last or previous frames are not there, this Op won't be able to recognize it.",
					defaultValue = True,
				),
				IntParameter(
					name = 'expectedSizeChange',
					description = "Percentage number that sets what would be considered a normal file size change between the frames. It is enforced from the previous and next frames.\nFor example, 30% means that frame 2 on the sequence 1,2 and 3 cannot be 30% bigger than 1 and 3. Frames that don't pass this test will be shown on the warning messages.",
					defaultValue = 30,
					minValue = 0,
					maxValue = 1000,
				),
			]
		)

	def doOperation( self, args ) :

		checkMissing = args.checkMissingFrames.value
		maxChange = 1.0 + (args.expectedSizeChange.value / 100.0 )

		src = self.parameters()["input"].getFileSequenceValue()

		errorCount = 0
		warningCount = 0

		frames = src.frameList.asList()
		frames.sort()
		firstFrame = frames[0]
		lastFrame = frames[-1]
		previousSize = -1
		previousGoodFrame = -1

		info( "Checking file sequence:", src.fileName )

		for f in xrange( firstFrame, lastFrame + 1 ):
			frameFile = src.fileNameForFrame( f )
			if checkMissing and not os.path.exists( frameFile ):
				error( "Frame %d is missing!" % f )
				errorCount += 1
				continue

			try:
				ft = os.stat( frameFile )

				reader = Reader.create( frameFile )
				if not isinstance( reader, ImageReader ):
					raise Exception, "Not an image file!"

				if not reader.isComplete():
					error( "Frame %d is corrupted!" % f )
					errorCount += 1
					continue

			except Exception, e:
				error( "Frame %d cannot be loaded:" % f, e )
				errorCount += 1
				continue

			currentSize = ft.st_size
			if previousSize != -1:
				currentRatio = float(currentSize)/previousSize
				if currentRatio > maxChange:
					warning( "Frame", f, "looks broken because it is \%%d" % int(currentRatio*100), "bigger then frame", previousGoodFrame )
					warningCount += 1

				previousRatio = previousSize/float(currentSize)
				if previousRatio > maxChange:
					warning( "Frame", f, "looks broken because frame", previousGoodFrame, "is \%%d bigger." % int(currentRatio*100) )
					warningCount += 1

			previousSize = currentSize
			previousGoodFrame = f	

		info( "Errors:", errorCount, "Warnings:", warningCount )
		if errorCount:
			raise Exception, "The file sequence did not pass the test."

		return IntData( warningCount )

makeRunTimeTyped( CheckImagesOp, 100017, Op )

