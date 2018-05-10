//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_PRIMITIVEVARIABLEALGOS_H
#define IECORESCENE_PRIMITIVEVARIABLEALGOS_H

#include "IECoreScene/PrimitiveVariable.h"

#include "IECore/VectorTypedData.h"

#include <unordered_map>

namespace IECoreScene
{

namespace PrimitiveVariableAlgos
{

struct IndexedData
{
	IndexedData()
	{
	}

	IndexedData( IECore::DataPtr data, IECore::IntVectorDataPtr indices ) : data( data ), indices( indices )
	{
	}

	IECore::DataPtr data;
	IECore::IntVectorDataPtr indices;
};

/// For building a filtered indexed data array from an existing data array
/// ensuring the resulting data array is compact.
template<typename T, template<typename> class V>
class IndexedPrimitiveVariableBuilder
{
	public:
		IndexedPrimitiveVariableBuilder( int dataReserveSize, int indexReserveSize )
			: m_data( new V<std::vector<T> >() ),
			m_writable( m_data->writable() ),
			m_indices( new IECore::IntVectorData() ),
			m_writableIndices( m_indices->writable() )
		{
			m_writable.reserve( dataReserveSize );

			if( indexReserveSize > 0 )
			{
				m_writableIndices.reserve( indexReserveSize );
			}
		}

		void addIndexedValue( const PrimitiveVariable::IndexedView<T> &indexedData, int i )
		{
			if( !indexedData.indices() )
			{
				m_writable.push_back( indexedData[i] );
				return;
			}

			int oldIndex = indexedData.index( i );

			auto it = m_indexMapping.find( oldIndex );

			if( it != m_indexMapping.end() )
			{
				m_writableIndices.push_back( it->second );
			}
			else
			{
				int newIndex = static_cast<int> ( m_writable.size() );
				m_writableIndices.push_back( newIndex );
				m_indexMapping[oldIndex] = newIndex;
				m_writable.push_back( indexedData[i] );
			}
		}

		IndexedData indexedData()
		{
			if( m_writableIndices.size() )
			{
				return IndexedData( m_data, m_indices );
			}

			return IndexedData( m_data, nullptr );

		}

	private:

		typename V<std::vector<T>>::Ptr m_data;
		std::vector<T> &m_writable;

		IECore::IntVectorDataPtr m_indices;
		std::vector<int> &m_writableIndices;

		std::unordered_map<int, int> m_indexMapping;
};

// Base type for all Functors which delete primivars
template<typename U>
class DeleteFlagged
{
	public:
		DeleteFlagged( const PrimitiveVariable::IndexedView<U> &deleteFlagView, bool invert )
			: m_deleteFlagView ( deleteFlagView ), m_dataIndices( nullptr ), m_invert( invert )
		{
		}

		bool shouldKeepPrimitive( size_t i ) const
		{
			return ( m_invert && m_deleteFlagView[i] ) || ( !m_invert && !m_deleteFlagView[i] );
		}

		void setIndices( const IECore::TypedData<std::vector<int> > *dataIndices )
		{
			m_dataIndices = dataIndices ? &dataIndices->readable() : nullptr;
		}

	private:
		const PrimitiveVariable::IndexedView<U> &m_deleteFlagView;

	protected:
		const std::vector<int> *m_dataIndices;
		bool m_invert;
};

/// Filter a primitive variable based on optionally indexed flag array
/// Lifetime of flags, flagIndices & dataIndices should be longer than this functor.
template<typename U>
class DeleteFlaggedUniformFunctor : public DeleteFlagged<U>
{
	public:

		DeleteFlaggedUniformFunctor( const PrimitiveVariable::IndexedView<U> &deleteFlagView, bool invert ) : DeleteFlagged<U>( deleteFlagView, invert )
		{
		}

