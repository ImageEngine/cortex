//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_GEOCOBIOTRANSLATOR_H
#define IECOREHOUDINI_GEOCOBIOTRANSLATOR_H

#include "GEO/GEO_IOTranslator.h"
#include "GU/GU_Detail.h"
#include "UT/UT_IStream.h"
#include "ieHoudini.h"

namespace IECoreHoudini
{

/// An IO Translator for saving and loading .cob files using a Houdini File SOP
class CortexHOUAPI GEO_CobIOTranslator : public GEO_IOTranslator
{

	public :

		GEO_CobIOTranslator();
		virtual ~GEO_CobIOTranslator();
		
		virtual GEO_IOTranslator *duplicate() const;
		
		virtual const char *formatName() const;

		virtual int checkExtension( const char *fileName );
		virtual int checkMagicNumber( unsigned magic );

		/// Loads a cob if the content of the cob has a registered ToHoudiniGeometryConverter
		virtual GA_Detail::IOStatus fileLoad( GEO_Detail *geo, UT_IStream &is, int ate_magic );
		
		/// Saves a cob by attempting to find a FromHoudiniGeometryConverter matching the given GEO_Detail
		virtual GA_Detail::IOStatus fileSaveToFile( const GEO_Detail *geo, ostream &os, const char *fileName );
};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GEOCOBIOTRANSLATOR_H
