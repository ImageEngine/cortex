//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "IECore/SmoothSkinningData.h"
#include "IECore/VectorTypedData.h"
#include "boost/format.hpp"

using namespace IECore;
using namespace std;
using namespace boost;

static IndexedIO::EntryID g_influenceNamesEntry("influenceNames");
static IndexedIO::EntryID g_influencePoseEntry("influencePose");
static IndexedIO::EntryID g_pointIndexOffsetsEntry("pointIndexOffsets");
static IndexedIO::EntryID g_pointInfluenceCountsEntry("pointInfluenceCounts");
static IndexedIO::EntryID g_pointInfluenceIndicesEntry("pointInfluenceIndices");
static IndexedIO::EntryID g_pointInfluenceWeightsEntry("pointInfluenceWeights");

const unsigned int SmoothSkinningData::m_ioVersion = 1;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(SmoothSkinningData);

SmoothSkinningData::SmoothSkinningData() :
	m_influenceNames( new StringVectorData ),
	m_influencePose( new M44fVectorData ),
	m_pointIndexOffsets( new IntVectorData ),
	m_pointInfluenceCounts( new IntVectorData ),
	m_pointInfluenceIndices( new IntVectorData ),
	m_pointInfluenceWeights( new FloatVectorData )
{
}

SmoothSkinningData::SmoothSkinningData( ConstStringVectorDataPtr influenceNames,
										ConstM44fVectorDataPtr influencePose,
										ConstIntVectorDataPtr pointIndexOffsets,
										ConstIntVectorDataPtr pointInfluenceCounts,
										ConstIntVectorDataPtr pointInfluenceIndices,
										ConstFloatVectorDataPtr pointInfluenceWeights)
{
	assert( influenceNames );
	assert( influencePose );
	assert( pointIndexOffsets );
	assert( pointInfluenceCounts );
	assert( pointInfluenceIndices );
	assert( pointInfluenceWeights );

	m_influenceNames = influenceNames->copy();
	m_influencePose = influencePose->copy();
	m_pointIndexOffsets = pointIndexOffsets->copy();
	m_pointInfluenceCounts = pointInfluenceCounts->copy();
	m_pointInfluenceIndices = pointInfluenceIndices->copy();
	m_pointInfluenceWeights = pointInfluenceWeights->copy();
}

SmoothSkinningData::~SmoothSkinningData()
{
}

const StringVectorData  *SmoothSkinningData::influenceNames() const
{
	return m_influenceNames;
}

StringVectorData *SmoothSkinningData::influenceNames()
{
	return m_influenceNames;
}

const M44fVectorData *SmoothSkinningData::influencePose() const
{
	return m_influencePose;
}

M44fVectorData *SmoothSkinningData::influencePose()
{
	return m_influencePose;
}

const IntVectorData *SmoothSkinningData::pointIndexOffsets() const
{
	return m_pointIndexOffsets;
}

IntVectorData *SmoothSkinningData::pointIndexOffsets()
{
	return m_pointIndexOffsets;
}

const IntVectorData *SmoothSkinningData::pointInfluenceCounts() const
{
	return m_pointInfluenceCounts;
}

IntVectorData *SmoothSkinningData::pointInfluenceCounts()
{
	return m_pointInfluenceCounts;
}

const IntVectorData *SmoothSkinningData::pointInfluenceIndices() const
{
	return m_pointInfluenceIndices;
}

IntVectorData *SmoothSkinningData::pointInfluenceIndices()
{
	return m_pointInfluenceIndices;
}

const FloatVectorData *SmoothSkinningData::pointInfluenceWeights() const
{
	return m_pointInfluenceWeights;
}

FloatVectorData *SmoothSkinningData::pointInfluenceWeights()
{
	return m_pointInfluenceWeights;
}

