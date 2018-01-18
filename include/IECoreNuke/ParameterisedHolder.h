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

#ifndef IECORENUKE_PARAMETERISEDHOLDER_H
#define IECORENUKE_PARAMETERISEDHOLDER_H

// Forward declaration of Nuke stuff

class Node; // Don't know why this isn't in a namespace
namespace DD
{
namespace Image
{
	class Op;
} // namespace Image
} // namespace DD

#include "IECore/Parameter.h"
#include "IECore/Parameterised.h"

#include "IECoreNuke/ParameterHandler.h"
#include "IECoreNuke/ObjectKnob.h"

namespace IECoreNuke
{

template<typename BaseType>
class IECORENUKE_API ParameterisedHolder : public BaseType
{

	public :

		ParameterisedHolder( Node *node );
		virtual ~ParameterisedHolder();

		//! @name Reimplementation of functions defined by the Nuke BaseType.
		/////////////////////////////////////////////////////////////////////
		//@{
		/// Uses a ParameterHandler to define the number of inputs.
		virtual int minimum_inputs() const;
		virtual int maximum_inputs() const;
		virtual bool test_input( int input, DD::Image::Op *op ) const;
		/// Calls knobs() on a ParameterHandler to represent the Parameters.
		virtual void knobs( DD::Image::Knob_Callback f );
		/// Implemented to load the Parameterised class.
		virtual int knob_changed( DD::Image::Knob *knob );
		/// Implemented to store the knob values into the held Parameters.
		virtual void _validate( bool forReal );
		//@}

		/// Returns the class instance held by this DD::Image::Op instance.
		/// Note that this is not a copy as with FnParameterisedHolder::getParameterised but is
		/// instead the internal class ready for use in processing in c++.
		/// If validate() has been called the parameter values will be up to date
		/// with respect to the knob values.
		IECore::ConstRunTimeTypedPtr parameterised();
		/// Convenience method to return dynamic_cast<const IECore::ParameterisedInterface *>( parameterised().get() )
		const IECore::ParameterisedInterface *parameterisedInterface();

	protected :

		void setParameterValuesFromInputs();
		void setParameterValues();
		void setKnobValues();

		/// Equivalent to DD::Image::Op::build_knob_handles(), but only processes
		/// knobs that represent parameters.
		void buildParameterKnobHandles( DD::Image::ViewerContext *ctx ) const;

	private :

		// class specification
		////////////////////////////////////////////////////
		ObjectKnob *m_classSpecifierKnob;
		DD::Image::Knob *m_versionChooserKnob; // for display of class name and user selection of version
		DD::Image::Knob *m_classReloadKnob; // for user to trigger reloading
		DD::Image::Knob *m_classDividerKnob;
		void updateVersionChooser();

		// class loading
		////////////////////////////////////////////////////
		IECore::RunTimeTypedPtr m_parameterised;
		IECore::ConstObjectPtr m_currentClassSpecification; // contents of m_classSpecifierKnob last time we updated
		// loads and returns an instance of the class specified by m_classSpecifierKnob.
		// this does not set m_parameterised.
		IECore::RunTimeTypedPtr loadClass( bool refreshLoader );
		// makes sure that m_parameterised is up to date with the class and state dictated by
		// m_classSpecifierKnob, and also makes sure that m_parameterHandler is valid.
		void updateParameterised( bool reload );

		// knob creation
		////////////////////////////////////////////////////
		ParameterHandlerPtr m_parameterHandler;
		size_t m_numParameterKnobs;
		void replaceKnobs();
		static void parameterKnobs( void *that, DD::Image::Knob_Callback f );

		// FnParameterisedHolder support
		////////////////////////////////////////////////////

		DD::Image::Knob *m_getParameterisedKnob; // this knob triggers a simulated getParameterised function.
		static IECore::RunTimeTypedPtr getParameterisedResult(); // and this function retrieves the result

		DD::Image::Knob *m_modifiedParametersKnob; // this knob triggers a simulated function call to do the work of FnParameterisedHolder.parameterModificationContext()
		static void setModifiedParametersInput( IECore::RunTimeTypedPtr parameterised ); // and this function specifies the parameter values to set the knobs from

		friend void bindFnParameterisedHolder();

};

typedef ParameterisedHolder<DD::Image::Op> ParameterisedHolderOp;

} // namespace IECoreNuke

#endif // IECORENUKE_PARAMETERISEDHOLDER_H
