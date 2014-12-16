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

#ifndef IECOREALEMBIC_FROMALEMBICGEOMBASECONVERTER_H
#define IECOREALEMBIC_FROMALEMBICGEOMBASECONVERTER_H

#include "Alembic/AbcGeom/IPolyMesh.h"

#include "IECore/MeshPrimitive.h"

#include "IECoreAlembic/FromAlembicConverter.h"
#include "IECoreAlembic/Export.h"

namespace IECoreAlembic
{

class IECOREALEMBIC_API FromAlembicGeomBaseConverter : public FromAlembicConverter
{

	public :

		typedef Alembic::AbcGeom::IPolyMesh InputType;
		typedef IECore::MeshPrimitive ResultType;
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromAlembicGeomBaseConverter, FromAlembicGeomBaseConverterTypeId, FromAlembicConverter );

	protected :

		FromAlembicGeomBaseConverter( const std::string &description, Alembic::Abc::IObject iGeom );
		
		/// Should be called by subclasses to convert uvs onto a Primitive.
		void convertUVs( Alembic::AbcGeom::IV2fGeomParam &uvs, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::Primitive *primitive ) const;
		/// Should be called by subclasses to convert Alembic's arbitrary geometry parameter into
		/// IECore::PrimitiveVariables.
		void convertArbGeomParams( Alembic::Abc::ICompoundProperty &params, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::Primitive *primitive ) const;
		/// May be called by subclasses to convert other geometry parameters into IECore::PrimitiveVariables.
		template<typename T>
		void convertGeomParam( T &param, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::Primitive *primitive ) const;
				
	private :
	
		IECore::PrimitiveVariable::Interpolation interpolationFromScope( Alembic::AbcGeom::GeometryScope scope ) const;
		
};

IE_CORE_DECLAREPTR( FromAlembicGeomBaseConverter )

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_FROMALEMBICGEOMBASECONVERTER_H
