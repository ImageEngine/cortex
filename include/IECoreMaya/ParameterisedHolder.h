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

#ifndef IE_COREMAYA_PARAMETERISEDHOLDER_H
#define IE_COREMAYA_PARAMETERISEDHOLDER_H

class MPxNode;
class MPxLocatorNode;
class MPxDeformerNode;
class MPxObjectSet;
class MPxFieldNode;
class MPxSurfaceShape;
class MPxComponentShape;
class MPxImagePlane;

#include "IECore/Parameterised.h"

#include "IECoreMaya/ParameterisedHolderInterface.h"
#include "IECoreMaya/MStringLess.h"
#include "IECoreMaya/PostLoadCallback.h"

namespace IECoreMaya
{

/// A base class from which nodes to hold IECore::Parametised objects
/// should derive (for example, Maya RI procedurals). It's templated
/// to allow inheritance from any Maya proxy.
///
/// This class represents the hierarchy of parameters in a flattened form
/// using name munging to generate unique names for the maya attributes. We
/// would much rather it used maya compound attributes to maintain the hierarchy
/// but this is problematic - it seems the maya api doesn't implement the
/// on the fly modification of compound attributes after they've been added to a
/// node, and that is required by a series of changing calls to setParameterised().
/// Maya also requires the names of children of nested compounds to be unique to
/// the node anyway, forcing a name munging approach even in the case that compound
/// usage was possible.
template< typename BaseType >
class ParameterisedHolder : public BaseType, public ParameterisedHolderInterface
{

	public:

		ParameterisedHolder();
		virtual ~ParameterisedHolder();

		static void *creator();
		static MStatus initialize();

		/// This is a template class instantiated into many different
		/// classes, so we specialise these in the implementation.
		static MTypeId id;
		static MString typeName;

		virtual void postConstructor();
		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );
		virtual MStatus shouldSave( const MPlug &plug, bool &isSaving );
		virtual void copyInternalData( MPxNode *node );

		//! @name ParameterisedHolderInterface implementation
		/////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Set the node to hold a particular Parameterised object. When using this version
		/// of setParameterised the node will not be able to preserve the object across scene
		/// save/load - this becomes your responsibility if it's necessary.
		virtual MStatus setParameterised( IECore::RunTimeTypedPtr p );
		virtual MStatus setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar );
		virtual MStatus updateParameterised();
		virtual IECore::RunTimeTypedPtr getParameterised( std::string *className = 0, int *classVersion = 0, std::string *searchPathEnvVar = 0 );
		virtual MStatus setNodeValues();
		virtual MStatus setNodeValue( IECore::ParameterPtr pa );
		virtual MStatus setParameterisedValues();
		virtual MStatus setParameterisedValue( IECore::ParameterPtr pa );
		virtual MPlug parameterPlug( IECore::ConstParameterPtr parameter );
		virtual IECore::ParameterPtr plugParameter( const MPlug &plug );
		//@}

		//! @name Attributes
		/////////////////////////////////////////////////////////////////////////////////////////
		static MObject aParameterisedClassName;
		static MObject aParameterisedVersion;
		static MObject aParameterisedSearchPathEnvVar;
		static MObject aDynamicParameters;
		//@}

	private:

		IECore::RunTimeTypedPtr loadClass( const MString &className, int classVersion, const MString &searchPathEnvVar );

		/// Creates (or updates existing) attributes for each parameter. Removes any old attributes no longer
		/// needed. If dynamicParameterStorage is specified then any new Parameters for which attributes must be
		/// created are placed in there - this is used by updateParameterised() to store dynamic parameters.
		MStatus createAndRemoveAttributes( IECore::CompoundObjectPtr dynamicParameterStorage=0 );
		// Makes (or updates existing) attributes for each parameter. Also fills in the two maps below.
		// This method is called by getParameterised(), so you should call that before expecting the maps
		// to be up to date.
		MStatus createAttributesWalk( IECore::ConstCompoundParameterPtr parameter, const std::string &rootName, IECore::CompoundObjectPtr dynamicParameterStorage );
		typedef std::map<IECore::ParameterPtr, MString> ParameterToAttributeNameMap;
		ParameterToAttributeNameMap m_parametersToAttributeNames;
		typedef std::map<MString, IECore::ParameterPtr> AttributeNameToParameterMap;
		AttributeNameToParameterMap	m_attributeNamesToParameters;

		MStatus removeUnecessaryAttributes( IECore::CompoundObjectPtr dynamicParameterStorage );

		// We store the Parameters which were added dynamically inside a CompoundObject within a plug
		// value on the node. This allows us to recreate those Parameters when the node is copied or
		// saved and then loaded. The keys in the CompoundObject are the names of the parameters which
		// are parents of dynamic parameters. The values are ObjectVectors containing all the Parameters
		// which belong to that parent.
		IECore::CompoundObjectPtr getDynamicParameters();
		void setDynamicParameters( IECore::CompoundObjectPtr dynamicParameters );
		// This function takes the stored dynamic parameters and adds them to the currently held
		// Parameterised object. This means parameters can be added on the fly and they'll survive
		// node duplication and file save/load.
		void addDynamicParameters();

		class PLCB : public PostLoadCallback
		{
			public:
				PLCB( ParameterisedHolder<BaseType> *node );
			protected:

				ParameterisedHolder<BaseType> *m_node;

				void postLoad();
		};
		IE_CORE_DECLAREPTR( PLCB );

		PLCBPtr m_plcb;

	protected :

		IECore::RunTimeTypedPtr m_parameterised;
		bool m_failedToLoad; // to avoid constantly trying to reload things that aren't there

};

typedef ParameterisedHolder<MPxNode> ParameterisedHolderNode;
typedef ParameterisedHolder<MPxLocatorNode> ParameterisedHolderLocator;
typedef ParameterisedHolder<MPxDeformerNode> ParameterisedHolderDeformer;
typedef ParameterisedHolder<MPxFieldNode> ParameterisedHolderField;
typedef ParameterisedHolder<MPxObjectSet> ParameterisedHolderSet;
typedef ParameterisedHolder<MPxSurfaceShape> ParameterisedHolderSurfaceShape;
typedef ParameterisedHolder<MPxComponentShape> ParameterisedHolderComponentShape;
typedef ParameterisedHolder<MPxImagePlane> ParameterisedHolderImagePlane;

}

#endif // IE_COREMAYA_PARAMETERISEDHOLDERNODE_H
