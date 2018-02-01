//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011 Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_TOMAYAPARTICLECONVERTER_H
#define IECOREMAYA_TOMAYAPARTICLECONVERTER_H

#include "maya/MFnParticleSystem.h"

#include "IECore/Data.h"

#include "IECoreMaya/ToMayaObjectConverter.h"

namespace IECoreMaya
{

/// \todo Could we make a ToMayaShapeConverter base class to provide utilities for this and the ToMayaMeshConverter etc?
/// \ingroup conversionGroup
class IECOREMAYA_API ToMayaParticleConverter : public ToMayaObjectConverter
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaParticleConverter, ToMayaParticleConverterTypeId, ToMayaObjectConverter );

		ToMayaParticleConverter( IECore::ConstObjectPtr object );

	protected:

		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

	private:

		void addAttribute( const IECore::Data *data, MFnParticleSystem &fnPS, const MString &attrName ) const;

		typedef ToMayaObjectConverterDescription<ToMayaParticleConverter> Description;
		static Description g_description;
};

} // namespace IECoreMaya

#endif // IECOREMAYA_TOMAYAPARTICLECONVERTER_H
