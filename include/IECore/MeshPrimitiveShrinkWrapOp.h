//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MESHPRIMITIVESHRINKWRAPOP_H
#define IE_CORE_MESHPRIMITIVESHRINKWRAPOP_H

#include <vector>

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/TypedObjectParameter.h"


namespace IECore
{

/// An Op to project the vertices of a mesh onto a second mesh, the target mesh, 
/// by means of raycasting. The direction of the ray for each vertex is
/// defined by either an (X/Y/Z) axis, the surface normal, or the vector to the
/// corresponding vertex on a third mesh, the direction mesh. In addition rays can be cast in
/// both directions ("inside" and "outside") with the closest intersection point being used as
/// the projected point, or in either one of these directions only.
class MeshPrimitiveShrinkWrapOp : public TypedPrimitiveOp<MeshPrimitive>
{
	public:

		typedef enum
		{
			Inside = 0,
			Outside = 1,
			Both = 2,
		} Direction;
		
		typedef enum
		{
			Normal = 0,
			XAxis = 1,
			YAxis = 2,
			ZAxis = 3,
			DirectionMesh = 4
		} Method;

		MeshPrimitiveShrinkWrapOp();
		virtual ~MeshPrimitiveShrinkWrapOp();

		IE_CORE_DECLARERUNTIMETYPED( MeshPrimitiveShrinkWrapOp, MeshPrimitiveOp );

		MeshPrimitiveParameterPtr targetMeshParameter();
		ConstMeshPrimitiveParameterPtr targetMeshParameter() const;

		IntParameterPtr directionParameter();
		ConstIntParameterPtr directionParameter() const;
		
		IntParameterPtr methodParameter();
		ConstIntParameterPtr methodParameter() const;
		
		MeshPrimitiveParameterPtr directionMeshParameter();
		ConstMeshPrimitiveParameterPtr directionMeshParameter() const;
		
		FloatParameterPtr triangulationToleranceParameter();
		ConstFloatParameterPtr triangulationToleranceParameter() const;		

	protected:

		/// \todo The meaning of this template parameter needs changing if we're gong to use depatchTypedData
		template<typename T>
		void doShrinkWrap( std::vector<T> &vertices, PrimitivePtr sourcePrimitive, ConstPrimitivePtr targetPrimitive, typename TypedData< std::vector<T> >::ConstPtr directionVerticesData, Direction direction, Method method );

		virtual void modifyTypedPrimitive( MeshPrimitivePtr typedPrimitive, ConstCompoundObjectPtr operands );

	private:

		MeshPrimitiveParameterPtr m_targetMeshParameter;
		IntParameterPtr m_directionParameter;
		IntParameterPtr m_methodParameter;
		MeshPrimitiveParameterPtr m_directionMeshParameter;
		FloatParameterPtr m_triangulationToleranceParameter;

};

IE_CORE_DECLAREPTR( MeshPrimitiveShrinkWrapOp );


} // namespace IECore

#endif // IE_CORE_MESHPRIMITIVESHRINKWRAPOP_H
