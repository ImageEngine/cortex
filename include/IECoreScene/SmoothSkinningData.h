//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_SMOOTHSKINNINGDATA_H
#define IECORESCENE_SMOOTHSKINNINGDATA_H

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/Data.h"
#include "IECore/VectorTypedData.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( SmoothSkinningData )

/// Defines a data class for storing smooth skinning data along with influenceNames and their respective pre-bind matrices.
/// SmoothSkinningData stores bind information for points that can be deformed using multiple weighted transforms.
/// The most common usecase is probably smooth skinning / smooth binding of points on a geometry to a skeleton hierarchy.
///
/// Consider the following example for some valid SmoothSkinningData:
///
///    influenceNames = [ infA, infB ] (this is a StringVector)
///    influencePose  = [ trfA, trfB ] (this is a M44fVector)
///    pointInfluenceCounts = [ 1, 1, 2, 2, 1 ]
///    pointIndexOffsets =    [ 0, 1, 2, 4, 6 ]
///    pointInfluenceIndices = [   0,   0,   0,   1,   1,   0,   1 ]
///    pointInfluenceWeights = [ 1.0, 1.0, 0.5, 0.5, 0.1, 0.9, 1.0 ]
///
/// The above SmoothSkinningData (SSD) stores smooth skinning information for 5 points (P) influenced by 2 transforms.
/// The number of points is only implicitly stored and equals the length of the 'pointIndexOffsets' and 'pointInfluenceCounts' arrays.
/// For fast access, the weighting information, which is being held in 'pointInfluenceIndices' and 'pointInfluenceWeights',
/// is stored in flat arrays. To retrieve this information for a specific point, one has to use two helper arrays:
/// 'pointInfluenceCounts' indicates how many influences do influence the point. 'pointIndexOffsets' stores for each
/// point where in the weighting info arrays the data specific to the point is stored.
///
/// So if we wanted to the skinning info for the 4th point, we'd do the following:
///
///    pid = 3
///    pio = pointIndexOffsets[pid]    // = 4
///    pic = pointInfluenceCounts[pid] // = 2
///
/// With this information we can now index into the skinning info arrays
///
///    for (i = pio; i < pio + pic; i++ )
///    {
///       pii.push_back( pointInfluenceIndices[i] )
///       piw.push_back( pointInfluenceWeights[i] )
///    }
///
/// This gives us the indices of the influences on the point (pii = [1, 0]) and their weighting ( piw = [0.1, 0.9] ).
/// The 'pointInfluenceIndices' are referring to the index of our data in the 'influencePose' and 'influenceNames'
/// arrays. In our example, the 4th point is influenced by 0.9*trfA and 0.1*trfB. The 2nd point is influenced by 1.0*trfA
/// and the third point is influenced by 50% from both infA and infB.
/// \ingroup skinningGroup
class IECORESCENE_API SmoothSkinningData : public IECore::Data
{
	public:

		IE_CORE_DECLAREEXTENSIONOBJECT( SmoothSkinningData, SmoothSkinningDataTypeId, IECore::Data );

		/// Returns the names of the influence objects, it is used for reference.
		const IECore::StringVectorData *influenceNames() const;
		IECore::StringVectorData *influenceNames() ;

		/// Returns a pose (an array of matrices) describing the pre-bind, world-space
		/// transformation of the influence objects. This array has the same length than'influenceNames'.
		const IECore::M44fVectorData *influencePose() const;
		IECore::M44fVectorData *influencePose() ;

		/// Returns an array of indices indicating where in the
		/// 'pointInfluenceIndices' and 'pointInfluenceWeights' arrays the smooth skinning information
		/// for the particular point can be found. The array holds one entry per deformable point.
		const IECore::IntVectorData *pointIndexOffsets() const;
		IECore::IntVectorData *pointIndexOffsets() ;

		/// Returns an array of counts (one entry per point) indicating how many entries in the
		/// 'pointInfluenceIndices' and 'pointInfluenceWeights' arrays from the respective 'pointIndexOffsets'
		/// are holding the smooth skinning information for the point.
		/// The array holds one entry per deformable point.
		const IECore::IntVectorData *pointInfluenceCounts() const;
		IECore::IntVectorData *pointInfluenceCounts() ;

		/// Returns an array that holds all influence indices for all points. The length of the array matches
		/// the 'pointInfluenceWeights'. The indices are refering to the respective index in the 'influencePose'
		/// and 'influenceNames' arrays.
		const IECore::IntVectorData *pointInfluenceIndices() const;
		IECore::IntVectorData *pointInfluenceIndices() ;

		/// Returns an array that holds all influence weights for all points. The length of the array matches
		/// the 'pointInfluenceIndices' length.
		const IECore::FloatVectorData *pointInfluenceWeights() const;
		IECore::FloatVectorData *pointInfluenceWeights() ;

		/// Assign-All Constructor
		SmoothSkinningData( IECore::ConstStringVectorDataPtr influenceNames,
							IECore::ConstM44fVectorDataPtr influencePose,
							IECore::ConstIntVectorDataPtr pointIndexOffsets,
							IECore::ConstIntVectorDataPtr pointInfluenceCounts,
							IECore::ConstIntVectorDataPtr pointInfluenceIndices,
							IECore::ConstFloatVectorDataPtr pointInfluenceWeights);

		/// Default constructor
		SmoothSkinningData();

		~SmoothSkinningData() override;

		/// raises an exception if the smooth skinning data is not valid
		void validate() const;

	private:

		static const unsigned int m_ioVersion;

		IECore::StringVectorDataPtr m_influenceNames;
		IECore::M44fVectorDataPtr m_influencePose;
		IECore::IntVectorDataPtr m_pointIndexOffsets;
		IECore::IntVectorDataPtr m_pointInfluenceCounts;
		IECore::IntVectorDataPtr m_pointInfluenceIndices;
		IECore::FloatVectorDataPtr m_pointInfluenceWeights;

		void validateSizes() const;
		void validateCounts() const;
		void validateIds() const;
		void validateOffsets() const;
};

IE_CORE_DECLAREPTR( SmoothSkinningData )

} // namespace IECoreScene

#endif // IECORESCENE_SMOOTHSKINNINGDATA_H
