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

#ifndef IE_CORE_PRIMITIVEMOTION_H
#define IE_CORE_PRIMITIVEMOTION_H

#include "IECore/Primitive.h"

namespace IECore
{

/// The MotionPrimitive is a simple means of constructing and rendering a
/// motionBegin/motionEnd block containing a Primitive. There's nothing to
/// stop you from calling motionBegin/motionEnd to construct motion blocks yourself
/// but this form is simple, and loadable and savable and copyable and all that.
class MotionPrimitive : public VisibleRenderable
{
	public:

		/// A type to map between points in time and corresponding
		/// Primitives.
		typedef std::map<float, PrimitivePtr> SnapshotMap;

		MotionPrimitive();
		virtual ~MotionPrimitive();

		IE_CORE_DECLAREOBJECT( MotionPrimitive, VisibleRenderable );

		//! @name Snapshots
		////////////////////////////////////////////////////
		//@{
		/// Gives const access to the internal snapshot data.
		const SnapshotMap &snapshots() const;
		/// Gives access to the internal snapshot data. This can
		/// be modified freely - it'll be validated in render().
		SnapshotMap &snapshots();
		/// Utility function to insert a time,primitive pair
		/// into snapshots().
		void addSnapshot( float time, PrimitivePtr primitive );
		/// Utility function to remove a snapshot.
		void removeSnapshot( float time );
		void removeSnapshot( PrimitivePtr primitive );
		//@}

		/// May throw an Exception if the contained Primitives are not
		/// compatible with each other.
		virtual void render( RendererPtr renderer ) const;
		virtual Imath::Box3f bound() const;

	private:

		static const unsigned int m_ioVersion;

		SnapshotMap m_snapshots;

};

IE_CORE_DECLAREPTR( MotionPrimitive );

} // namespace IECore

#endif // IE_CORE_PRIMITIVEMOTION_H
