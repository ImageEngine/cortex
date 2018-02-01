//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLCurvesConverter.h"

#include "IECoreGL/CurvesPrimitive.h"

#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

using namespace std;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLCurvesConverter );

ToGLConverter::ConverterDescription<ToGLCurvesConverter> ToGLCurvesConverter::g_description;

ToGLCurvesConverter::ToGLCurvesConverter( IECoreScene::ConstCurvesPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECoreScene::CurvesPrimitive objects to IECoreGL::CurvesPrimitiveObjects.", IECoreScene::CurvesPrimitive::staticTypeId() )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECoreScene::CurvesPrimitive>( toConvert ) );
}

ToGLCurvesConverter::~ToGLCurvesConverter()
{
}

/// \todo Maybe this same functionality should be wrapped up in an Op in IECore?
class ToGLCurvesConverter::ToVertexConverter
{

	public :

		typedef IECore::DataPtr ReturnType;

		ToVertexConverter( const vector<int> &vertsPerCurve, size_t numVertices, size_t step )
			:	m_vertsPerCurve( vertsPerCurve ), m_numVertices( numVertices ), m_step( step )
		{
		}

		template<typename T>
		IECore::DataPtr operator()( const T *inData )
		{
			const typename T::Ptr outData = new T();
			typename T::ValueType &out = outData->writable();
			out.resize( m_numVertices );

			size_t inIndex = 0;
			size_t outIndex = 0;
			const typename T::ValueType &in = inData->readable();
			for( vector<int>::const_iterator it = m_vertsPerCurve.begin(), eIt = m_vertsPerCurve.end(); it != eIt; it++ )
			{
				for( int i=0; i<*it; i++ )
				{
					out[outIndex++] = in[inIndex];
				}
				inIndex += m_step;
			}

			return outData;
		}

	private :

		const vector<int> &m_vertsPerCurve;
		size_t m_numVertices;
		size_t m_step;

};

IECore::RunTimeTypedPtr ToGLCurvesConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECoreScene::CurvesPrimitive::ConstPtr curves = boost::static_pointer_cast<const IECoreScene::CurvesPrimitive>( src ); // safe because the parameter validated it for us

	IECore::V3fVectorData::ConstPtr points = curves->variableData<IECore::V3fVectorData>( "P", IECoreScene::PrimitiveVariable::Vertex );
	if( !points )
	{
		throw IECore::Exception( "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	IECore::FloatData::ConstPtr widthData = curves->variableData<IECore::FloatData>( "width", IECoreScene::PrimitiveVariable::Constant );
	if( !widthData  )
	{
		widthData = curves->variableData<IECore::FloatData>( "constantwidth", IECoreScene::PrimitiveVariable::Constant );
	}

	float width = 1;
	if( widthData )
	{
		width = widthData->readable();
	}

	CurvesPrimitive::Ptr result = new CurvesPrimitive( curves->basis(), curves->periodic(), curves->verticesPerCurve(), width );

	for ( IECoreScene::PrimitiveVariableMap::const_iterator pIt = curves->variables.begin(); pIt != curves->variables.end(); ++pIt )
	{
		if( !pIt->second.data )
		{
			IECore::msg( IECore::Msg::Warning, "ToGLCurvesConverter", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
			continue;
		}

		if( pIt->second.interpolation == IECoreScene::PrimitiveVariable::Vertex || pIt->second.interpolation == IECoreScene::PrimitiveVariable::Constant )
		{
			result->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else if( pIt->second.interpolation == IECoreScene::PrimitiveVariable::Uniform )
		{
			ToVertexConverter converter( curves->verticesPerCurve()->readable(), curves->variableSize( IECoreScene::PrimitiveVariable::Vertex ), 1 );
			IECore::DataPtr newData = IECore::despatchTypedData<ToVertexConverter, IECore::TypeTraits::IsVectorTypedData>( pIt->second.data.get(), converter );
			if( newData )
			{
				result->addPrimitiveVariable( pIt->first, IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, newData ) );
			}
		}
	}

	return result;
}
