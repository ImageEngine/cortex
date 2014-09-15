//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_TOAPPLESEEDMESHCONVERTER_H
#define IECOREAPPLESEED_TOAPPLESEEDMESHCONVERTER_H

#include "IECoreAppleseed/ToAppleseedShapeConverter.h"

namespace IECore
{
IE_CORE_FORWARDDECLARE( MeshPrimitive );
} // namespace IECore

namespace IECoreAppleseed
{

class ToAppleseedMeshConverter : public ToAppleseedShapeConverter
{

	public :

		typedef IECore::MeshPrimitive InputType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToAppleseedMeshConverter, ToAppleseedMeshConverterTypeId, ToAppleseedShapeConverter );

		ToAppleseedMeshConverter( IECore::MeshPrimitivePtr toConvert );
		virtual ~ToAppleseedMeshConverter();

	protected :

		virtual renderer::Entity *doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		static ConverterDescription<ToAppleseedMeshConverter> g_description;

};

IE_CORE_DECLAREPTR( ToAppleseedMeshConverter );

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_TOAPPLESEEDMESHCONVERTER_H
