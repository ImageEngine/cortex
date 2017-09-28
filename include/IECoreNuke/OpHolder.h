//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_OPHOLDER_H
#define IECORENUKE_OPHOLDER_H

#include "DDImage/Op.h"
#include "DDImage/Executable.h"

#include "IECore/Op.h"

#include "IECoreNuke/ParameterisedHolder.h"

namespace IECoreNuke
{

/// This class allows IECore::Op objects to be executed by nodes in Nuke.
class OpHolder : public ParameterisedHolderOp, public DD::Image::Executable
{

	public :

		OpHolder( Node *node );
		virtual ~OpHolder();

		//! @name Reimplementation of Nuke methods.
		/////////////////////////////////////////////////////////////////////
		//@{
		virtual const char *Class() const;
		virtual const char *node_help() const;
		virtual DD::Image::Executable *executable();
		virtual void execute();
		/// \todo We /are/ threadsafe, but Nuke doesn't release the
		/// GIL when calling through to here from nuke.execute().
		/// We therefore have to pretend not to be threadsafe - if
		/// they fix this we can return true from this instead of false.
		virtual bool isExecuteThreadSafe() const;
		virtual bool isWrite();
		//@}

		/// Executes the held IECore::Op and returns the result.
		virtual IECore::ObjectPtr engine();

	private :

		static const Description g_description;
		static DD::Image::Op *build( Node *node );

		IECore::ObjectPtr m_result;
		DD::Image::Hash m_resultHash;

		static IECore::ObjectPtr executeResult();

		friend void bindFnOpHolder();

};

} // namespace IECoreNuke

#endif // IECORENUKE_OPHOLDER_H
