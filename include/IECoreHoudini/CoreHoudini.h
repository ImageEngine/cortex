//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_COREHOUDINI_H
#define IECOREHOUDINI_COREHOUDINI_H

// \todo: libIECoreHoudini should not use python. Move any use case to the IECoreHoudini bindings
#include "boost/python.hpp"

#include "IECoreHoudini/Export.h"

#include "IECore/Object.h"

#include "GU/GU_DetailHandle.h"

#include <string>
#include <vector>

/// The IECoreHoudini namespace holds all the functionality of libIECoreHoudini.
namespace IECoreHoudini
{

class IECOREHOUDINI_API CoreHoudini
{
	public:

		/// This loads hou into the global context
		static void initPython();
		/// Used to cleanup any python globals during module shutdown
		static void cleanupPython();

		/// Utility method to import a python module into the global context
		static void import( const std::string &module );

		/// Utility method for getting the global python context
		static boost::python::object &globalContext()
		{
			return g_globalContext;
		}

		/// Run misc python command
		static boost::python::object evalPython( const std::string &cmd );

	private:
		/// our global context
		static boost::python::object g_globalContext;

		/// initialized
		static bool g_initialized;
};

}

#endif // IECOREHOUDINI_COREHOUDINI_H
