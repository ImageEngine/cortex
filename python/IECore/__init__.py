##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

## \defgroup python Python functionality
#
# Some parts of the IECore library are defined purely in Python. These are shown below.

from _IECore import *
# access by a shorter name for convenience
Msg = MessageHandler
from RunTimeTypedUtil import *
from Log import *
from Formatter import Formatter
from WrappedTextFormatter import WrappedTextFormatter
import StringUtil
from DataTraits import *
from FrameList import FrameList
from FrameRange import FrameRange
from CompoundFrameList import CompoundFrameList
from ReorderedFrameList import ReorderedFrameList
from ReversedFrameList import ReversedFrameList
from BinaryFrameList import BinaryFrameList
from ExclusionFrameList import ExclusionFrameList
from FileSequence import FileSequence
from FileSequenceFunctions import *
from EmptyFrameList import EmptyFrameList
from FileSequenceParameter import FileSequenceParameter
from FrameListParameter import FrameListParameter
from ClassLoader import ClassLoader
from RemoteOpLoader import RemoteOpLoader
from RemovePrimitiveVariables import RemovePrimitiveVariables
from RenamePrimitiveVariables import RenamePrimitiveVariables
from SequenceCpOp import SequenceCpOp
from SequenceLsOp import SequenceLsOp
from SequenceMvOp import SequenceMvOp
from SequenceRmOp import SequenceRmOp
from SequenceRenumberOp import SequenceRenumberOp
from SequenceConvertOp import SequenceConvertOp
from FormattedParameterHelp import formatParameterHelp
from ReadProcedural import ReadProcedural
from ClassLsOp import ClassLsOp
from OptionalCompoundParameter import OptionalCompoundParameter
from FileExaminer import FileExaminer
from NukeFileExaminer import NukeFileExaminer
from RIBFileExaminer import RIBFileExaminer
from FileDependenciesOp import FileDependenciesOp
from CheckFileDependenciesOp import CheckFileDependenciesOp
from PointsExpressionOp import PointsExpressionOp
from Struct import Struct
import Enum
from LsHeaderOp import LsHeaderOp
from curry import curry
from MenuItemDefinition import MenuItemDefinition
from MenuDefinition import MenuDefinition
from ParameterParser import *
from SearchReplaceOp import SearchReplaceOp
from CapturingMessageHandler import CapturingMessageHandler
from FileSequenceAnalyzerOp import FileSequenceAnalyzerOp
from CheckImagesOp import CheckImagesOp
from FileSequenceVectorParameter import FileSequenceVectorParameter
from FileSequenceGraphOp import FileSequenceGraphOp
from BatchSingleOp import BatchSingleOp

# importing internal utility modules and class overwrites
from ObjectOverwriting import *
from OpOverwriting import *
from ParameterOverwriting import *
from ParameterisedOverwriting import *

# calling log initialization function.
initializeLog()

from ConfigLoader import loadConfig
