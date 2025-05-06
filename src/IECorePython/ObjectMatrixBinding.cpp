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

#include "boost/python.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECorePython/ObjectMatrixBinding.h"

#include "IECore/ObjectMatrix.h"

using namespace boost::python;
using namespace IECorePython;
using namespace IECore;

namespace
{

ObjectMatrixPtr constructFromSequence( object o )
{
	const size_t rows = boost::python::len( o );
	size_t columns = 0;
	for( size_t i = 0; i < rows; ++i )
	{
		object row = o[i];
		if( !PyList_Check( row.ptr() ) )
		{
			PyErr_SetString( PyExc_ValueError, "Each element must be a list" );
			throw_error_already_set();
		}
		size_t rowLength = boost::python::len( row );
		if( rowLength > columns )
		{
			columns = rowLength;
		}
	}

	ObjectMatrixPtr result = new ObjectMatrix( rows, columns );
	ObjectMatrix *m = result.get();
	for( size_t i = 0; i < rows; ++i )
	{
		for( size_t j = 0; j < columns; ++j )
		{
			if( j < (size_t)boost::python::len( o[i] ) )
			{
				(*m)[i][j] = extract<IECore::ObjectPtr>( o[i][j] );
			}
		}
	}

	return result;
}

std::string repr( const ObjectMatrix &m )
{
	std::stringstream s;

	s << "IECore.ObjectMatrix(";

	if( m.numRows() > 0 )
	{
		s << " [";

		for( size_t x = 0; x < m.numRows(); x++ )
		{
			s << " [";

			for( size_t y = 0; y < m.numColumns(); y++ )
			{
				object item( m[x][y] );
				std::string v = call_method< std::string >( item.ptr(), "__repr__" );
				s << " " << v << ( y == m.numColumns() -1 ? " " : "," );
			}

			s << "]" << ( x == m.numRows() - 1 ? " " : "," );
		}

		s << "] ";
	}

	s << ")";

	return s.str();
}

size_t convertRowIndex( const ObjectMatrix &m, tuple index )
{
	int64_t row = extract<int64_t>( index[0] );
	if( row < 0 )
	{
		row += m.numRows();
	}
	if( row >= (int64_t)m.numRows() || row < 0 )
	{
		PyErr_SetString( PyExc_IndexError, "Index out of range" );
		throw_error_already_set();
	}
	return row;
}

size_t convertColumnIndex( const ObjectMatrix &m, tuple index )
{
	int64_t column = extract<int64_t>( index[1] );
	if( column < 0 )
	{
		column += m.numColumns();
	}
	if( column >= (int64_t)m.numColumns() || column < 0 )
	{
		PyErr_SetString( PyExc_IndexError, "Index out of range" );
		throw_error_already_set();
	}

	return column;
}

IECore::ObjectPtr getItem( const ObjectMatrix &m, tuple index )
{
	return m[ convertRowIndex( m, index ) ][ convertColumnIndex( m, index ) ];
}

void setItem( ObjectMatrix &m, tuple index, IECore::ObjectPtr value )
{
	m[ convertRowIndex( m, index ) ][ convertColumnIndex( m, index ) ] = value;
}

} // namespace

void IECorePython::bindObjectMatrix()
{
	RunTimeTypedClass<ObjectMatrix>()
		.def( init< size_t, size_t >( ( arg( "rows" ) = 0, arg( "columns" ) = 0 ) ) )
		.def( "__init__", make_constructor( &constructFromSequence ) )
		.def( "__repr__", &repr )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		.def( "numRows", &ObjectMatrix::numRows )
		.def( "numColumns", &ObjectMatrix::numColumns )
		.def( "resize", &ObjectMatrix::resize )
	;
}
