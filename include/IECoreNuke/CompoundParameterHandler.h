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

#ifndef IECORENUKE_COMPOUNDPARAMETERHANDLER_H
#define IECORENUKE_COMPOUNDPARAMETERHANDLER_H

#include "IECoreNuke/ParameterHandler.h"

namespace IECoreNuke
{

class IECORENUKE_API CompoundParameterHandler : public ParameterHandler
{

	public :

		CompoundParameterHandler();

		virtual int minimumInputs( const IECore::Parameter *parameter );
		virtual int maximumInputs( const IECore::Parameter *parameter );
		virtual bool testInput( const IECore::Parameter *parameter, int input, const DD::Image::Op *op );
		virtual void setParameterValue( IECore::Parameter *parameter, InputIterator first, InputIterator last );

		virtual void knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f );
		virtual void setParameterValue( IECore::Parameter *parameter, ValueSource valueSource = Storage );
		virtual void setKnobValue( const IECore::Parameter *parameter );
		virtual void setState( IECore::Parameter *parameter, const IECore::Object *state );
		virtual IECore::ObjectPtr getState( const IECore::Parameter *parameter );

	protected :

		void beginGroup( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f );
		void childKnobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f );
		void endGroup( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f );
		std::string knobLabel( const IECore::Parameter *parameter ) const;

	private :

		enum ContainerType
		{
			Collapsible,
			Tab,
			Toolbar
		};
		ContainerType containerType( const IECore::Parameter *parameter );

		void inputs( const IECore::Parameter *parameter, int &minimum, int &maximum, bool &error );

		ParameterHandlerPtr handler( const IECore::Parameter *child, bool createIfMissing );
		typedef std::map<IECore::InternedString, ParameterHandlerPtr> HandlerMap;
		HandlerMap m_handlers;

		static Description<CompoundParameterHandler> g_description;

};

} // namespace IECoreNuke

#endif // IECORENUKE_COMPOUNDPARAMETERHANDLER_H
