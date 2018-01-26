//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/PointsMotionOp.h"

#include "IECoreScene/MotionPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Object.h"
#include "IECore/ObjectParameter.h"
#include "IECore/ObjectVector.h"

using namespace IECore;
using namespace IECoreScene;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointsMotionOp );

PointsMotionOp::PointsMotionOp()
	:	Op(
		"Creates a MotionPrimitive object from a list of PointsPrimitive objects. If a point does not exist on any given snapshot then it's non-masked primvars are copied from the closest available snapshot. Masked primvars are set to zero.",
		new ObjectParameter(
			"result",
			"Resulting motion primitive object.",
			new MotionPrimitive(),
			MotionPrimitive::staticTypeId()
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

	m_maskedPrimVarsParameter = new StringVectorParameter(
		"maskedPrimVars",
		"List of primitive variables that should be set to zero when the corresponding point does not exist on the snapshot.",
		new StringVectorData()
	);

	parameters()->addParameter( m_snapshotTimesParameter );
	parameters()->addParameter( m_pointsPrimitiveVectorParameter );
	parameters()->addParameter( m_idPrimVarNameParameter );
	parameters()->addParameter( m_maskedPrimVarsParameter );
}

PointsMotionOp::~PointsMotionOp()
{
}

FloatVectorParameter * PointsMotionOp::snapshotTimesParameter()
{
	return m_snapshotTimesParameter.get();
}

const FloatVectorParameter * PointsMotionOp::snapshotTimesParameter() const
{
	return m_snapshotTimesParameter.get();
}

ObjectVectorParameter * PointsMotionOp::pointsPrimitiveVectorParameter()
{
	return m_pointsPrimitiveVectorParameter.get();
}

const ObjectVectorParameter * PointsMotionOp::pointsPrimitiveVectorParameter() const
{
	return m_pointsPrimitiveVectorParameter.get();
}

StringParameter * PointsMotionOp::idPrimVarNameParameter()
{
	return m_idPrimVarNameParameter.get();
}

const StringParameter * PointsMotionOp::idPrimVarNameParameter() const
{
	return m_idPrimVarNameParameter.get();
}

StringVectorParameter * PointsMotionOp::maskedPrimVarsParameter()
{
	return m_maskedPrimVarsParameter.get();
}

const StringVectorParameter * PointsMotionOp::maskedPrimVarsParameter() const
{
	return m_maskedPrimVarsParameter.get();
}

struct PointsMotionOp::IdInfo
{
	unsigned int finalIndex;
	int firstValidSnapshot;
	unsigned firstSnapshotIndex;
	int lastValidSnapshot;
	unsigned lastSnapshotIndex;
	IdInfo() : finalIndex(0), firstValidSnapshot(-1), firstSnapshotIndex(0), lastValidSnapshot(-1), lastSnapshotIndex(0) {}
};

struct PointsMotionOp::PrimVarBuilder
{
	typedef DataPtr ReturnType;

	template< typename T >
	struct CompatibleTypedData : boost::mpl::or_< IECore::TypeTraits::IsNumericVectorTypedData<T>, IECore::TypeTraits::IsVecVectorTypedData<T>, IECore::TypeTraits::IsColor< typename IECore::TypeTraits::VectorValueType<T>::type > > {};

	const std::string &m_primVarName;
	bool m_masked;
	int m_snapshot;
	ConstIntVectorDataPtr m_ids;
	const PointsMotionOp::IdMap &m_map;
	const std::vector<ObjectPtr> &m_objectVector;

	PrimVarBuilder( const std::string &primVarName, bool masked, int snapshot, ConstIntVectorDataPtr ids, PointsMotionOp::IdMap &map, const std::vector<ObjectPtr> &objectVector ) :
					m_primVarName(primVarName), m_masked(masked), m_snapshot(snapshot), m_ids(ids), m_map(map), m_objectVector(objectVector)
	{
	}

	template<typename T>
	ReturnType operator() ( T *data )
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
		if ( !m_masked )
		{
			// Collect snapshot pointers for the given primvar
			std::vector< const typename T::ValueType * > snapshots;
			for ( std::vector<ObjectPtr>::const_iterator it = m_objectVector.begin(); it != m_objectVector.end(); it++ )
			{
				ConstPointsPrimitivePtr points = boost::static_pointer_cast< const PointsPrimitive >( *it );
				typename T::ConstPtr primVarData = points->variableData< T >( m_primVarName );
				assert( primVarData );
				snapshots.push_back( &primVarData->readable() );
			}

			// Now process missing values by setting them to the "closest" known value
			// For each id on the map:
			for ( IdMap::const_iterator mapIt = m_map.begin(); mapIt != m_map.end(); mapIt++ )
			{
				const IdInfo &info = mapIt->second;
				// Set all the snapshots that come prior to the first valid one equal to the first one.
				if ( info.firstValidSnapshot > m_snapshot )
				{
					newDataVec[ info.finalIndex ] = (*snapshots[ info.firstValidSnapshot ])[ info.firstSnapshotIndex ];
				}
				// Set all the snapshots that come after the last valid one equal to the last one.
				if ( info.lastValidSnapshot < m_snapshot )
				{
					newDataVec[ info.finalIndex ] = (*snapshots[ info.lastValidSnapshot ])[ info.lastSnapshotIndex ];
				}
			}
		}

		return newData;
	}
};

