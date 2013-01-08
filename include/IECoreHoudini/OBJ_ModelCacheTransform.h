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

#ifndef IECOREHOUDINI_OBJMODELCACHETRANSFORM_H
#define IECOREHOUDINI_OBJMODELCACHETRANSFORM_H

#include "OBJ/OBJ_SubNet.h"
#include "UT/UT_Matrix.h"

#include "IECore/ModelCache.h"

#include "IECoreHoudini/ModelCacheNode.h"

namespace IECoreHoudini
{

/// OBJ for loading a single transform from an IECore::ModelCache
/// \todo: add option to build underlying hierarchy by nesting OBJ_ModelCacheTransforms
/// \todo: add option to build underlying hierarchy by chaining OBJ_ModelCacheSources (once they exist)
class OBJ_ModelCacheTransform : public ModelCacheNode<OBJ_SubNet>
{
	public :
		
		OBJ_ModelCacheTransform( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~OBJ_ModelCacheTransform();
		
		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );
		static OP_TemplatePair *buildParameters();
	
	protected :
		
		virtual OP_ERROR cookMyObj( OP_Context &context );

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_OBJMODELCACHETRANSFORM_H
