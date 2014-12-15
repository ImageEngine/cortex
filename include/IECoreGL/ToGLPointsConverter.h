//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_TOGLPOINTSCONVERTER_H
#define IECOREGL_TOGLPOINTSCONVERTER_H

#include "IECoreGL/Export.h"
#include "IECoreGL/ToGLConverter.h"

namespace IECore
{
	IE_CORE_FORWARDDECLARE( PointsPrimitive );
}

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( PointsPrimitive );

/// Converts IECore::PointsPrimitive objects into IECoreGL::PointsPrimitive objects.
/// \ingroup conversionGroup
class IECOREGL_API ToGLPointsConverter : public ToGLConverter
{

	public :

		typedef IECore::PointsPrimitive InputType;
		typedef IECoreGL::PointsPrimitive ResultType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::ToGLPointsConverter, ToGLPointsConverterTypeId, ToGLConverter );

		ToGLPointsConverter( IECore::ConstPointsPrimitivePtr toConvert = 0 );
		virtual ~ToGLPointsConverter();

	protected :

		virtual IECore::RunTimeTypedPtr doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const;

	private :
	
		static ConverterDescription<ToGLPointsConverter> g_description;

};

} // namespace IECoreGL

#endif // IECOREGL_TOGLPOINTSCONVERTER_H
