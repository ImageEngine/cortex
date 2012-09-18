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

#ifndef IECORE_SKELETONPRIMITIVE_H
#define IECORE_SKELETONPRIMITIVE_H

#include <sstream>
#include <string>

#include "IECore/Primitive.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"

#include "OpenEXR/ImathMatrixAlgo.h"


namespace IECore
{

IE_CORE_FORWARDDECLARE( SkeletonPrimitive )

/** Class that represent an entire skeletal structure.
 *
 *  Internally this class stores two M44fVectorData, the first vector of matrices contains what I called @b default @b matrices
 *  and the second vector of matrices contained what I called @b local @b matrices.
 *  The default pose stores any pose required, often this is the bind pose or any other pose wanted for reference.
 *  Changing the local matrix over time will produce an animation.
 *  SkeletonPrimitive stores also a StringVectorData (the name of each joint) and an IntVectorData which containes all the parents id for each joint.
 *  For example a simple structure like this one:
 *  joint0 -> joint1 -> joint2, where "->" represent hierarchical relationship, the parent ids are -1, 0, 1 (-1 means no parent).
 */
class SkeletonPrimitive : public Primitive
{

	public:
		IE_CORE_DECLAREOBJECT( SkeletonPrimitive, Primitive );

		/** Spaces that is possible to work in */
		enum Space { Local = 0,
					World = 2 };

		/** Generates an empty Skeleton */
		SkeletonPrimitive();
		/** Generates a copy of the given SkeletonPrimitivePtr */
		SkeletonPrimitive( ConstSkeletonPrimitivePtr other );
		/** Generates a new SkeletonPrimitive.
		 *
		 *  @param poses
		 *  	a M44fVectorDataPtr containing the poses of each joint.
		 *  	These poses can be in Reference, Local or Global space (see below).
		 *  @param parentIds
		 *  	The parents indices
		 *  @param space
		 *  	Describe the space in which the given poses will be stored.
		 *  	If its value is Reference than the internal M44fVectorData that describes the reference poses will be set
		 *  	with these values and theM44fVectorData that represents the local poses will be set with identity matrices,
		 *  	if its value is Local, then the opposite of the previous operation will be performed.
		 *  	If its value is Global the Reference matrices will be all identity and the local ones will have an opportune value. */
		SkeletonPrimitive( ConstM44fVectorDataPtr poses, ConstIntVectorDataPtr parentIds, Space space=World);

		virtual void render( Renderer *renderer ) const;
		virtual Imath::Box3f bound() const;
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;

		/** makes this SkeletonPrimitive a copy of the given one */
		void setAsCopyOf( ConstSkeletonPrimitivePtr other );
		/** Checks this SkeletonPrimitive's data is @b almost similar to the given SkeletonPrimitive's data, within an approximation of 1e-06f*/
		bool isSimilarTo( ConstObjectPtr other ) const;

		/** Adds another joint to this SkeletonPrimitive.
		 *
		 *  @param parentId
		 *  	The id of the new joint's parent (-1 if there's no parent)
		 *  @param name
		 *  	A name for this new joint */
		void addJoint(int parentId, std::string name="joint");
		/** Returns the number of joints contained in this SkeletonPrimitive */
		unsigned int numJoints() const { return m_defaultPose->readable().size(); }

		/** Sets the matrices of all the joints in this SkeletonPrimitive.
		 *
		 *  @param poses
		 *  	a M44fVectorDataPtr containing the poses of each joint.
		 *  	These poses can be in Reference, Local or Global space
		 *  @param space
		 *  	Describe the space in which the given poses will be stored. */

		// TODO: should just be "setPose", as a pose is defined as an array of matrices in cortex
		void setJointPoses( ConstM44fVectorDataPtr poses, Space space=World );
		/** Sets one single joint's matrix.
		 *
		 *  @param jointId
		 *  	The Id of the joint is going to be changed
		 *  @param pose
		 *  	The new "pose" for the joint. It can be in Reference, Local or World space.
		 *  @param space
		 *  	Describe the space in which the given pose will be stored.*/
		void setJointPose( unsigned int jointId, ConstM44fDataPtr pose, Space space=World );

