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

#ifndef IECORERI_GXEVALUATOR_H
#define IECORERI_GXEVALUATOR_H

#include "tbb/queuing_rw_mutex.h"

#include "boost/noncopyable.hpp"

/// \todo Can this include be relocated so you don't need a direct dependency on 3Delight in the consuming code?
#include "gx.h"

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MeshPrimitiveEvaluator.h"

#include "IECoreRI/Export.h"

namespace IECoreRI
{

/// This class simplifies the use of the Gx API by
/// wrapping it to accept IECore datatypes for input and output.
/// \todo Can we make this refcounted? It will help simplify most threaded implementations.
/// \ingroup geometryProcessingGroup
class IECORERI_API GXEvaluator : public boost::noncopyable
{

	public :
	
		GXEvaluator( const IECore::Primitive *primitive );
		~GXEvaluator();
		
		unsigned numFaces() const;
		
		/// Evaluates points at the specified u,v positions of the specified faces.
		/// \threading It is safe to call this from multiple concurrent threads.
		IECore::CompoundDataPtr evaluate( const IECore::IntVectorData *faceIndices, const IECore::FloatVectorData *u, const IECore::FloatVectorData *v, const std::vector<std::string> &primVarNames ) const;
		/// Evaluates points at the specified s,t positions. As an individual evaluation can fail if no geometry exists
		/// at that location, an additional "gxStatus" BoolVectorData is returned, with elements being true only if the corresponding
		/// results are valid.
		/// \threading It is safe to call this from multiple concurrent threads.
		IECore::CompoundDataPtr evaluate( const IECore::FloatVectorData *s, const IECore::FloatVectorData *t, const std::vector<std::string> &primVarNames ) const;
		
	private :

		void validatePrimVarNames( const std::vector<std::string> &primVarNames ) const;

		RtContextHandle m_context;
		GxGeometryHandle m_geo;
		
		typedef std::map<std::string, IECore::TypeId> PrimitiveVariableTypeMap;
		PrimitiveVariableTypeMap m_primitiveVariableTypes;
		
		void buildSTEvaluator() const;
		typedef tbb::queuing_rw_mutex Mutex;
		mutable Mutex m_stEvaluatorMutex;
		mutable IECore::MeshPrimitiveEvaluatorPtr m_stEvaluator;
		
};

} // namespace IECoreRI

#endif // IECORERI_GXEVALUATOR_H