void SmoothSkinningData::validateSizes() const
{
	int cin = m_influenceNames->readable().size();
	int cip = m_influencePose->readable().size();

	// check vector sizes
	if (cin != cip)
	{
		string error = str( format( "SmoothSkinningData: Number of influenceNames '%e' does not match number of influencePose '%e'!" ) % cin % cip);
		throw Exception( error );
	}

	int cpio = m_pointIndexOffsets->readable().size();
	int cpic =  m_pointInfluenceCounts->readable().size();

	if (cpio != cpic)
	{
		string error = str( format( "SmoothSkinningData: Number of pointIndexOffsets '%e' does not match number of pointInfluenceCounts '%e'!" ) % cpio % cpic);
		throw Exception( error );
	}

	int cpii = m_pointInfluenceIndices->readable().size();
	int cpiw =  m_pointInfluenceWeights->readable().size();

	if (cpii != cpiw)
	{
		string error = str( format( "SmoothSkinningData: Number of pointInfluenceIndices '%e' does not match number of pointInfluenceWeights '%e'!" ) % cpii % cpiw);
		throw Exception( error );
	}
}

void SmoothSkinningData::validateCounts() const
{
	// check for wrong counts in m_pointInfluenceCounts
	int sum =0;
	for( std::vector<int>::const_iterator it = m_pointInfluenceCounts->readable().begin();
		 it!=m_pointInfluenceCounts->readable().end(); ++it )
	{
		sum += (*it);
	}

	int cpii = m_pointInfluenceIndices->readable().size();
	if ( sum != cpii )
	{
		string error = str( format( "SmoothSkinningData: Sum of all pointInfluenceCounts '%e' does not match size of pointInfluenceIndices and pointInfluenceWeightsmatch number of pointInfluenceWeights '%e'!" ) % sum % cpii);
		throw Exception( error );
	}
}

void SmoothSkinningData::validateIds() const
{
	int cpii = m_pointInfluenceIndices->readable().size();
	int cin = m_influenceNames->readable().size();
	// check for invalid ids in m_pointIndexOffset
	for( std::vector<int>::const_iterator it = m_pointIndexOffsets->readable().begin();
		 it!=m_pointIndexOffsets->readable().end(); ++it )
	{
		int o = (*it);
		if ( ( o < 0 ) or ( o > (cpii-1) ) )
		{
			int id = it - m_pointIndexOffsets->readable().begin();
			string error = str( format( "SmoothSkinningData: pointIndexOffset[%e] with value '%e' is not pointing to valid index in pointInfluenceWeights vector range [ 0, %e ]!" ) % id % o % (cpii-1) );
			throw Exception( error );
		}
	}

	// check for invalid ids in_pointInfluenceIndices
	for( std::vector<int>::const_iterator it = m_pointInfluenceIndices->readable().begin();
		 it!=m_pointInfluenceIndices->readable().end(); ++it )
	{
		int o = (*it);
		if ( ( o < 0 ) or ( o > (cin-1) ) )
		{
			int id = it - m_pointInfluenceIndices->readable().begin();
			string error = str( format( "SmoothSkinningData: pointInfluenceIndices[%e] with value '%e' is not pointing to valid index in influenceNames vector range [ 0, %e ]!" ) % id % o % (cin-1) );
			throw Exception( error );
		}
	}
}

void SmoothSkinningData::validateOffsets() const
{
	// check for mismatches of m_pointInfluenceCounts and m_pointIndexOffsets
	std::vector<int>::const_iterator pio_it = m_pointIndexOffsets->readable().begin();
	int sum = 0;
	for( std::vector<int>::const_iterator pic_it = m_pointInfluenceCounts->readable().begin();
		 pic_it!=m_pointInfluenceCounts->readable().end(); ++pic_it )
	{
		int pic = (*pic_it);
		int pio = (*pio_it);


		if (sum != pio)
		{
			int id = pio_it - m_pointIndexOffsets->readable().begin();
			string error = str( format( "SmoothSkinningData: pointInfluenceOffsets[%e] is pointing to index '%e', but sum of all pointInfluenceCounts up to this id is '%e'!" ) % id % pio % sum );
			throw Exception( error );
		}
		sum += pic;
		++pio_it;
	}
}