		/** Gets all the joint's matrices in the specified space */
		// TODO: should just be "getPose", see above
		M44fVectorDataPtr getJointPoses( Space space=World );
		/** Gets the matrix of one sigle joint in the specified space */
		M44fDataPtr getJointPose( unsigned int jointId, Space space=World );

		// TODO: should just be "setDefaultPose", seeAbove
		void setDefaultPoses( ConstM44fVectorDataPtr poses );
		// TODO: should just be "getDefaultPose", seeAbove
		M44fVectorDataPtr getDefaultPoses();
		void applyDefaultPose();

		IntVectorDataPtr getParentIds() const;
		int getParentId( unsigned int jointId ) const;
		IntVectorDataPtr getChildrenIds(unsigned int jointId) const;

		void setJointNames( const StringVectorDataPtr names );
		void setJointName( unsigned int jointId, const std::string &name );
		StringVectorDataPtr getJointNames() const;
		std::string getJointName( unsigned int jointId ) const;

		/** Shares the Reference matrices with the given SkeletonPrimitive.
		 *  This makes possible to have two or more SkeletonPrimitve with the same rest pose but different animation */
		void shareStaticData( SkeletonPrimitivePtr other );
		/** Shares the Local matrices with the given SkeletonPrimitive.
		 *  This makes possible to have two or more SkeletonPrimitve with different rest pose but same animation */
		void shareAnimatableData( SkeletonPrimitivePtr other );

		/** Recomputes the internal @b global matrices @b pulling the process from the joint with the given id @b up to the hierarchy structure. */
		void pullUpdate( unsigned int jointId );
		/** Recomputes the internal @b global matrices @b pushing the process from the joint with the given id @b down to the hierarchy structure. */
		void pushUpdate( unsigned int jointId );
		/** Searchs the hierarchy structure and find the root joint's id. The search starts form the joint with the given Id */
		int getRootJointId( unsigned int fromId=0 ) const;

		/** Recomputes the internal @b global matrices for the entire hierachycal structure.
		 *  This is equivalent to call getRootJoint first and then use the id returned to call pushUpadate.*/
		void update();

		/** Set the joints radius.
		 *  @note: Only useful to render the joints */
		// TODO: should just be "setJointRadius", seeAbove
		void setRadius(float radius);
		/** Activate or deactivate an internal debug flag.
		 *  @note: Only useful to render the joints, for example when this flag is set to true, it can display each joint axis */
		// TODO: should be a better name like, "glDisplayJointAxis" or something
		void setDebug(bool debug);
		// TODO: should be a better name like, "getJointRadius" or something
		float getRadius();
		bool getDebug();

		/** Generates a SkeletonPrimitives to represent a very simple and rough human skeleton. Particularly useful in quick tests */
		static SkeletonPrimitivePtr createHuman();

	private:
		static const unsigned int m_ioVersion;

		// ******* the hierarchy vectors ******* //
		StringVectorDataPtr m_jointNames;
		IntVectorDataPtr   	m_parentIds;

		// ******* a whole pose that acts as default pose for the character ******* //
		M44fVectorDataPtr   m_defaultPose;

		// ******* the matrix that represent the animation part ******* //
		M44fVectorDataPtr   m_localMatrices;

		// ******* internal storaged data for easy access ******* //
		std::vector< std::vector<int> >  m_childrenIds;
		M44fVectorDataPtr  m_globalMatrices;

		// ******* useful to render the joints ******* //
		bool m_jointsAxis;
		float m_jointsRadius;

		void computeGlobalTransform(int jointId);
		void setFromGlobalMatrices(int jointId, ConstM44fVectorDataPtr matrices);

		void synchVectorIds();
};

}

#endif // IECORE_SKELETONPRIMITIVE_H
