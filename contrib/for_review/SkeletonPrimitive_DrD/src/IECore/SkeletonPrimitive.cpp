//////////////////////////////////////////////////////////////////////////
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

#include "IECore/SkeletonPrimitive.h"
#include "IECore/Renderer.h"

#include <algorithm>
#include <numeric>

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

const unsigned int SkeletonPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(SkeletonPrimitive);


SkeletonPrimitive::SkeletonPrimitive() :
	m_jointNames(new StringVectorData), m_parentIds(new IntVectorData),
	m_defaultPose(new M44fVectorData), m_localMatrices(new M44fVectorData), m_globalMatrices(new M44fVectorData)
{
	m_jointsAxis = false;
	m_jointsRadius = 1.0;
}


SkeletonPrimitive::SkeletonPrimitive( ConstSkeletonPrimitivePtr other ) :
	m_jointNames(new StringVectorData), m_parentIds(new IntVectorData),
	m_defaultPose(new M44fVectorData), m_localMatrices(new M44fVectorData), m_globalMatrices(new M44fVectorData)
{
	m_jointNames = other->m_jointNames->copy();
	m_parentIds = other->m_parentIds->copy();

	m_defaultPose = other->m_defaultPose->copy();
	m_localMatrices = other->m_localMatrices->copy();
	m_globalMatrices = other->m_globalMatrices->copy();

	synchVectorIds();
	update();

	m_jointsAxis = false;
	m_jointsRadius = 1.0;
}


SkeletonPrimitive::SkeletonPrimitive( ConstM44fVectorDataPtr poses, ConstIntVectorDataPtr parentIds, Space space) :
	m_jointNames(new StringVectorData), m_parentIds(new IntVectorData),
	m_defaultPose(new M44fVectorData), m_localMatrices(new M44fVectorData), m_globalMatrices(new M44fVectorData)
{
	if ( poses->readable().size() != parentIds->readable().size() )
		throw Exception( "Bad topology - skeleton parents must equal bones!" );

	m_parentIds = parentIds->copy();

	int newSize = poses->readable().size();

	m_jointNames->writable().resize(newSize);

	synchVectorIds();

	std::stringstream ss;
	for (int i=0; i<newSize; i++)
	{
		ss << i;
		m_jointNames->writable()[i] = "joint"+ss.str();
	}

	switch(space)
	{
		case SkeletonPrimitive::Local :
			m_localMatrices = poses->copy();
			m_defaultPose = m_localMatrices->copy();
			break;

		case SkeletonPrimitive::World :
			m_localMatrices->writable().resize(newSize);
			m_globalMatrices->writable().resize(newSize);
			for (int i=0; i<newSize; i++)
			{
				m_localMatrices->writable()[i].makeIdentity();
			}

			int rootId = getRootJointId(0);
			setFromGlobalMatrices(rootId, poses);
			m_defaultPose = m_localMatrices->copy();
			break;
	}
	update();

	m_jointsAxis = false;
	m_jointsRadius = 1.0;
}


void SkeletonPrimitive::setRadius(float radius)
{
	m_jointsRadius = radius;
}

void SkeletonPrimitive::setDebug(bool debug)
{
	m_jointsAxis = debug;
}

float SkeletonPrimitive::getRadius()
{
	return m_jointsRadius;
}

bool SkeletonPrimitive::getDebug()
{
	return m_jointsAxis;
}

/** @note: remember to call update before any call to this one */
Imath::Box3f SkeletonPrimitive::bound() const
{
	Imath::Box3f bbox;
	for (unsigned int i=0; i<numJoints(); i++)
	{
		bbox.extendBy( m_globalMatrices->readable()[i].translation() );
	}

	return bbox;
}

