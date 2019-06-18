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

#ifndef IE_COREHOUDINI_NODEHANDLE_H
#define IE_COREHOUDINI_NODEHANDLE_H

#include "IECoreHoudini/Export.h"

#include "HOM/HOM_Node.h"
#include "OP/OP_Node.h"

#include "boost/shared_ptr.hpp"

namespace IECoreHoudini
{

/// The NodeHandle is a class that may be used to verify the existence of an OP_Node.
/// Use a NodeHandle if you want a raw pointer to an OP_Node, but are concerned about
/// the lifespan of that OP_Node.
class IECOREHOUDINI_API NodeHandle
{

	public :

		NodeHandle();
		NodeHandle( const OP_Node *node );

		virtual ~NodeHandle();

		/// Returns True if the OP_Node represented by this handle is still alive
		bool alive() const;

		/// Returns a pointer to the OP_Node represented by this handle, or 0 if alive is false.
		OP_Node *node() const;

	private :

		// we are using a HOM_Node because it lets us know if the OP_Node has been deleted
		boost::shared_ptr<HOM_Node> m_homNode;

};

} // namespace IECoreHoudini

#endif // IE_COREHOUDINI_NODEHANDLE_H