void SmoothSkinningData::validate() const
{
	validateSizes();
	validateIds();
	validateCounts();
	validateOffsets();
}

void SmoothSkinningData::copyFrom( const IECore::Object *other, IECore::Object::CopyContext *context )
{
	Data::copyFrom( other, context );
	const SmoothSkinningData *tOther = static_cast<const SmoothSkinningData *>( other );
	m_influenceNames = tOther->m_influenceNames->copy();
	m_influencePose = tOther->m_influencePose->copy();
	m_pointIndexOffsets = tOther->m_pointIndexOffsets->copy();
	m_pointInfluenceCounts = tOther->m_pointInfluenceCounts->copy();
	m_pointInfluenceIndices = tOther->m_pointInfluenceIndices->copy();
	m_pointInfluenceWeights = tOther->m_pointInfluenceWeights->copy();
}

void SmoothSkinningData::save( IECore::Object::SaveContext *context ) const
{
	Data::save(context);
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_influenceNames, container, g_influenceNamesEntry );
	context->save( m_influencePose, container, g_influencePoseEntry );
	context->save( m_pointIndexOffsets, container, g_pointIndexOffsetsEntry );
	context->save( m_pointInfluenceCounts, container, g_pointInfluenceCountsEntry );
	context->save( m_pointInfluenceIndices, container, g_pointInfluenceIndicesEntry );
	context->save( m_pointInfluenceWeights, container, g_pointInfluenceWeightsEntry );
}

void SmoothSkinningData::load( IECore::Object::LoadContextPtr context )
{
	Data::load(context);
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	m_influenceNames = context->load<StringVectorData>( container, g_influenceNamesEntry );
	m_influencePose = context->load<M44fVectorData>( container, g_influencePoseEntry );
	m_pointIndexOffsets = context->load<IntVectorData>( container, g_pointIndexOffsetsEntry );
	m_pointInfluenceCounts = context->load<IntVectorData>( container, g_pointInfluenceCountsEntry );
	m_pointInfluenceIndices = context->load<IntVectorData>( container, g_pointInfluenceIndicesEntry );
	m_pointInfluenceWeights = context->load<FloatVectorData>( container, g_pointInfluenceWeightsEntry );

}

bool SmoothSkinningData::isEqualTo(  const IECore::Object *other ) const
{

	if( !Data::isEqualTo( other ) )
	{
		return false;
	}

	const SmoothSkinningData *tOther = static_cast<const SmoothSkinningData *>( other );

	if(	!m_influenceNames->isEqualTo( tOther->m_influenceNames ) )
	{
		return false;
	}

	if(	!m_influencePose->isEqualTo( tOther->m_influencePose ) )
	{
		return false;
	}

	if(	!m_influencePose->isEqualTo( tOther->m_influencePose ) )
	{
		return false;
	}

	if(	!m_pointInfluenceCounts->isEqualTo( tOther->m_pointInfluenceCounts ) )
	{
		return false;
	}

	if(	!m_pointInfluenceIndices->isEqualTo( tOther->m_pointInfluenceIndices ) )
	{
		return false;
	}

	if(	!m_pointInfluenceWeights->isEqualTo( tOther->m_pointInfluenceWeights ) )
	{
		return false;
	}

	return true;
}

void SmoothSkinningData::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Data::memoryUsage( a );
	a.accumulate( m_influenceNames );
	a.accumulate( m_influencePose );
	a.accumulate( m_pointIndexOffsets );
	a.accumulate( m_pointInfluenceCounts );
	a.accumulate( m_pointInfluenceIndices );
	a.accumulate( m_pointInfluenceWeights );
}

void SmoothSkinningData::hash( MurmurHash &h ) const
{
	Data::hash( h );
	m_influenceNames->hash( h );
	m_influencePose->hash( h );
	m_pointIndexOffsets->hash( h );
	m_pointInfluenceCounts->hash( h );
	m_pointInfluenceIndices->hash( h );
	m_pointInfluenceWeights->hash( h );
}
