//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_STANDARDRADIALLENSMODEL_H
#define IECORE_STANDARDRADIALLENSMODEL_H

#include "IECore/Export.h"
#include "IECore/LensModel.h"

#include <map>
#include <string>

namespace IECore
{

/// StandardRadialLensModel
/// An implementation of a 2 degree Anamorphic or 4 degree Radial lens model.
/// This model and it's parameters produce identical results to that of 3DE.
/// It was implemented from a paper that is available on 3DE's website.
/// http://www.3dequalizer.com/user_daten/tech_docs/pdf/distortion.pdf
///
/// Further information on this algorithm and others can be found freely at:
/// Lens Distortion Plugin Kit v1.3 by Uwe Sassenberg, Science-D-Visions
/// http://www.3dequalizer.com/index.php#?site=tech_docs&id=110216_01

class IECORE_API StandardRadialLensModel : public LensModel
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( StandardRadialLensModel, LensModel );

		void validate() override;
		Imath::V2d distort( Imath::V2d p ) override;
		Imath::V2d undistort( Imath::V2d p ) override;

	protected:

		/// The Default Constructor is protected and the LensModel::create() method
		/// should be used to instantiate a new LensModel instead.
		StandardRadialLensModel();
		~StandardRadialLensModel() override;

	private:

		/// Transforms UV coordinates in the range 0-1
		/// to dimesionless coordinates which
		/// are used by the distortion algorithm.
		Imath::V2d UVtoDN( const Imath::V2d& uv );

		/// Transforms the dimesionless coordinates
		/// used by the distortion algorithm to UV
		/// coordinates of in the range 0-1.
		Imath::V2d DNtoUV( const Imath::V2d& uv );

		/// Coeficients needed by the distortion algorithm.
		/// These values are calculated within validate().
		double m_filmbackDiagonal;
		Imath::V2d m_dnFilmback, m_dnOffset;
		double m_cxx, m_cxy, m_cyx, m_cyy, m_cxxx;
		double m_cxxy, m_cxyy, m_cyxx, m_cyyx, m_cyyy;

		/// A static member that registers this LensModel so that it
		/// can be instantiated using the LensModel::create method.
		static LensModelRegistration<StandardRadialLensModel> m_registration;

		/// The LensModel baseclass needs to be a friend so that the creator can
		/// instantiate the default constructor which is protected.
		friend class LensModel;
};

IE_CORE_DECLAREPTR( StandardRadialLensModel );

} // namespace IECore

#endif // IECORE_STANDARDRADIALLENSMODEL_H

