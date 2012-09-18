//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_FROMNUKETILECONVERTER_H
#define IECORENUKE_FROMNUKETILECONVERTER_H

#include "DDImage/Tile.h"

#include "IECoreNuke/FromNukeConverter.h"

namespace IECoreNuke
{

/// The FromNukeTileConverter converts DD::Image::Tiles into IECore::ImagePrimitives.
/// \ingroup conversionGroup.
class FromNukeTileConverter : public FromNukeConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromNukeTileConverter, FromNukeTileConverterTypeId, FromNukeConverter );

		/// The caller is responsible for ensuring that tile is alive
		/// for as long as the converter is.
		FromNukeTileConverter( const DD::Image::Tile *tile );
		virtual ~FromNukeTileConverter();
		
	protected :

		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;

	private :

		const DD::Image::Tile *m_tile;

};

IE_CORE_DECLAREPTR( FromNukeTileConverter );

} // namespace IECoreNuke

#endif // IECORENUKE_FROMNUKETILECONVERTER_H