size_t SkeletonPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch(interpolation)
	{
		case PrimitiveVariable::Constant :
			return 1;

		case PrimitiveVariable::Uniform :
			return 1;

		case PrimitiveVariable::Vertex :
			return numJoints();

		case PrimitiveVariable::Varying:
			return numJoints();

		case PrimitiveVariable::FaceVarying:
			return numJoints();

		default :
			return 0;
	}
}



/** @note: remember to call update before any call to this one */
void SkeletonPrimitive::render( Renderer *renderer ) const
{
	renderer->skeleton( m_globalMatrices, m_parentIds, m_jointsAxis, m_jointsRadius, variables );
}


void SkeletonPrimitive::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const SkeletonPrimitive *tOther = static_cast<const SkeletonPrimitive *>( other );

	m_jointNames = tOther->m_jointNames->copy();
	m_parentIds = tOther->m_parentIds->copy();

	m_defaultPose = tOther->m_defaultPose->copy();
	m_localMatrices = tOther->m_localMatrices->copy();
	m_globalMatrices = tOther->m_globalMatrices->copy();

	synchVectorIds();
}


void SkeletonPrimitive::setAsCopyOf( ConstSkeletonPrimitivePtr other )
{
	m_jointNames = other->m_jointNames->copy();
	m_parentIds = other->m_parentIds->copy();

	m_defaultPose = other->m_defaultPose->copy();
	m_localMatrices = other->m_localMatrices->copy();
	m_globalMatrices = other->m_globalMatrices->copy();

	synchVectorIds();
}

void SkeletonPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_jointNames, container, "names" );
	context->save( m_parentIds, container, "parentIds" );
	context->save( m_defaultPose, container, "defaultPose" );
	context->save( m_localMatrices, container, "localMatrices" );
}


void SkeletonPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;

	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	m_jointNames = context->load<StringVectorData>( container, "names" );
	m_parentIds = context->load<IntVectorData>( container, "parentIds" );

	m_localMatrices = context->load<M44fVectorData>( container, "localMatrices" );

	try // try to load new SkeletonPrimitives
	{
		m_defaultPose = context->load<M44fVectorData>( container, "defaultPose" );
	}
	catch(Exception) {
		m_defaultPose = m_localMatrices->copy();
	}


	int newSize = numJoints();
	m_globalMatrices->writable().resize(newSize);

	synchVectorIds();
	update();
}


bool SkeletonPrimitive::isSimilarTo(  ConstObjectPtr other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}

	const SkeletonPrimitive *tOther = static_cast<const SkeletonPrimitive *>( other.get() );

	if (numJoints() != tOther->numJoints())
	{
		return false;
	}

	float eps = 1e-06f;
	for (unsigned int i=0; i<numJoints(); i++)
	{
		if (m_jointNames->readable()[i] != tOther->m_jointNames->readable()[i]) return false;

		if (m_parentIds->readable()[i] != tOther->m_parentIds->readable()[i]) return false;

		for (unsigned int row=0; row<4; row++)
		{
			for (unsigned int col=0; col<4; col++)
			{
				float val0 = Imath::abs<float>(m_defaultPose->readable()[i].x[row][col]) -
							Imath::abs<float>(tOther->m_defaultPose->readable()[i].x[row][col]);

				float val1 = Imath::abs<float>(m_localMatrices->readable()[i].x[row][col]) -
							Imath::abs<float>(tOther->m_localMatrices->readable()[i].x[row][col]);

				if ( val0>eps || val1>eps ) return false;
			}
		}
	}

	return true;
}

bool SkeletonPrimitive::isEqualTo( const Object *other ) const
{

	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}

	const SkeletonPrimitive *tOther = static_cast<const SkeletonPrimitive *>( other );

	if (numJoints() != tOther->numJoints())
	{
		return false;
	}

	for (unsigned int i=0; i<numJoints(); i++)
	{
		if (m_jointNames->readable()[i] != tOther->m_jointNames->readable()[i]) return false;

		if (m_parentIds->readable()[i] != tOther->m_parentIds->readable()[i]) return false;

		for (unsigned int row=0; row<4; row++)
		{
			for (unsigned int col=0; col<4; col++)
			{
				if (m_defaultPose->readable()[i].x[row][col] != tOther->m_defaultPose->readable()[i].x[row][col])
					return false;

				if (m_localMatrices->readable()[i].x[row][col] != tOther->m_localMatrices->readable()[i].x[row][col])
					return false;
			}
		}
	}

	return true;
}


void SkeletonPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( m_jointNames );
	a.accumulate( m_parentIds );
	a.accumulate( m_defaultPose );
	a.accumulate( m_localMatrices );
}

void SkeletonPrimitive::addJoint(int parentId, string name)
{
	m_jointNames->writable().push_back(name);
	m_parentIds->writable().push_back(parentId);

	Imath::M44f mat;
	mat.makeIdentity();

	m_defaultPose->writable().push_back(mat);
	m_localMatrices->writable().push_back(mat);

	synchVectorIds();
}


void SkeletonPrimitive::setJointPoses( ConstM44fVectorDataPtr poses, Space space )
{
	if ( poses->readable().size() != numJoints() )
			throw Exception( "Bad topology - skeleton parents must equal bones!" );


	switch(space)
	{
		case SkeletonPrimitive::Local :
			m_localMatrices = poses->copy();
			break;

		case SkeletonPrimitive::World :
			int rootId = getRootJointId(0);
			setFromGlobalMatrices(rootId, poses);
			break;
	}
}

void SkeletonPrimitive::setJointPose( unsigned int jointId, ConstM44fDataPtr pose, Space space )
{
	switch(space)
	{
		case SkeletonPrimitive::Local :
			m_localMatrices->writable()[jointId] = pose->readable();
			break;

		case SkeletonPrimitive::World :
			int parentId = m_parentIds->readable()[jointId];

			if (parentId == -1)
			{
				m_localMatrices->writable()[jointId] = pose->readable();
				m_globalMatrices->writable()[jointId] = pose->readable();
			}
			else
			{
				M44fDataPtr parentGlobalMatrix = getJointPose(parentId, SkeletonPrimitive::World);
				m_localMatrices->writable()[jointId] = pose->readable() * parentGlobalMatrix->readable().inverse();
			}
			break;
	}
}

M44fVectorDataPtr SkeletonPrimitive::getJointPoses( Space space )
{
	switch(space)
	{
		case SkeletonPrimitive::Local :
			return m_localMatrices;
			break;

		case SkeletonPrimitive::World:
			update();
			return m_globalMatrices;
			break;
	}
}

M44fDataPtr SkeletonPrimitive::getJointPose( unsigned int jointId, Space space )
{
	M44fDataPtr ret = new M44fData();
	switch(space)
	{
		case SkeletonPrimitive::Local :
			ret->writable() = m_localMatrices->readable()[jointId];
			break;

		case SkeletonPrimitive::World:
			pullUpdate(jointId);
			ret->writable() = m_globalMatrices->readable()[jointId];
			break;
	}
	return ret;
}


void SkeletonPrimitive::setDefaultPoses( ConstM44fVectorDataPtr poses )
{
	if ( poses->readable().size() != numJoints() )
		throw Exception( "Bad topology - The number of given matrices is different than the number of joints in this SkeletonPrimitive" );

	m_defaultPose = poses->copy();
}

M44fVectorDataPtr SkeletonPrimitive::getDefaultPoses()
{
	return m_defaultPose;
}

void SkeletonPrimitive::applyDefaultPose()
{
	m_localMatrices = m_defaultPose->copy();
	update();
}

IntVectorDataPtr SkeletonPrimitive::getParentIds() const
{
	return m_parentIds->copy();
}

int SkeletonPrimitive::getParentId( unsigned int jointId ) const
{
	return m_parentIds->readable()[jointId];
}

