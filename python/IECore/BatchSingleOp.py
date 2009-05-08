##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

from IECore import *
from urllib import unquote

## Op for executing another Op multiple times with different parameters.
# \todo Move this somewhere IE specific - it's relies on IEFarm too much to be in cortex.
class BatchSingleOp( Op ) :

	## If classLoader is None then ClassLoader.defaultOpLoader() will be used.
	def __init__( self, classLoader = None ) :

		Op.__init__( self,
			"BatchSingleOp",
			"Executes a single Op multiple times.",
			IntParameter(
				name = "result",
				description = "",
				defaultValue = 0
			)
		)

		self.userData()['UI'] = CompoundObject(
			{
				'IEFarm':
					{
						'executeInTheFarm': BoolData( True )
					}
			}
		)

		self.__ops = []
		self.__classLoader = classLoader or ClassLoader.defaultOpLoader()

		self.parameters().addParameters(
			[
				ValidatedStringParameter(
					name = "opName",
					description = "Specifies the full Op name to execute.",
					defaultValue = '',
					regex = ".*",
					regexDescription = "Any character",
					allowEmptyString = False,
				),
				StringVectorParameter(
					name = "opParameters",
					description = "Sets the parameters of each Op formatted in the same syntax as the DO or GUIDO commands. It may contain characters coded in hex for example the space character would be %20.",
					defaultValue = StringVectorData(),
				),
				BoolParameter(
					name = "orderIsImportant",
					description = "Set this On if you want to execute the Ops in the same order they were defined. Set this Off if the Ops can run in parallel and the execution order is not important.",
					defaultValue = False,
				),
			]
		)

	# check parameters ( also used in IEFarm )
	def preCheck( self, operands ):

		opName = operands['opName'].value
		opParameters = operands['opParameters']

		if len( opParameters ) == 0:
			raise Exception, "Not a single Op to run!"

		goodOps = []
		errors = []
		pp = ParameterParser()

		opType = self.__classLoader.load( opName )

		for i in xrange( 0, len(opParameters) ):
			try:
				op = opType()
				pp.parse( unquote(opParameters[i]), op.parameters() )
			except Exception, e:
				errors.append( "Op %02d: %s" % (i,str(e)) )
				continue

			goodOps.append( op )

		if len( errors ):
			raise Exception, "\n".join( errors )

		self.__ops = goodOps
		return operands

	# returns the internal Ops created with the parameter parsing.
	# Values available after calling preCheck function.
	def internalOps( self ):
		return self.__ops

	# multiple jobs ( only used in IEFarm )
	def multipleJobs( self, operands ):
		if operands.orderIsImportant.value:
			# if order is important, keep it as one single job.
			return [ operands ]
		else:
			# if order is not important then breaks each Op on a separate job.
			jobs = []
			for i in xrange( 0, len(operands.opParameters) ):
				newOperands = self.parameters().defaultValue
				newOperands['opName'] = operands.opName
				newOperands['opParameters'] = StringVectorData( [ operands.opParameters[i] ] )
				newOperands['orderIsImportant'] = operands.orderIsImportant
				jobs.append( newOperands )
			return jobs

	def jobName( self, args ):
		return "BatchSingleOp - " + args.opName.value

	def doOperation( self, operands ) :

		operands = self.preCheck( operands )

		for (i,op) in enumerate(self.__ops):
			try:
				op()
			except Exception, e:
				# \todo Stop hiding true reason for exception!
				raise Exception, "Op %02d: %s" % (i,str(e))

		return IntData(1)

registerRunTimeTyped( BatchSingleOp, 100021, Op )
