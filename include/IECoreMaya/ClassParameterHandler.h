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

#ifndef IECOREMAYA_CLASSPARAMETERHANDLER_H
#define IECOREMAYA_CLASSPARAMETERHANDLER_H

#include "IECoreMaya/ParameterHandler.h"

namespace IECoreMaya
{

/// A ParameterHandler which deals with ClassParameters. Note that no attempt is made to represent the child
/// parameters - separate calls to other parameter handlers should be used to do that.
class IECOREMAYA_API ClassParameterHandler : public ParameterHandler
{

	public :

		/// Convenience function for setting the class held by parameter. This makes no changes to the maya representation
		/// of the parameter whatsoever.
		static MStatus setClass( IECore::ParameterPtr parameter, const MString &className, int classVersion, const MString &searchPathEnvVar );
		/// Convenience function for getting the class held by parameter. This has nothing to do with the maya
		/// representation of the class whatsoever - see below for that.
		static MStatus getClass( IECore::ConstParameterPtr parameter, MString &className, int &classVersion, MString &searchPathEnvVar );

		/// Can be used to query the details of the class currently being held by the parameter represented
		/// by the specified plug. Note that this returns the class currently being represented in maya,
		/// which may not be the same as the class on the parameter if the two have not been synchronised by a call
		/// to ParameterisedHolderInterface::updateParameterised.
		/// This function typically shouldn't be called directly, as a combination of direct access
		/// to parameters and IECoreMaya.FnParameterisedHolder should be enough to achieve most things.
		static void currentClass( const MPlug &plug, MString &className, int &classVersion, MString &searchPathEnvVar );

	protected:

		virtual MPlug doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const;
		virtual MStatus doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const;
		virtual MStatus doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const;
		virtual MStatus doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const;
		virtual MStatus doRestore( const MPlug &plug, IECore::ParameterPtr parameter ) const;

	private :

		static MStatus storeClass( IECore::ConstParameterPtr parameter, MPlug &plug );


};

} // namespace IECoreMaya

#endif // IECOREMAYA_CLASSPARAMETERHANDLER_H
