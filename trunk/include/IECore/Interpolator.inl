//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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



template<typename T>
void LinearInterpolator<T>::operator()(const T &y0, const T & y1, double x, T &result) const
{
	assert(x >= 0.0);
	assert(x <= 1.0);	
	
	result = static_cast<T>(y0 + (y1 - y0) * x);
}

// Partially specialise for std::vector
template<typename T>
struct LinearInterpolator< std::vector<T> >
{
	void operator()(const std::vector<T> &y0, 
			const std::vector<T> &y1,
			double x, 
			std::vector<T> &result) const
	{		
		unsigned size =  y0.size();
		assert(y1.size() == size);
		
		result.resize( size );
		
		LinearInterpolator<T> interp;
		for (unsigned i = 0; i < size; i++)
		{
			interp( y0[i], y1[i], x, result[i]);
		}
		
		assert(result.size() == size);		
	}
};

// Partially specialise for TypedData
template<typename T>
struct LinearInterpolator< TypedData< T > >
{
	void operator()(const boost::intrusive_ptr< TypedData< T > > &y0, 
			const boost::intrusive_ptr< TypedData< T > > &y1,
			double x, 
			boost::intrusive_ptr< TypedData< T > > &result) const
	{		
		LinearInterpolator<T>()( y0->readable(), y1->readable(), x, result->writable());
	}
};

template<typename T>
void CosineInterpolator<T>::operator()(const T &y0, const T &y1, double x, T &result) const
{
	assert(x >= 0.0);
	assert(x <= 1.0);	
	
	double cx = (1.0 - cos(x * M_PI)) / 2.0;
	result = y0 * (1 - cx) + y1 * cx;
}

// Partially specialise for std::vector
template<typename T>
struct CosineInterpolator< std::vector<T> >
{
	void operator()(const std::vector<T> &y0, 
			const std::vector<T> &y1,
			double x, 
			std::vector<T> &result) const
	{		
		unsigned size =  y0.size();
		assert(y1.size() == size);
		
		result.resize( size );
		
		CosineInterpolator<T> interp;
		for (unsigned i = 0; i < size; i++)
		{
			interp( y0[i], y1[i], x, result[i]);
		}
		
		assert(result.size() == size);
	}
};

// Partially specialise for TypedData
template<typename T>
struct CosineInterpolator< TypedData< T > >
{
	void operator()(const boost::intrusive_ptr< TypedData< T > > &y0, 
			const boost::intrusive_ptr< TypedData< T > > &y1,
			double x, 
			boost::intrusive_ptr< TypedData< T > > &result) const
	{		
		CosineInterpolator<T>()( y0->readable(), y1->readable(), x, result->writable());
	}
};

template<typename T>
void CubicInterpolator<T>::operator()(const T &y0, const T &y1, const T &y2, const T &y3, double x, T &result ) const
{
	assert(x >= 0.0);
	assert(x <= 1.0);	

	T a0 = y3 - y2 - y0 + y1;
	T a1 = y0 - y1 - a0;
	T a2 = y2 - y0;
	T a3 = y1;

	result = static_cast<T>(a0*x*x*x + a1*x*x + a2*x + a3);
}

// Partially specialise for std::vector
template<typename T>
struct CubicInterpolator< std::vector<T> >
{
	void operator()(const std::vector<T> &y0, 
			const std::vector<T> &y1,
			const std::vector<T> &y2,
			const std::vector<T> &y3,
			
			double x, 
			std::vector<T> &result) const
	{		
		unsigned size =  y0.size();
		assert(y1.size() == size);
		assert(y2.size() == size);
		assert(y3.size() == size);				
		
		result.resize( size );
		
		CubicInterpolator<T> interp;
		for (unsigned i = 0; i < size; i++)
		{
			interp( y0[i], y1[i], y2[i], y3[i], x, result[i]);
		}
		
		assert(result.size() == size);
	}
};

// Partially specialise for TypedData
template<typename T>
struct CubicInterpolator< TypedData<T > >
{
	void operator()(const boost::intrusive_ptr< TypedData< T > > &y0, 
			const boost::intrusive_ptr< TypedData< T > > &y1,
			const boost::intrusive_ptr< TypedData< T > > &y2,
			const boost::intrusive_ptr< TypedData< T > > &y3,
			
			double x, 
			boost::intrusive_ptr< TypedData< T > > &result) const
	{		
		CubicInterpolator<T>()( y0->readable(), y1->readable(), y2->readable(), y3->readable(), x, result->writable());
	}
};
