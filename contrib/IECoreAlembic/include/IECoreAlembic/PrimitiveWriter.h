//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREALEMBIC_PRIMITIVEWRITER_H
#define IECOREALEMBIC_PRIMITIVEWRITER_H

#include "IECoreAlembic/ObjectWriter.h"

#include "IECoreScene/Primitive.h"

#include "Alembic/AbcGeom/GeometryScope.h"
#include "Alembic/Abc/OArrayProperty.h"

#include "boost/container/flat_map.hpp"

namespace IECoreAlembic
{

class PrimitiveWriter : public ObjectWriter
{

	protected :

		void writeArbGeomParams( const IECoreScene::Primitive *primitive, Alembic::Abc::OCompoundProperty &params, const char **namesToIgnore = nullptr );

		static Alembic::AbcGeom::GeometryScope geometryScope( IECoreScene::PrimitiveVariable::Interpolation interpolation );

	private :

		template<typename DataType, typename GeomParamType>
		void writeArbGeomParam( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, Alembic::Abc::OCompoundProperty &arbGeomParams );

		typedef boost::container::flat_map<std::string, Alembic::Abc::OArrayProperty> GeomParamMap;
		GeomParamMap m_geomParams;

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_PRIMITIVEWRITER_H
