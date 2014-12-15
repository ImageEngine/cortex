//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef IE_CORE_FILESEQUENCEFUNCTIONS_H
#define IE_CORE_FILESEQUENCEFUNCTIONS_H

#include "IECore/Export.h"
#include "IECore/FileSequence.h"
#include "IECore/FrameList.h"

namespace IECore
{

/// Returns a list of FileSequence objects representing all the sequences in names.
/// names is just a list of arbitrary strings, which may or may not represent files
/// on disk. The returned sequences have at least 2 elements.
IECORE_API void findSequences( const std::vector< std::string > &names, std::vector< FileSequencePtr > &sequences );

/// Returns a list of FileSequence objects representing all the sequences in names.
/// names is just a list of arbitrary strings, which may or may not represent files
/// on disk. The returned sequences have at least the minSequenceSize elements.
IECORE_API void findSequences( const std::vector< std::string > &names, std::vector< FileSequencePtr > &sequences, size_t minSequenceSize );

/// Generates all sequences with at least minSequenceSize elements residing in given directory in the form of a list of FileSequences.
IECORE_API void ls( const std::string &path, std::vector< FileSequencePtr > &sequences, size_t minSequenceSize = 2 );

/// Attempts to find a sequence matching the given sequence template (e.g. with at least one '#' character).
IECORE_API void ls( const std::string &sequencePath, FileSequencePtr &sequence, size_t minSequenceSize = 2 );

/// Returns a FrameList instance that "best" represents the specified list of integer
/// frame numbers. This function attempts to be intelligent and uses a CompoundFrameList
/// of FrameRange objects to represent the specified frames compactly.
IECORE_API FrameListPtr frameListFromList( const std::vector< FrameList::Frame > &frames );

} // namespace IECore

#endif // IE_CORE_FILESEQUENCEFUNCTIONS_H
