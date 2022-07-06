#ifndef NX_FRAMEWORK_PXITERATOR
#define NX_FRAMEWORK_PXITERATOR

template<class T> 
class PxIterator
{
private:
	T* firstData;
	T* lastData;
	T* current;
public:
	PxIterator(T* firstData, T* lastData) 
		: firstData(firstData), lastData(lastData), current(firstData) 
	{
	}

	PxIterator<T>& operator++()
	{
		++current;
		return *this;
	}

	T* operator*()
	{
		return current;
	}

	T* begin() const
	{
		return firstData;
	}

	T* end() const
	{
		return lastData;
	}

	void skip(NxU32 steps)
	{
		current += steps;
		if (current>end())
			current = end();
	}

	T* operator*() const 
	{ 
		return current; 
	}

	void reset() 
	{ 
		current = firstData; 
	}

	NxU32 size() const 
	{ 
		return (NxU32)(lastData-firstData); 		
	}

	void copy(void* destination) const 
	{ 
		if(size()>0)
			memcpy(destination, firstData, sizeof(T)*size()); 
	}
};


template<class T> 
class PxBackwardIterator
{
private:
	T* firstData;
	T* lastData;
	T* current;
public:
	PxBackwardIterator(T* firstData, T* lastData) 
		: firstData(firstData), lastData(lastData), current(firstData) 
	{
	}

	PxBackwardIterator(const PxIterator<T>& fwd) 
		: firstData(fwd.end()-1), lastData(fwd.begin()-1), current(fwd.end()-1) 
	{
	}

	PxBackwardIterator<T>& operator++()
	{
		--current;
		return *this;
	}

	T* operator*()
	{
		return current;
	}

	T* first() const
	{
		return firstData;
	}

	T* end() const
	{
		return lastData;
	}

	T* operator*() const 
	{ 
		return current; 
	}

	void reset() 
	{ 
		current = firstData; 
	}

	NxU32 size() const 
	{ 
		return (NxU32)(firstData-lastData); 		
	}
};

#endif //NX_FRAMEWORK_PXITERATOR
