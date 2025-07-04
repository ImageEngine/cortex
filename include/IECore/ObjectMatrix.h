//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Cinesite VFX Ltd. All rights reserved.
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

#ifndef IECORE_OBJECTMATRIX_H
#define IECORE_OBJECTMATRIX_H

#include "IECore/Export.h"
#include "IECore/Object.h"

namespace IECore
{

/// An Object which holds a matrix of child Objects.
class IECORE_API ObjectMatrix : public Object
{

	public :

		ObjectMatrix( size_t rows = 0, size_t columns = 0 );
		~ObjectMatrix() override;

		IE_CORE_DECLAREOBJECT( ObjectMatrix, Object );

		size_t numRows() const;
		size_t numColumns() const;

		/// Resizes the matrix, preserving the original positions of its values.
		void resize( size_t rows, size_t columns );

		ObjectPtr *operator[]( size_t row )
		{
			return &m_members[ row * m_columns ];
		}

		const ObjectPtr *operator[]( size_t row ) const
		{
			return &m_members[ row * m_columns ];
		}

	private :

		using MemberContainer = std::vector<ObjectPtr>;

		MemberContainer m_members;
		size_t m_rows;
		size_t m_columns;

};

IE_CORE_DECLAREPTR( ObjectMatrix );

} // namespace IECore

#endif // IECORE_OBJECTMATRIX_H