ObjectPtr PointsMotionOp::doOperation( const CompoundObject *operands )
{
	const std::string &idPrimVarName = idPrimVarNameParameter()->getTypedValue();
	const std::vector<float> &snapshotTimes = m_snapshotTimesParameter->getTypedValue();
	const std::vector<ObjectPtr> &objectVector = boost::static_pointer_cast< ObjectVector, Object >( m_pointsPrimitiveVectorParameter->getValue() )->members();
	const std::vector<std::string> &maskedPrimvars = m_maskedPrimVarsParameter->getTypedValue();
	std::set< std::string > maskedPrimvarsSet;

	for ( std::vector<std::string>::const_iterator mit = maskedPrimvars.begin(); mit != maskedPrimvars.end(); mit++ )
	{
		maskedPrimvarsSet.insert( *mit );
	}

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

	for ( std::vector<ObjectPtr>::const_iterator it = objectVector.begin(); it != objectVector.end(); it++, snapshot++ )
	{
		if ( !(*it) )
		{
			throw InvalidArgumentException( "PointsMotionOp : NULL Object pointer in the pointsPrimitive parameter vector!" );
		}
		// Check if all objects in the vector are PointsPrimitive and contain the id prim var...
		if ( (*it)->typeId() != PointsPrimitive::staticTypeId() )
		{
			throw InvalidArgumentException( "PointsMotionOp : Invalid object passed on pointsPrimitives parameter!" );
		}
		ConstPointsPrimitivePtr points = boost::static_pointer_cast< const PointsPrimitive >( *it );

		if ( !points->arePrimitiveVariablesValid() )
		{
			throw InvalidArgumentException( "PointsMotionOp : Invalid primvars in given PointsPrimitive object." );
		}

		ConstIntVectorDataPtr ids = points->variableData< IntVectorData >( idPrimVarName );
		if ( !ids )
		{
			throw InvalidArgumentException( "PointsMotionOp : Could not find particle ids on the given PointsPrimitive object." );
		}

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
		ConstPointsPrimitivePtr points = boost::static_pointer_cast< const PointsPrimitive >( *it );
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
				bool masked = ( maskedPrimvarsSet.find( pIt->first ) != maskedPrimvarsSet.end() );
				PrimVarBuilder primVarBuilder( pIt->first, masked, snapshot, ids, idMap, objectVector );
				primitive->variables[ pIt->first ] = PrimitiveVariable( pIt->second.interpolation,
						IECore::despatchTypedData< PrimVarBuilder, PrimVarBuilder::CompatibleTypedData >( pIt->second.data.get(), primVarBuilder )
				);
			}
		}
		result->addSnapshot( snapshotTimes[ it - objectVector.begin() ], primitive );
	}
	return result;
}