		template<typename T, template<typename> class V>
		IndexedData operator()( const V<std::vector<T> > *data )
		{

			const std::vector<T> &inputs = data->readable();
			IECoreScene::PrimitiveVariable::IndexedView<T> dataView( inputs, this->m_dataIndices );

			IndexedPrimitiveVariableBuilder<T, V> builder( inputs.size(), this->m_dataIndices ? this->m_dataIndices->size() : 0 );

			for( size_t i = 0; i < dataView.size(); ++i )
			{
				if( this->shouldKeepPrimitive( i ) )
				{
					builder.addIndexedValue( dataView, i );
				}
			}

			return builder.indexedData();
		}

		IndexedData operator()( const IECore::Data *data )
		{
			throw IECore::Exception(
				boost::str( boost::format( "Unexpected Data: %1%" ) % ( data ? data->typeName() : std::string( "nullptr" ) ) )
			);
		}

};

template<typename U>
class DeleteFlaggedVertexFunctor : public DeleteFlagged<U>
{
	public:

		DeleteFlaggedVertexFunctor( const PrimitiveVariable::IndexedView<U> &deleteFlagView, IECore::ConstIntVectorDataPtr verticesPerCurve, bool invert )
			: DeleteFlagged<U>( deleteFlagView, invert ), m_verticesPerCurve( verticesPerCurve )
		{
		}

		template<typename T, template<typename> class V>
		IndexedData operator()( const V<std::vector<T> > *data )
		{
			const std::vector<T> &inputs = data->readable();
			const std::vector<int> &verticesPerCurve = m_verticesPerCurve->readable();

			IECoreScene::PrimitiveVariable::IndexedView<T> dataView( inputs, this->m_dataIndices );
			IndexedPrimitiveVariableBuilder<T, V> builder( inputs.size(), this->m_dataIndices ? this->m_dataIndices->size() : 0 );

			size_t offset = 0;
			for( size_t c = 0; c < verticesPerCurve.size(); ++c )
			{
				int numVerts = verticesPerCurve[c];
				if( this->shouldKeepPrimitive( c ) )
				{
					for( int v = 0; v < numVerts; ++v )
					{
						builder.addIndexedValue( dataView, offset + v );
					}
				}
				offset += numVerts;
			}

			return builder.indexedData();
		}

		IndexedData operator()( const IECore::Data *data )
		{
			throw IECore::Exception(
				boost::str( boost::format( "Unexpected Data: %1%" ) % ( data ? data->typeName() : std::string( "nullptr" ) ) )
			);
		}

	private:

		IECore::ConstIntVectorDataPtr m_verticesPerCurve;

};

template<typename U>
class DeleteFlaggedVaryingFunctor : public DeleteFlagged<U>
{
	public:

		DeleteFlaggedVaryingFunctor( const PrimitiveVariable::IndexedView<U> &deleteFlagView, const CurvesPrimitive *curvesPrimitive, bool invert )
			: DeleteFlagged<U>( deleteFlagView, invert ), m_curvesPrimitive( curvesPrimitive )
		{
		}

		template<typename T, template<typename> class V>
		IndexedData operator()( const V<std::vector<T> > *data )
		{
			const std::vector<T> &inputs = data->readable();
			IECoreScene::PrimitiveVariable::IndexedView<T> dataView( inputs, this->m_dataIndices );
			IndexedPrimitiveVariableBuilder<T, V> builder( inputs.size(), this->m_dataIndices ? this->m_dataIndices->size() : 0 );

			size_t offset = 0;
			for( size_t c = 0; c < m_curvesPrimitive->numCurves(); ++c )
			{
				int numVarying = m_curvesPrimitive->numSegments( c ) + 1;

				if( this->shouldKeepPrimitive( c ) )
				{
					for( int v = 0; v < numVarying; ++v )
					{
						builder.addIndexedValue( dataView, offset + v );
					}
				}
				offset += numVarying;
			}

			return builder.indexedData();
		}

		IndexedData operator()( const IECore::Data *data )
		{
			throw IECore::Exception(
				boost::str( boost::format( "Unexpected Data: %1%" ) % ( data ? data->typeName() : std::string( "nullptr" ) ) )
			);
		}

	private:

		const CurvesPrimitive *m_curvesPrimitive;
};

} // PrimitiveVariableAlgos

} // IECoreScene


#endif  // IECORESCENE_PRIMITIVEVARIABLEALGOS_H