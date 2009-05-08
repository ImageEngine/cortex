//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_CHANNELOP_H
#define IECORE_CHANNELOP_H

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// A base class for operations which modify a selection of
/// channels on an ImagePrimitive.
class ChannelOp : public ImagePrimitiveOp
{
	public:

		ChannelOp( const std::string &name, const std::string &description );
		virtual ~ChannelOp();

		IE_CORE_DECLARERUNTIMETYPED( ChannelOp, ImagePrimitiveOp );

		StringVectorParameterPtr channelNamesParameter();
		ConstStringVectorParameterPtr channelNamesParameter() const;

	protected :

		typedef std::vector<DataPtr> ChannelVector;

		/// Should be implemented by derived classes to modify the data in the passes channels in place.
		/// The base class will already have verified the following :
		///
		///		* the channels have an appropriate interpolation value - vertex, varying or facevarying.
		/// 		* the channels contain the appropriate number of elements for the dataWindow.
		///		* the channels are all of type FloatVectorData, HalfVectorData or IntVectorData.
		///		* the dataWindow is not empty.
		/// \todo ChannelVector doesn't contain any indicator as to which channel is which, so why not just pass a single channel at a time? As
		/// things are right now, every derived class is iterating over the channels vector - there's not much else they can do - so it would
		/// make sense to move that step to the base class.
		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels ) = 0;

	private :

		/// Implemented to call modifyChannels().
		virtual void modifyTypedPrimitive( ImagePrimitivePtr image, ConstCompoundObjectPtr operands );

		StringVectorParameterPtr m_channelNamesParameter;

};

IE_CORE_DECLAREPTR( ChannelOp );


} // namespace IECore

#endif // IECORE_CHANNELOP_H

