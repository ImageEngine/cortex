//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_BATCHPRIMITIVECONVERTER_H
#define IECOREAPPLESEED_BATCHPRIMITIVECONVERTER_H

#include "boost/filesystem/path.hpp"

#include "IECoreAppleseed/private/PrimitiveConverter.h"

namespace IECoreAppleseed
{

/// A PrimitiveConverter subclass that writes primitives to geometry files.
class BatchPrimitiveConverter : public PrimitiveConverter
{
	public :

		BatchPrimitiveConverter( const boost::filesystem::path &projectPath, const foundation::SearchPaths &searchPaths );

		void setOption( const std::string &name, IECore::ConstDataPtr value ) override;

	private :

		boost::filesystem::path m_projectPath;
		std::string m_meshGeomExtension;

		foundation::auto_release_ptr<renderer::Object> doConvertPrimitive( IECoreScene::PrimitivePtr primitive,
			const std::string &name ) override;

		foundation::auto_release_ptr<renderer::Object> doConvertPrimitive( const std::vector<IECoreScene::PrimitivePtr> &primitives,
			const std::string &name ) override;

		std::string objectEntityName( const std::string& objectName ) const override;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_BATCHPRIMITIVECONVERTER_H
