//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECore/VectorDataFilterOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace boost;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( VectorDataFilterOp );

VectorDataFilterOp::VectorDataFilterOp()
	:	ModifyOp( "VectorDataFilterOp", "Filters VectorData.", new ObjectParameter( "result", "The filtered result", new IntVectorData, DataTypeId ), new ObjectParameter( "input", "The data to filter.", new IntVectorData, DataTypeId ) )
{
	m_filterParameter = new ObjectParameter(
		"filter",
		"A vector of booleans to filter the data with. "
		"A true boolean value keeps the corresponding "
		"data element, and a false one removes it. "
		"Use the invertFilter parameter to reverse this "
		"relationship.",
		new BoolVectorData,
		BoolVectorData::staticTypeId()
	);
	m_invertFilterParameter = new BoolParameter(
		"invert",
		"When this is on, the values of the filter "
		"parameter are flipped, so that false values "
		"keep the corresponding element and true "
		"values remove it.",
		false
	);
	m_clipParameter = new BoolParameter(
		"clip",
		"This parameter only comes into play if the data "
		"to be filtered is longer than the filter. "
		"In this case, the extra data is discarded if clip "
		"is true, and kept if clip is false.",
		true
	);
	parameters()->addParameter( m_filterParameter );
	parameters()->addParameter( m_invertFilterParameter );
	parameters()->addParameter( m_clipParameter );
}

VectorDataFilterOp::~VectorDataFilterOp()
{
}

struct Filter
{
	typedef void ReturnType;
	
	bool invert;
	bool clip;
	const vector<bool> *filter;

	template<typename T>
	void operator() ( typename T::Ptr data )
	{
		assert( data );
		assert( filter );
		
		typedef typename T::ValueType Vector;
		
		const Vector &v = data->readable();
		Vector vf;
		typename Vector::const_iterator vIt = v.begin();		
		typename Vector::const_iterator vItEnd = vIt + min( v.size(), filter->size() );
		vector<bool>::const_iterator fIt = filter->begin();
		for( ; vIt!=vItEnd; vIt++, fIt++ )
		{
			if( *fIt == !invert )
			{
				vf.push_back( *vIt );
			}
		}
		if( !clip )
		{
			std::copy( vItEnd, v.end(), back_insert_iterator<Vector>( vf ) );
		}
		data->writable().swap( vf );
	}
};

void VectorDataFilterOp::modify( ObjectPtr object, ConstCompoundObjectPtr operands )
{
	Filter f;
	f.invert = operands->member<BoolData>( "invert" )->readable();
	f.clip = operands->member<BoolData>( "clip" )->readable();
	f.filter = &( operands->member<BoolVectorData>( "filter" )->readable() );
	despatchTypedData<Filter, TypeTraits::IsVectorTypedData>( static_pointer_cast<Data>( object ), f );
}
