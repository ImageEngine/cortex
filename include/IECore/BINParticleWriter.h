//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_BINPARTICLEWRITER_H
#define IE_CORE_BINPARTICLEWRITER_H

#include <fstream>

#include "IECore/ParticleWriter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// The BINParticleWriter class creates files in Realflow binary format.
class BINParticleWriter : public ParticleWriter
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( BINParticleWriter, ParticleWriter )

		BINParticleWriter( );
		BINParticleWriter( ObjectPtr object, const std::string &fileName );

		IntParameterPtr fluidTypeParameter();
		ConstIntParameterPtr fluidTypeParameter() const;

		IntParameterPtr frameNumberParameter();
		ConstIntParameterPtr frameNumberParameter() const;

		FloatParameterPtr radiusParameter();
		ConstFloatParameterPtr radiusParameter() const;

		FloatParameterPtr scaleSceneParameter();
		ConstFloatParameterPtr scaleSceneParameter() const;

		FloatParameterPtr elapsedSimulationTimeParameter();
		ConstFloatParameterPtr elapsedSimulationTimeParameter() const;

		IntParameterPtr frameRateParameter();
		ConstIntParameterPtr frameRateParameter() const;

		StringParameterPtr positionPrimVarParameter();
		ConstStringParameterPtr positionPrimVarParameter() const;

		StringParameterPtr velocityPrimVarParameter();
		ConstStringParameterPtr velocityPrimVarParameter() const;

		StringParameterPtr forcePrimVarParameter();
		ConstStringParameterPtr forcePrimVarParameter() const;

		StringParameterPtr vortisityPrimVarParameter();
		ConstStringParameterPtr vortisityPrimVarParameter() const;

		StringParameterPtr normalPrimVarParameter();
		ConstStringParameterPtr normalPrimVarParameter() const;

		StringParameterPtr numNeighboursPrimVarParameter();
		ConstStringParameterPtr numNeighboursPrimVarParameter() const;

		StringParameterPtr uvwPrimVarParameter();
		ConstStringParameterPtr uvwPrimVarParameter() const;

		StringParameterPtr agePrimVarParameter();
		ConstStringParameterPtr agePrimVarParameter() const;

		StringParameterPtr isolationTimePrimVarParameter();
		ConstStringParameterPtr isolationTimePrimVarParameter() const;

		StringParameterPtr viscosityPrimVarParameter();
		ConstStringParameterPtr viscosityPrimVarParameter() const;

		StringParameterPtr densityPrimVarParameter();
		ConstStringParameterPtr densityPrimVarParameter() const;

		StringParameterPtr pressurePrimVarParameter();
		ConstStringParameterPtr pressurePrimVarParameter() const;

		StringParameterPtr massPrimVarParameter();
		ConstStringParameterPtr massPrimVarParameter() const;

		StringParameterPtr temperaturePrimVarParameter();
		ConstStringParameterPtr temperaturePrimVarParameter() const;

		StringParameterPtr particleIdPrimVarParameter();
		ConstStringParameterPtr particleIdPrimVarParameter() const;

	protected :

		IntParameterPtr m_fluidTypeParameter;
		IntParameterPtr m_frameNumberParameter;
		FloatParameterPtr m_radiusParameter;
		FloatParameterPtr m_scaleSceneParameter;
		FloatParameterPtr m_elapsedSimulationTimeParameter;
		IntParameterPtr m_frameRateParameter;

		V3fParameterPtr m_emitterPositionParameter;
		V3fParameterPtr m_emitterRotationParameter;
		V3fParameterPtr m_emitterScaleParameter;

		StringParameterPtr m_positionPrimVarParameter;
		StringParameterPtr m_velocityPrimVarParameter;
		StringParameterPtr m_forcePrimVarParameter;
		StringParameterPtr m_vortisityPrimVarParameter;
		StringParameterPtr m_normalPrimVarParameter;
		StringParameterPtr m_numNeighboursPrimVarParameter;
		StringParameterPtr m_uvwPrimVarParameter;
		StringParameterPtr m_agePrimVarParameter;
		StringParameterPtr m_isolationTimePrimVarParameter;
		StringParameterPtr m_viscosityPrimVarParameter;
		StringParameterPtr m_densityPrimVarParameter;
		StringParameterPtr m_pressurePrimVarParameter;
		StringParameterPtr m_massPrimVarParameter;
		StringParameterPtr m_temperaturePrimVarParameter;
		StringParameterPtr m_particleIdPrimVarParameter;

		void getMaxMinAvg( ConstFloatVectorDataPtr data, float &mx, float &mn, float &avg ) const;

		template<typename T>
		typename T::ConstPtr getPrimVar( ConstStringParameterPtr parameter );

		template<typename T>
		void writeParticlePrimVar( std::ofstream &f, typename T::ConstPtr data, uint32_t i ) const;

	private :

		void constructParameters();

		virtual void doWrite();

		static const WriterDescription<BINParticleWriter> m_writerDescription;

};

IE_CORE_DECLAREPTR( BINParticleWriter );

} // namespace IECore

#endif // IE_CORE_BINPARTICLEWRITER_H
