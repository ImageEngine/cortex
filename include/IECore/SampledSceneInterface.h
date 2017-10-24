//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SAMPLEDSCENEINTERFACE_H
#define IECORE_SAMPLEDSCENEINTERFACE_H

#include "IECore/Export.h"
#include "IECore/SceneInterface.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( SampledSceneInterface );


/// A pure virtual base class for navigating a hierarchical sampled-animated 3D scene.
/// The calls to the base class functions readFoo() will return interpolated values from
/// the stored sampled data. They may return the closest sample if the object cannot be interpolated.
/// The functions numFooSamples() can be used in combination with fooSampleTime() and
/// readFooAtSample() to read the exact stored samples without interpolation.
/// The functions fooSampleInterval should be used when the interpolation provided
/// by the functions readFoo are not suitable. These functions
/// return the two closest samples that enclose the given time and also
/// return the appropriate lerp factor between the two samples.
/// In the case of time falling outside of the sample range, or coinciding
/// nearly exactly with a single sample, 0 is returned and floorIndex==ceilIndex
/// will hold.
/// \ingroup ioGroup
class IECORE_API SampledSceneInterface : public SceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( SampledSceneInterface, SceneInterface );

		~SampledSceneInterface() override = 0;

		/// Returns the number of bounding box samples are available for reading
		virtual size_t numBoundSamples() const = 0;
		/// Returns the number of transform samples are available for reading
		virtual size_t numTransformSamples() const = 0;
		/// Returns the number of attribute samples are available for reading
		virtual size_t numAttributeSamples( const SceneInterface::Name &name ) const = 0;
		/// Returns the number of object samples are available for reading
		virtual size_t numObjectSamples() const = 0;

		/// Returns the time associated with the specified bounding box sample.
		virtual double boundSampleTime( size_t sampleIndex ) const = 0;
		/// Returns the time associated with the specified transform sample.
		virtual double transformSampleTime( size_t sampleIndex ) const = 0;
		/// Returns the time associated with the specified attribute sample.
		virtual double attributeSampleTime( const SceneInterface::Name &name, size_t sampleIndex ) const = 0;
		/// Returns the time associated with the specified object sample.
		virtual double objectSampleTime( size_t sampleIndex ) const = 0;

		/// Returns the local bounding box of this node stored for the specified sample.
		virtual Imath::Box3d readBoundAtSample( size_t sampleIndex ) const = 0;
		/// Returns the transform applied to this path within the scene.
		virtual ConstDataPtr readTransformAtSample( size_t sampleIndex ) const = 0;
		/// Returns the transform applied to this path within the scene as a matrix.
		virtual Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const = 0;
		/// Reads the named attribute applied to this path within the scene.
		virtual ConstObjectPtr readAttributeAtSample( const SceneInterface::Name &name, size_t sampleIndex ) const = 0;
		/// Reads the object stored at this path in the scene - may
		/// return 0 when no object has been stored.
		virtual ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const = 0;

		/// Computes a sample interval suitable for use in producing interpolated bounding box values.
		virtual double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const = 0;
		/// Computes a sample interval suitable for use in producing interpolated transform values.
		virtual double transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const = 0;
		/// Computes a sample interval suitable for use in producing interpolated attribute values.
		virtual double attributeSampleInterval( const SceneInterface::Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const = 0;
		/// Computes a sample interval suitable for use in producing interpolated objects
		virtual double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const = 0;

		/// Implementations of base class methods
		/// =====================================
		///
		/// These are implemented using the `read*AtSample()` and
		/// `sample*Interval()` methods above. Derived classes may
		/// reimplement them again, but typically this should not
		/// be necessary.

		Imath::Box3d readBound( double time ) const override;
		ConstDataPtr readTransform( double time ) const override;
		/// Implemented using readTransform() rather than readTransformMatrixAtSample(),
		/// as it potentially provides improved interpolation.
		Imath::M44d readTransformAsMatrix( double time ) const override;
		ConstObjectPtr readAttribute( const Name &name, double time ) const override;
		ConstObjectPtr readObject( double time ) const override;

};


} // namespace IECore

#endif // IECORE_SAMPLEDSCENEINTERFACE_H
