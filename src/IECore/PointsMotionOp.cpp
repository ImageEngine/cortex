//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PointsMotionOp.h"
#include "IECore/MotionPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/DespatchTypedData.h"



using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointsMotionOp );

PointsMotionOp::PointsMotionOp()
	:	Op(
		"Creates a MotionPrimitive object from a list of PointsPrimitive objects. If a particle does not exist on any given time then it's primVars are set to 0.",
		new ObjectParameter(
			"result",
			"Resulting motion primitive object.",
			new MotionPrimitive(),
			MotionPrimitiveTypeId
		)
	)
{
	m_snapshotTimesParameter = new FloatVectorParameter(
		"snapshotTimes",
		"Snapshot times for each PointsPrimitive.",
		new FloatVectorData()
	);
	m_pointsPrimitiveVectorParameter = new ObjectVectorParameter(
		"pointsPrimitives",
		"List of PointsPrimitive objects for each motion snapshot.",
		new ObjectVector()
	);
	m_idPrimVarNameParameter = new StringParameter(
		"idPrimVarName",
		"Primvar name used as unique ID for each particle in the PointsPrimitive objects.",
		new StringData( "id" )
	);
	parameters()->addParameter( m_snapshotTimesParameter );
	parameters()->addParameter( m_pointsPrimitiveVectorParameter );
	parameters()->addParameter( m_idPrimVarNameParameter );
}

PointsMotionOp::~PointsMotionOp()
{
}

FloatVectorParameter * PointsMotionOp::snapshotTimesParameter()
{
	return m_snapshotTimesParameter;
}

const FloatVectorParameter * PointsMotionOp::snapshotTimesParameter() const
{
	return m_snapshotTimesParameter;
}

ObjectVectorParameter * PointsMotionOp::pointsPrimitiveVectorParameter()
{
	return m_pointsPrimitiveVectorParameter;
}

const ObjectVectorParameter * PointsMotionOp::pointsPrimitiveVectorParameter() const
{
	return m_pointsPrimitiveVectorParameter;
}

StringParameter * PointsMotionOp::idPrimVarNameParameter()
{
	return m_idPrimVarNameParameter;
}

const StringParameter * PointsMotionOp::idPrimVarNameParameter() const
{
	return m_idPrimVarNameParameter;
}

struct PointsMotionOp::IdInfo
{
	unsigned int finalIndex;
	int firstValidSnapshot;
	unsigned firstSnapshotIndex;
	int lastValidSnapshot;
	unsigned lastSnapshotIndex;
	IdInfo() : finalIndex(0), firstValidSnapshot(-1), firstSnapshotIndex(-1), lastValidSnapshot(-1), lastSnapshotIndex(-1) {}
};

struct PointsMotionOp::PrimVarBuilder
{
	typedef DataPtr ReturnType;

	template< typename T > 
	struct CompatibleTypedData : boost::mpl::or_< IECore::TypeTraits::IsNumericVectorTypedData<T>, IECore::TypeTraits::IsVecVectorTypedData<T>, IECore::TypeTraits::IsColor< typename IECore::TypeTraits::VectorValueType<T>::type > > {};

	ConstIntVectorDataPtr m_ids;
	PointsMotionOp::IdMap &m_map;

	PrimVarBuilder( ConstIntVectorDataPtr ids, PointsMotionOp::IdMap &map ) : m_ids(ids), m_map(map)
	{
	}

	template<typename T>
	ReturnType operator() ( typename T::Ptr data )
	{
		typename T::Ptr newData = new T();
		const typename T::ValueType &dataVec = data->readable();
		typename T::ValueType &newDataVec = newData->writable();
		// Create primVarVector with default value 0. Opacity primVar will get 0 for "dead" particles (if existent).
		newDataVec.resize( m_map.size(), typename T::ValueType::value_type(0) );
		// For each id from the snapshot:
		unsigned index = 0;
		for ( std::vector<int>::const_iterator idIt = m_ids->readable().begin(); idIt != m_ids->readable().end(); idIt++, index++ )
		{
			// Get the value at new data index
			newDataVec[ m_map.find( *idIt )->second.finalIndex ] = dataVec[ index ];
		}
		return newData;
	}
};

