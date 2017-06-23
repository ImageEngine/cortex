//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/MeshPrimitive.h"

#include "IECoreAlembic/FromAlembicXFormConverter.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

FromAlembicXFormConverter::ConverterDescription<FromAlembicXFormConverter> FromAlembicXFormConverter::g_description;

IE_CORE_DEFINERUNTIMETYPED( FromAlembicXFormConverter );

FromAlembicXFormConverter::FromAlembicXFormConverter( Alembic::Abc::IObject iXForm )
	:	FromAlembicConverter( "Converts AbcGeom::IXForm objects to IECore::M44f objects", iXForm )
{
}

IECore::ObjectPtr FromAlembicXFormConverter::doAlembicConversion( const Alembic::Abc::IObject &iObject, const Alembic::Abc::ISampleSelector &sampleSelector, const IECore::CompoundObject *operands ) const
{
	IXform iXForm( iObject, kWrapExisting );
	IXformSchema &iXFormSchema = iXForm.getSchema();
	XformSample sample;
	iXFormSchema.get( sample, sampleSelector );
	M44d m = sample.getMatrix();
	return new M44fData(
		M44f(
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]
		)
	);
}
