//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include <string.h>

#include "IECore/RadixSort.h"

using namespace IECore;

RadixSort::RadixSort() : m_currentSize( 0 ), m_ranks( 0 ), m_ranks2( 0 )
{
	m_currentSize |= 0x80000000;
}

RadixSort::~RadixSort()
{
}

void RadixSort::resize( unsigned int s )
{
	m_ranks = new UIntVectorData();
	m_ranks->writable().resize( s );

	m_ranks2 = new UIntVectorData();
	m_ranks2->writable().resize( s );
}

template<typename T>
bool RadixSort::checkPassValidity( const std::vector<T> &input, unsigned int pass, unsigned int* &curCount, unsigned char &uniqueVal )
{
	curCount = &m_histogram[ pass << 8 ];

	uniqueVal = *((( unsigned char* )( &input[0] ) ) + pass );

	if ( curCount[ uniqueVal ] == input.size() )
	{
		return false;
	}

	return true;
}

void RadixSort::checkResize( unsigned int s )
{
	unsigned int curSize = ( m_currentSize & 0x7fffffff );

	if ( s != curSize )
	{
		resize( s );
		
		m_currentSize = s;
		m_currentSize |= 0x80000000;
	}
}

const std::vector<unsigned int> &RadixSort::operator()( const std::vector<float> &input2 )
{
	const unsigned int nb = input2.size();
	checkResize( nb );
	
	if ( !nb )
	{
		return m_ranks->readable();
	}

	const unsigned int *input = ( const unsigned int * ) &input2[0];
	
	bool alreadySorted = createHistograms< float >( input2 );
	if ( alreadySorted )
	{
		return m_ranks->readable();
	}

	unsigned int numNegativeValues = 0;

	const unsigned int *h3 = &m_histogram[ 768 ];
	for ( unsigned int i = 128; i < 256; i ++ )
	{
		numNegativeValues += h3[i];
	}

	for ( unsigned int j = 0; j < 4; j ++ )
	{
		if ( j != 3 )
		{
			unsigned char uniqueVal = 0;
			unsigned int *curCount = 0;
			bool performPass = checkPassValidity( input2, j, curCount, uniqueVal );

			if ( performPass )
			{
				m_link[0] = &m_ranks2->writable()[0];

				for ( unsigned int i = 1; i < 256; i++ )
				{
					m_link[i] = m_link[ i - 1 ] + curCount[ i - 1 ];
				}

				const unsigned char *inputBytes = ( const unsigned char * ) input;
				inputBytes += j;

				if ( m_currentSize & 0x80000000 )
				{
					for ( unsigned int i = 0; i < nb; i++ )
					{
						*m_link[ inputBytes[i<<2] ]++ = i;						
					}
					m_currentSize&=0x7fffffff;
				}
				else
				{
					for ( UIntVectorData::ValueType::const_iterator it = m_ranks->readable().begin(); it != m_ranks->readable().end(); ++it )
					{
						unsigned int id = *it;
						*m_link[ inputBytes[ id << 2 ] ]++ = id;
					}
				}

				std::swap( m_ranks, m_ranks2 );
			}
		}
		else
		{
			unsigned char uniqueVal = 0;
			unsigned int *curCount = 0;
			bool performPass = checkPassValidity( input2, j, curCount, uniqueVal );

			if ( performPass )
			{
				m_link[0] = &m_ranks2->writable()[numNegativeValues];
				for ( unsigned int i = 1; i < 128; i++ )
				{
					m_link[i] = m_link[ i - 1 ] + curCount[ i - 1 ];
				}

				m_link[255] = &m_ranks2->writable()[0];
				for ( unsigned int i = 0; i < 127; i ++ )
				{
					m_link[254 - i ] = m_link[255-i] + curCount[255 - i ];
				}

				for ( unsigned int i = 128; i < 256; i ++ )
				{
					m_link[i] += curCount[i];
				}

				if ( m_currentSize & 0x80000000 )
				{
					for ( unsigned int i = 0; i < nb; i++ )
					{
						unsigned int radix = input[i] >> 24;

						if ( radix < 128 )
						{
							*m_link[ radix ]++ = i;
						}
						else
						{
							*( --m_link[ radix] ) = i;
						}
					}
					m_currentSize&=0x7fffffff;
				}
				else
				{
					for ( unsigned int i = 0; i < nb; i ++ )
					{
						unsigned int radix = input[ m_ranks->readable()[i] ] >> 24;

						if ( radix < 128 )
						{
							*m_link[ radix ] ++ = m_ranks->readable()[i];
						}
						else
						{
							*( --m_link[ radix ] ) = m_ranks->readable()[i];
						}
					}
				}

				std::swap( m_ranks, m_ranks2 );
			}
			else
			{
				if ( uniqueVal >= 128 )
				{
					if ( m_currentSize & 0x80000000 )
					{
						for ( unsigned int i = 0; i < nb; i++ )
						{
							m_ranks2->writable()[i] = nb - i - 1;
						}
						m_currentSize&=0x7fffffff;
					}
					else
					{
						for ( unsigned int i = 0; i < nb; i++ )
						{
							m_ranks2->writable()[i] = m_ranks->readable()[ nb - i - 1 ];
						}
					}

					std::swap( m_ranks, m_ranks2 );
				}
			}
		}
	}

	return m_ranks->readable();
}