IntVectorDataPtr SkeletonPrimitive::getChildrenIds( unsigned int jointId ) const
{
	IntVectorDataPtr childrenIds = new IntVectorData(m_childrenIds[jointId]);
	return childrenIds;
}

void SkeletonPrimitive::setJointNames( const StringVectorDataPtr names )
{
	if ( names->readable().size() != numJoints() )
			throw Exception( "Wrong number of names" );
	m_jointNames = names->copy();
}

void SkeletonPrimitive::setJointName( unsigned int jointId, const std::string &name )
{
	m_jointNames->writable()[jointId] = name;
}

StringVectorDataPtr SkeletonPrimitive::getJointNames() const
{
	return m_jointNames->copy();
}

std::string SkeletonPrimitive::getJointName( unsigned int jointId ) const
{
	return m_jointNames->readable()[jointId];
}

void SkeletonPrimitive::shareStaticData( SkeletonPrimitivePtr other )
{
	other->m_jointNames = m_jointNames;
	other->m_defaultPose = m_defaultPose;
	other->m_parentIds = m_parentIds;
	other->synchVectorIds();
}

void SkeletonPrimitive::shareAnimatableData( SkeletonPrimitivePtr other )
{
	other->m_localMatrices = m_localMatrices;
}

void SkeletonPrimitive::pullUpdate(unsigned int jointId)
{
	int parentId = m_parentIds->readable()[jointId];
	if (parentId != -1)
	{
		pullUpdate(parentId);
	}
	computeGlobalTransform(jointId);
}


void SkeletonPrimitive::pushUpdate(unsigned int jointId)
{
	computeGlobalTransform(jointId);
	for (unsigned int i=0; i<m_childrenIds[jointId].size(); i++)
	{
		pushUpdate( m_childrenIds[jointId][i] );
	}
}


int SkeletonPrimitive::getRootJointId(unsigned int fromId) const
{
	unsigned int maximumRecursion = numJoints();
	unsigned int count = 0;
	while (m_parentIds->readable().at(fromId) != -1)
	{
		fromId = m_parentIds->readable().at(fromId);
		count += 1;
		if (count == maximumRecursion)
			throw Exception( "Was impossible to find a root joint. At least one joint as to have parent index equal to -1 (no parent at all)" );
	}

	return fromId;
}


void SkeletonPrimitive::update()
{
	unsigned int size = numJoints();

	m_globalMatrices->writable().resize(size);

	pushUpdate( getRootJointId(0) );
}


void SkeletonPrimitive::computeGlobalTransform( int jointId )
{
	int parentId = m_parentIds->readable()[jointId];

	if (parentId == -1)
	{
		m_globalMatrices->writable()[jointId] = m_localMatrices->readable()[jointId];
	}
	else
	{
		m_globalMatrices->writable()[jointId] = m_localMatrices->readable()[jointId] * m_globalMatrices->readable()[parentId];
	}
}


/*
 * IntermediatePose = GlobalPose * InverseParentGlobalPose
 * LocalPose = IntermediatePose * InverseReferencePose
 */
void SkeletonPrimitive::setFromGlobalMatrices(int jointId, ConstM44fVectorDataPtr matrices)
{
	m_globalMatrices->writable()[jointId] = matrices->readable()[jointId];
	int parentId = m_parentIds->readable()[jointId];

	if (parentId == -1)
	{
		m_localMatrices->writable()[jointId] = m_globalMatrices->readable()[jointId];
	}
	else
	{
		Imath::M44f parentGlobalMatrix = matrices->readable()[parentId];

		m_localMatrices->writable()[jointId] = m_globalMatrices->readable()[jointId] * parentGlobalMatrix.inverse();
	}

	for (unsigned int i=0; i<m_childrenIds[jointId].size(); i++)
	{
		setFromGlobalMatrices(m_childrenIds[jointId][i], matrices);
	}
}

