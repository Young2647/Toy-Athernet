#include<iostream>
#include<string>

#ifndef Iterator_H
#define Iterator_H

template<typename T> class Iterator
{
	private:
		const T* m_data;
		int m_idx = -1;
		const long int m_frontIdx;
		const long int m_rearIdx;
		const size_t   m_size;
		const size_t   m_capacity;

	public:
		Iterator(const T* startIdx, const size_t& frontIdx, const size_t& rearIdx, const size_t& size, const size_t& capacity): 
			m_data(startIdx), m_frontIdx(frontIdx), m_rearIdx(rearIdx), m_size(size), m_capacity(capacity)
	{
		// Nothing else needs to be done here.
	}
		bool hasNext()
		{

			// case 1 : empty queue.
			if(m_size==0)
				return(false);

			// case 2 : rearIdx is greater than frontIdx [ not wrapped around yet ].
			if(m_rearIdx>=m_frontIdx)
			{
				if(m_idx==-1)
				{
					m_idx=m_frontIdx;
					return(true);
				}
				else
				{
					if(m_idx+1<=m_rearIdx)
					{
						++m_idx;
						return(true);
					}
					else
					{
						reset();
						return(false);
					}
				}	
			}
			// case 3 : rearIdx is less than frontIdx [ wrapped around queue ].
			if(m_rearIdx < m_frontIdx)
			{
				if(m_idx==-1)
				{
					m_idx=m_frontIdx;
					return(true);
				}
				else
				{		
					if(m_idx>=m_frontIdx)					
					{
						if(m_idx+1<static_cast<long int>(m_capacity))
						{
							++m_idx;
							return(true);
						}	
						else
						{
							++m_idx;
							m_idx=m_idx%m_capacity;
							return(true);
						}
					}
					if(m_idx>=0 && m_idx<=m_rearIdx)
					{
						if(m_idx+1<=m_rearIdx)
						{
							++m_idx;
							return(true);
						}
						else
							reset();
						return(false);
					}
				}
			}		
			return(false);
		}

		T getNext()
		{
			return(m_data[m_idx]);
		}

		void reset()
		{
			m_idx=-1;
		}
};

#endif