const std::vector<unsigned int> &RadixSort::operator()( const std::vector<unsigned int> &input2 )
{
	const unsigned int nb = input2.size();
	checkResize( nb );
	
	if ( !nb )
	{
		return m_ranks->readable();
	}
	const unsigned int *input = &input2[0];
	
	bool alreadySorted = createHistograms< unsigned int >( input2 );
	if ( alreadySorted )
	{
		return m_ranks->readable();
	}

	for ( unsigned int j = 0; j < 4; j ++ )
	{
		unsigned char uniqueVal = 0;
		unsigned int *curCount = 0;
		bool performPass = checkPassValidity( input2, j, curCount, uniqueVal );

		if ( performPass )
		{
			m_link[0] = &m_ranks2->writable()[0];
				
			for( unsigned int i = 1; i < 256; i++ )
			{
				m_link[i] = m_link[i - 1] + curCount[ i - 1 ];
			} 
			
			const unsigned char *inputBytes = ( const unsigned char * )( input );
			inputBytes += j;
			
			if (m_currentSize & 0x80000000)
			{
				for ( unsigned int i = 0; i < nb; i++ )
				{
					*m_link[ inputBytes[ i<<2 ] ]++ = i;
				}
				m_currentSize &= 0x7fffffff;
			}
			else
			{
				for ( UIntVectorData::ValueType::const_iterator it = m_ranks->readable().begin(); it != m_ranks->readable().end(); ++it )
				{
					unsigned int id = *it;
					*m_link[ inputBytes[ id << 2 ] ]++ = id;
				}	
			}
			
			std::swap( m_ranks, m_ranks2 );
		}		
	}

	return m_ranks->readable();
}

const std::vector<unsigned int> &RadixSort::operator()( const std::vector<int> &input2 )
{
	const unsigned int nb = input2.size();
	checkResize( nb );
	
	if ( !nb )
	{
		return m_ranks->readable();
	}
	const unsigned int *input = ( const unsigned int * ) &input2[0];
	
	bool alreadySorted = createHistograms< int >( input2 );
	if ( alreadySorted )
	{
		return m_ranks->readable();
	}
	
	unsigned int numNegativeValues = 0;
	unsigned int* h3= &m_histogram[768];	
	for ( unsigned int i = 128; i < 256; i++ )
	{
		numNegativeValues += h3[i];		
	}

	for ( unsigned int j = 0; j < 4; j ++ )
	{
		unsigned char uniqueVal = 0;
		unsigned int *curCount = 0;
		bool performPass = checkPassValidity( input2, j, curCount, uniqueVal );

		if ( performPass )
		{
			if ( j != 3 )
			{
				m_link[0] = &m_ranks2->writable()[0];

				for( unsigned int i = 1; i < 256; i++ )
				{
					m_link[i] = m_link[i - 1] + curCount[ i - 1 ];
				} 
			}
			else
			{
				m_link[0] = &m_ranks2->writable()[numNegativeValues];
				for( unsigned int i = 1; i < 128; i++ )
				{
					m_link[i] = m_link[i - 1] + curCount[ i - 1 ];
				} 
				m_link[128] = &m_ranks2->writable()[0];
				for( unsigned int i = 129; i < 256; i++ )
				{
					m_link[i] = m_link[i - 1] + curCount[ i - 1 ];
				} 
			}
			
			const unsigned char *inputBytes = ( const unsigned char * )( input );
			inputBytes += j;
			
			if (m_currentSize & 0x80000000)
			{
				for ( unsigned int i = 0; i < nb; i++ )
				{
					*m_link[ inputBytes[ i<<2 ] ]++ = i;
				}
				m_currentSize &= 0x7fffffff;
			}
			else
			{
				for ( UIntVectorData::ValueType::const_iterator it = m_ranks->readable().begin(); it != m_ranks->readable().end(); ++it )
				{
					unsigned int id = *it;
					*m_link[ inputBytes[ id << 2 ] ]++ = id;
				}	
			}
			
			std::swap( m_ranks, m_ranks2 );
		}		
	}

	return m_ranks->readable();
}


template<typename T>
bool RadixSort::createHistograms( const std::vector<T> &input )
{
	BOOST_STATIC_ASSERT( sizeof(T) == 4 );
	
	memset( &m_histogram[0], 0, 256 * 4 * sizeof( unsigned int ) );

	const unsigned char *p = reinterpret_cast< const unsigned char * > ( &input[0] );
	const unsigned char *pEnd = &p[ input.size() * 4 ] ;

	unsigned int *h0 = &m_histogram[0];
	unsigned int *h1 = &m_histogram[256];
	unsigned int *h2 = &m_histogram[512];
	unsigned int *h3 = &m_histogram[768];

	bool alreadySorted = true;

	if ( m_currentSize & 0x80000000 )
	{
		const T *running = &input[0];
		T prevVal = *running;

		while ( p != pEnd )
		{
			T val = *running++;

			if ( val < prevVal )
			{
				alreadySorted = false;
				break;
			}

			prevVal = val;

			h0[ *p++ ] ++;
			h1[ *p++ ] ++;
			h2[ *p++ ] ++;
			h3[ *p++ ] ++;
		}

		if ( alreadySorted )
		{
			for ( unsigned int i = 0; i < input.size(); i ++ )
			{
				m_ranks->writable()[i] = i;
			}
			return true;
		}
	}
	else
	{
		const unsigned int* indices = &m_ranks->readable()[0];
		T prevVal = input[ *indices ];

		while ( p != pEnd )
		{
			T val = input[ *indices++ ];
			if ( val < prevVal )
			{
				alreadySorted = false;
				break;
			}

			prevVal = val;

			h0[ *p++ ] ++;
			h1[ *p++ ] ++;
			h2[ *p++ ] ++;
			h3[ *p++ ] ++;
		}

		if ( alreadySorted )
		{
			return true;
		}

	}

	while ( p != pEnd )
	{
		h0[ *p++ ] ++;
		h1[ *p++ ] ++;
		h2[ *p++ ] ++;
		h3[ *p++ ] ++;
	}

	return false;
}