ObjectPtr PointsMotionOp::doOperation( const CompoundObject *operands )
{
	const std::string &idPrimVarName = idPrimVarNameParameter()->getTypedValue();
	const std::vector<float> &snapshotTimes = m_snapshotTimesParameter->getTypedValue();
	const std::vector<ObjectPtr> &objectVector = staticPointerCast< ObjectVector, Object >( m_pointsPrimitiveVectorParameter->getValue() )->members();

	// Check if shutter time vector has same length as the points primitive vector...
	if ( snapshotTimes.size() != objectVector.size() )
	{
		throw InvalidArgumentException( "PointsMotionOp : Number of snapshot times does not match number of pointsPrimitives!" );
	}

	IdMap idMap;
	std::map< std::string, PrimitiveVariable::Interpolation > primVars, tmpVars;
	bool firstObject = true;
	int snapshot = 0;
	IntVectorDataPtr newIdsPtr = new IntVectorData();
	std::vector<int> &newIds = newIdsPtr->writable();
	std::vector< const std::vector< Imath::V3f > * > positions;

	for ( std::vector<ObjectPtr>::const_iterator it = objectVector.begin(); it != objectVector.end(); it++, snapshot++ )
	{
		if ( !(*it) )
		{
			throw InvalidArgumentException( "PointsMotionOp : NULL Object pointer in the pointsPrimitive parameter vector!" );
		}
		// Check if all objects in the vector are PointsPrimitive and contain the id prim var...
		if ( (*it)->typeId() != PointsPrimitiveTypeId )
		{
			throw InvalidArgumentException( "PointsMotionOp : Invalid object passed on pointsPrimitives parameter!" );
		}
		ConstPointsPrimitivePtr points = staticPointerCast< const PointsPrimitive >( *it );

		if ( !points->arePrimitiveVariablesValid() )
		{
			throw InvalidArgumentException( "PointsMotionOp : Invalid primvars in given PointsPrimitive object." );
		}
	
		ConstIntVectorDataPtr ids = points->variableData< IntVectorData >( idPrimVarName );
		if ( !ids )
		{
			throw InvalidArgumentException( "PointsMotionOp : Could not find particle ids on the given PointsPrimitive object." );
		}

		ConstV3fVectorDataPtr pos = points->variableData< V3fVectorData >( "P" );
		if ( !pos )
		{
			throw InvalidArgumentException( "PointsMotionOp : Could not find particle Ps on the given PointsPrimitive object." );
		}
		positions.push_back( &pos->readable() );

		// Check if all objects contain the same set of prim vars and with same data type and interpolation.
		tmpVars.clear();
		for ( PrimitiveVariableMap::const_iterator pit = points->variables.begin(); pit != points->variables.end(); pit++ )
		{
			tmpVars[ pit->first ] = pit->second.interpolation;
		}

		if ( firstObject )
		{
			primVars = tmpVars;
			firstObject = false;
			newIds.reserve( ids->readable().size() );
		}
		else
		{
			if ( primVars != tmpVars )
			{
				throw InvalidArgumentException( "PointsMotionOp : The given PointsPrimitive objects don't have same set of primVars!" );
			}
		}

		// Collect ids and snapshot valid ranges from all particles.
		unsigned int index = 0;
		for ( std::vector<int>::const_iterator idIt = ids->readable().begin(); idIt != ids->readable().end(); idIt++, index++ )
		{
			IdInfo &idInfo = idMap[ *idIt ];

			if ( idInfo.firstValidSnapshot == -1 )
			{
				// item was just created and added to the idMap...
				idInfo.finalIndex = newIds.size();
				idInfo.firstValidSnapshot = snapshot;
				idInfo.firstSnapshotIndex = index;
				idInfo.lastValidSnapshot = snapshot;
				idInfo.lastSnapshotIndex = index;

				newIds.push_back( *idIt );
			}
			else
			{
				idInfo.lastValidSnapshot = snapshot;
				idInfo.lastSnapshotIndex = index;
			}
		}
	}
	size_t totalPoints = idMap.size();

	MotionPrimitivePtr result = new MotionPrimitive();
	// For each snapshot:
	snapshot = 0;
	for ( std::vector<ObjectPtr>::const_iterator it = objectVector.begin(); it != objectVector.end(); it++, snapshot++ )
	{
		ConstPointsPrimitivePtr points = staticPointerCast< const PointsPrimitive >( *it );
		PointsPrimitivePtr primitive = new PointsPrimitive( totalPoints );

		// Set the 'id' primVar value with the complete list of ids from the map.
		primitive->variables[ idPrimVarName ] = PrimitiveVariable( PrimitiveVariable::Vertex, newIdsPtr->copy() );

		// For each primVarName, except the id one...
		// \todo paralelize this loop
		for ( PrimitiveVariableMap::const_iterator pIt = points->variables.begin(); pIt != points->variables.end(); pIt++ )
		{
			if ( pIt->second.interpolation == PrimitiveVariable::Uniform || pIt->second.interpolation == PrimitiveVariable::Constant )
			{
				primitive->variables[ pIt->first ] = PrimitiveVariable( pIt->second.interpolation, pIt->second.data->copy() );
			}
			else if ( pIt->first != idPrimVarName )
			{
				ConstIntVectorDataPtr ids = points->variableData< IntVectorData >( idPrimVarName );
				PrimVarBuilder primVarBuilder( ids, idMap );
				// \todo: make sure we support all relevant primvar types....
				primitive->variables[ pIt->first ] = PrimitiveVariable( pIt->second.interpolation, 
						IECore::despatchTypedData< PrimVarBuilder, PrimVarBuilder::CompatibleTypedData >( pIt->second.data, primVarBuilder ) 
				);
			}
		}

		// Now process missing P values by setting them to the "closest" valid P
		std::vector< Imath::V3f > &p = primitive->variableData< V3fVectorData >( "P" )->writable();
		// For each id on the map:
		for ( IdMap::const_iterator mapIt = idMap.begin(); mapIt != idMap.end(); mapIt++ )
		{
			const IdInfo &info = mapIt->second;
			// Set all the snapshots that come prior to the first valid one equal to the first one.
			if ( info.firstValidSnapshot > snapshot )
			{
				p[ info.finalIndex ] = (*positions[ info.firstValidSnapshot ])[ info.firstSnapshotIndex ];
			}
			// Set all the snapshots that come after the last valid one equal to the last one.
			if ( info.lastValidSnapshot < snapshot )
			{
				p[ info.finalIndex ] = (*positions[ info.lastValidSnapshot ])[ info.lastSnapshotIndex ];
			}
		}
		result->addSnapshot( snapshotTimes[ it - objectVector.begin() ], primitive );
	}
	return result;
}