void SkeletonPrimitive::synchVectorIds()
{
	m_childrenIds.resize( m_parentIds->readable().size() );

	for (unsigned int i=0; i<m_parentIds->readable().size(); i++)
	{
		int thisParentId = m_parentIds->readable()[i];
		if ( thisParentId >= 0)
		{
			m_childrenIds[ thisParentId ].push_back( i );
		}
	}
}


SkeletonPrimitivePtr SkeletonPrimitive::createHuman()
{
	M44fVectorDataPtr matrices = new M44fVectorData();
	IntVectorDataPtr parentIds = new IntVectorData();

	parentIds->writable().push_back(-1);
	parentIds->writable().push_back(0);
	parentIds->writable().push_back(1);
	parentIds->writable().push_back(2);
	parentIds->writable().push_back(3);
	parentIds->writable().push_back(4);
	parentIds->writable().push_back(5);
	parentIds->writable().push_back(6);
	parentIds->writable().push_back(7);
	parentIds->writable().push_back(5);
	parentIds->writable().push_back(9);
	parentIds->writable().push_back(10);
	parentIds->writable().push_back(11);
	parentIds->writable().push_back(5);
	parentIds->writable().push_back(13);
	parentIds->writable().push_back(14);
	parentIds->writable().push_back(15);
	parentIds->writable().push_back(0);
	parentIds->writable().push_back(17);
	parentIds->writable().push_back(18);
	parentIds->writable().push_back(0);
	parentIds->writable().push_back(20);
	parentIds->writable().push_back(21);

	matrices->writable().push_back( Imath::M44f(2.2204460492503131e-16, 0.99423389712372556, 0.10723319360239569, 0.0, -0.99999999999999989, 2.2204460492503131e-16, 0.0, 0.0, -2.0816681711721685e-17, -0.10723319360239569, 0.99423389712372567, 0.0, 0.0, 7.4654297985666869, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.98203960772693566, 0.0, -0.18867487605806552, 0.0, 0.0, 1.0, 0.0, 0.0, 0.18867487605806552, 0.0, 0.98203960772693566, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.96824884464617145, 0.0, -0.24998834941123571, 0.0, 0.0, 1.0, 0.0, 0.0, 0.24998834941123571, 0.0, 0.96824884464617145, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.99316879236364553, 0.0, 0.11668654538950909, 0.0, -0.0, 1.0, 0.0, 0.0, -0.11668654538950909, -0.0, 0.99316879236364553, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.97534811557415679, 0.0, 0.22067182295422608, 0.0, -0.0, 1.0, 0.0, 0.0, -0.22067182295422608, -0.0, 0.97534811557415679, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.96023488953606984, 0.0, 0.27919340414424509, 0.0, -0.0, 1.0, 0.0, 0.0, -0.27919340414424509, -0.0, 0.96023488953606984, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(2.2204460492503131e-16, -1.0, -0.0, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 0.0, 0.0, -0.0, 1.0, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.0, 0.9589928569026932, -0.28343023905294706, 0.0, -1.0, 0.0, 0.0, 0.0, 2.7755575615628914e-17, 0.28343023905294712, 0.95899285690269309, 0.0, 1.6260470003868648e-17, 0.96517555562964752, -0.089505324154478316, 1.0) );
	matrices->writable().push_back( Imath::M44f(2.2204460492503131e-16, -1.0, -0.0, 0.0, 1.0, 2.2204460492503131e-16, 0.0, 0.0, 0.0, -0.0, 1.0, 0.0, 2.0, 4.4408920985006262e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.0, 0.99999999999999978, 2.7755575615628907e-17, 0.0, 0.95899285690269298, 2.2204460492503131e-16, -0.2834302390529469, 0.0, -0.2834302390529469, -2.7755575615628907e-17, -0.95899285690269309, 0.0, 0.79140231856104748, 1.0000000000000002, 0.30395991058622451, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.29715689821363533, -0.89147069464090656, 0.34202014332566877, 0.0, 0.94868329805051399, 0.31622776601683777, 2.7755575615628914e-17, 0.0, -0.10815626585663497, 0.32446879756990488, 0.93969262078590843, 0.0, 1.0, 0.0, -1.2246467991473532e-16, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.86602540378443871, 0.0, -0.49999999999999994, 0.0, 0.0, 1.0, 0.0, 0.0, 0.49999999999999994, 0.0, 0.86602540378443871, 0.0, 2.6191653120576799, -8.8817841970012523e-16, -1.224646799147353e-16, 1.0) );
	matrices->writable().push_back( Imath::M44f(-0.316227766016838, -0.94868329805051366, -1.224646799147353e-16, 0.0, -0.94868329805051366, 0.31622776601683811, 0.0, 0.0, 3.8726732145403892e-17, 1.1618019643621161e-16, -1.0, 0.0, 2.6854914710480733, 4.4408920985006262e-15, -1.2246467991473495e-16, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.0, -1.0, 0.0, 0.0, 0.9589928569026932, 0.0, -0.28343023905294706, 0.0, 0.28343023905294706, 5.5511151231257827e-17, 0.95899285690269309, 0.0, 0.79140231856104748, -0.99999999999999989, 0.30395991058622451, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.29715689821363533, -0.89147069464090656, -0.34202014332566877, 0.0, 0.94868329805051399, 0.31622776601683777, -2.7755575615628914e-17, 0.0, 0.10815626585663497, -0.32446879756990488, 0.93969262078590843, 0.0, 1.0, 0.0, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.86602540378443871, 0.0, 0.49999999999999994, 0.0, -0.0, 1.0, 0.0, 0.0, -0.49999999999999994, -0.0, 0.86602540378443871, 0.0, 2.6191653120576799, -8.8817841970012523e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.31622776601683789, 0.94868329805051388, 0.0, 0.0, -0.94868329805051388, 0.31622776601683789, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.6854914710480742, 1.7763568394002505e-15, 9.8607613152626476e-32, 1.0) );
	matrices->writable().push_back( Imath::M44f(-0.94511204731701848, -0.16881626495434912, 0.27975754985897555, 0.0, 0.1705291427993407, -0.98518096292376045, -0.018392437270097019, 0.0, 0.27871675491860232, 0.030323901125603691, 0.95989448979987813, 0.0, -0.33944620949534787, -1.0000000000000002, 0.036611003915393847, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.91772761356451982, 0.0, -0.39721030613665032, 0.0, 0.0, 1.0, 0.0, 0.0, 0.39721030613665032, 0.0, 0.91772761356451982, 0.0, 3.6933553329544622, -8.8817841970012523e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(2.2204460492503131e-16, 1.0, 0.0, 0.0, -1.0, 2.2204460492503131e-16, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 3.6933553329544622, -8.8817841970012523e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(-0.94353025296907878, 0.17771269443522297, 0.27958694527584749, 0.0, -0.17951583903834623, -0.98356453177948699, 0.019361698266358748, 0.0, 0.27843262248965228, -0.031921937002193507, 0.95992513493061671, 0.0, -0.33944620949534787, 1.0, 0.036611003915393847, 1.0) );
	matrices->writable().push_back( Imath::M44f(0.91772761356451982, 0.0, -0.39721030613665032, 0.0, 0.0, 1.0, 0.0, 0.0, 0.39721030613665032, 0.0, 0.91772761356451982, 0.0, 3.6933553329544622, -8.8817841970012523e-16, 0.0, 1.0) );
	matrices->writable().push_back( Imath::M44f(2.2204460492503131e-16, 1.0, 0.0, 0.0, -1.0, 2.2204460492503131e-16, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 3.6933553329544622, -8.8817841970012523e-16, 0.0, 1.0) );

	return new SkeletonPrimitive(matrices, parentIds);
}
