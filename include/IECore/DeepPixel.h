//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DEEPPIXEL_H
#define IECORE_DEEPPIXEL_H

#include <string>
#include <vector>

#include "IECore/RefCounted.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( DeepPixel )

/// A DeepPixel represents arbitrary channel data stored at varying depths in space.
/// By convention, depth is measured as distance from the eye plane
/// \ingroup deepCompositingGroup
class DeepPixel : public RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( DeepPixel );

		/// Constructs a new DeepPixel. Each character of the string will be considered a
		/// seperate channel. For multi-character channel names, use the alternate constructor.
		/// In either case, numSamples is only used to reserve the appropriate amount of space.
		/// It does not actually initialize data or add default samples.
		DeepPixel( const std::string channelNames = "RGBA", unsigned numSamples = 0 );
		DeepPixel( const std::vector<std::string> &channelNames, unsigned numSamples = 0 );
		
		DeepPixel( const DeepPixel &pixel );
		
		virtual ~DeepPixel();
		
		//! @name Depth Samples
		/// A DeepPixel contains interleaved channel data at many depth samples. Each
		/// DeepPixel may contain any number of depth samples, but each depth sample
		/// must contain data for all of the channels in the DeepPixel.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		/// The number of depth samples
		unsigned numSamples() const;
		/// The nearest depth
		float min() const;
		/// The farthest depth
		float max() const;
		
		/// The depth of an individual sample
		float getDepth( unsigned index ) const;
		/// Sets the depth for the indexed depth sample
		void setDepth( unsigned index, float depth );
		
		/// Adds a new depth sample
		void addSample( float depth, const float *channelData );
		/// Removes an existing depth sample
		void removeSample( unsigned index );
		
		/// Returns all channel data for the indexed depth sample
		float *channelData( unsigned index );
		const float *channelData( unsigned index ) const;
		/// Returns all channel data interpolated to the given depth
		/// \todo: should there be a parameter for interpolation type, or is linear enough?
		void interpolatedChannelData( float depth, float *result ) const;
		//@}
		
		//! @name Channels
		/// Each depth sample contains data for each of the channels in the DeepPixel.
		/// It can be assumed that every DeepPixel from a single DeepImage will contain
		/// the same channels. As such, channels cannot be added or destroyed once a
		/// DeepPixel has been initialized. Note that RGBA are considered 4 separate
		/// channels, and depth is not considered a channel. The channel data is
		/// considered uncomposited; that is to say the channel values match exactly
		/// the value at each depth sample, and not a summation of values over depth.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		/// Returns the number of channels
		unsigned numChannels() const;
		/// Get the index for the named channel.
		int channelIndex( const std::string &name ) const;
		/// Returns the names of all channels
		const std::vector<std::string> *channelNames() const;
		//@}
		
		//! @name Deep Compositing
		/// These are methods to aid in Deep Compositing. All DeepPixels must contain
		/// the same channels, though they may have a varying number of depth samples.
		/// \todo: should these be methods or should they go in DeepPixelAlgo.h
		//////////////////////////////////////////////////////////////////////////////
		//@{
		/// Merge the given DeepPixel into this one.
		void merge( const DeepPixel *pixel );
		/// Fills result with the composited channel data, having accumulated the depth samples.
		void composite( float *result ) const;
		/// Returns a new DeepPixel containing the weighted average of the given DeepPixels.
		/// All pixels will be resampled at the depths associated with the first pixel.
		static DeepPixelPtr average( std::vector<const DeepPixel *> &pixels, std::vector<float> &weights );
		//@}
	
	private :
		
		class DepthComparison;
		
		void sort();
		// While technically sorting is not const, the sort doesn't affect any data exposed to
		// the user, and is required by various const query methods since we are lazily sorting
		// the heap of depth indices.
		void sort() const;
		
		bool m_sorted;
		std::vector<unsigned> m_depthIndices;
		std::vector<float> m_samples;
		std::vector<std::string> m_channels;

};

IE_CORE_DECLAREPTR( DeepPixel );

} // namespace IECore

#endif // IECORE_DEEPPIXEL_H
