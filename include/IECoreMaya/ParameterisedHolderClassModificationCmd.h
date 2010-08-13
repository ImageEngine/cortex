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

#ifndef IECOREMAYA_PARAMETERISEDHOLDERCLASSMODIFICATIONCMD_H
#define IECOREMAYA_PARAMETERISEDHOLDERCLASSMODIFICATIONCMD_H

#include <algorithm>

#include "maya/MPxCommand.h"
#include "maya/MSyntax.h"
#include "maya/MStringArray.h"

#include "IECoreMaya/ParameterisedHolderInterface.h"
#include "IECoreMaya/MArrayIter.h"

namespace IECoreMaya
{

// This class has two purposes :
//
// 1) It is used by FnParameterisedHolder.setParameterised() to implement
// changing of the held class in an undoable way.
// 
// 2) It is used by FnParameterisedHolder.parameterModificationContext()
// for the changing of the classes held by ClassParameter and ClassVectorParameter,
// and the setting of Parameter values. It must be implemented here as a command
// so that we can support undo for these operations.
//
// Under no circumstances should this class or the command it creates be
// used directly - it should be considered to be a private implementation
// detail of FnParameterisedHolder.
class ParameterisedHolderClassModificationCmd : public MPxCommand
{

	public :

		ParameterisedHolderClassModificationCmd();
		virtual ~ParameterisedHolderClassModificationCmd();

		static void *creator();

		virtual bool isUndoable() const;
		virtual bool hasSyntax() const;

		virtual MStatus doIt( const MArgList &argList );
		virtual MStatus undoIt();
		virtual MStatus redoIt();

	private :

		struct ClassParameterInfo
		{
			MString className;
			int classVersion;
			MString searchPathEnvVar;
			bool operator == ( const ClassParameterInfo &other ) const
			{
				return className==other.className && classVersion==other.classVersion && searchPathEnvVar==other.searchPathEnvVar;
			}
			bool operator != ( const ClassParameterInfo &other ) const
			{
				return !(*this == other);
			}
		};
		typedef std::map<std::string, ClassParameterInfo> ClassParameterInfoMap;
		
		struct ClassVectorParameterInfo
		{
			MStringArray parameterNames;
			MStringArray classNames;
			MIntArray classVersions;
			bool operator == ( const ClassVectorParameterInfo &other ) const
			{
				if( parameterNames.length() != other.parameterNames.length() ||
					classNames.length() != other.classNames.length() ||
					classVersions.length() != other.classVersions.length() )
				{
					return false;
				}
				int l = parameterNames.length();
				for( int i=0; i<l; i++ )
				{
					if( parameterNames[i] != other.parameterNames[i] ||
						classNames[i] != other.classNames[i] ||
						classVersions[i] != other.classVersions[i]
					)
					{
						return false;
					}
				}
				return true;
			}
			bool operator != ( const ClassVectorParameterInfo &other ) const
			{
				return !(*this == other);
			}
		};
		typedef std::map<std::string, ClassVectorParameterInfo> ClassVectorParameterInfoMap;

		struct ClassInfo
		{
			ClassParameterInfoMap classParameters;
			ClassVectorParameterInfoMap classVectorParameters;
		};
		
		void storeClassParameterStates( ClassInfo &classInfo, const IECore::Parameter *parameter, const std::string &parentParameterPath, bool changedOnly );
		void restoreClassParameterStates( const ClassInfo &classInfo, IECore::Parameter *parameter, const std::string &parentParameterPath );
		void storeParametersWithNewValues( const IECore::Object *originalValue, const IECore::Object *newValue,  const std::string &parameterPath );
		void setNodeValuesForParametersWithNewValues() const;
		void setNodeValue( IECore::Parameter *parameter ) const;
		void despatchSetParameterisedCallbacks() const;
		void despatchClassSetCallbacks() const;
		IECore::Parameter *parameterFromPath( IECore::ParameterisedInterface *parameterised, const std::string &path ) const;
		
		MObject m_node;
		ParameterisedHolderInterface *m_parameterisedHolder;
		
		ClassInfo m_originalClassInfo;
		ClassInfo m_newClassInfo;
		
		IECore::ObjectPtr m_originalValues;
		IECore::ObjectPtr m_newValues;
		std::set<std::string> m_parametersWithNewValues;
		
		bool m_changingClass;
		MString m_originalClassName;
		int m_originalClassVersion;
		MString m_originalSearchPathEnvVar;
		
		MString m_newClassName;
		int m_newClassVersion;
		MString m_newSearchPathEnvVar;
		
		// When using FnParameterisedHolder.classParameterModificationContext(), it is too late
		// to calculate the parameter value to undo back to in this command, so that value
		// is passed in from the context manager instead.
		static IECore::ObjectPtr g_undoValue;
		friend void parameterisedHolderAssignUndoValue( IECore::ObjectPtr value );
		
};

}

#endif // IECOREMAYA_PARAMETERISEDHOLDERCLASSMODIFICATIONCMD_H
