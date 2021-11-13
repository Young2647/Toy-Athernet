#include<iostream>
#include<string>

#include "ArrayBlockingQueue.h"
#include "ArrayBlockingQueueImpl.h"

using namespace std;

// We first implement the constructor which takes capacity, and fairness mode.
template<typename T> ArrayBlockingQueue<T>::ArrayBlockingQueue(const size_t& capacity, const bool& fair) : m_capacity(capacity), m_fair(fair), m_frontIdx(-1), m_rearIdx(-1), m_size(0)
{
	if(capacity<1)
		throw IllegalArgumentException();
	m_queue = new T[capacity];
	m_name = getName();
}

// We next implement the constructor which takes capacity, fairness mode and a vector from which the ArrayBlockingQueue is initialized.
template<typename T> ArrayBlockingQueue<T>::ArrayBlockingQueue(const size_t& capacity, const bool& fair, const vector<T>& inputCollection) : m_capacity(capacity), m_fair(fair), 
	m_frontIdx(-1), m_rearIdx(-1), m_size(0)
{
	if(capacity<inputCollection.size() || capacity<1)
		throw IllegalArgumentException();

	m_queue = new T[capacity];
	for(const auto& iter : inputCollection)
		add(iter);
	m_name = getName();
}

// We need a few private internal implementations which manage the insertion and removal from the queue.
template<typename T> bool ArrayBlockingQueue<T>::isEmpty()
{
	// Internal call no lock needed. The method calling this takes a lock.
	return(m_size==0);
}

// We need to identify if the queue is full.
template<typename T> bool ArrayBlockingQueue<T>::isFull()
{
	// Internal call no lock needed. The method calling this takes a lock.
	return(m_size==m_capacity);
}

// Implement isFair policy.
template<typename T> bool ArrayBlockingQueue<T>::isFair()
{
	// Fairness never changes after construction hence it does not need any lock!
	return(m_fair);
}

// We implement the add method now.Inserts the specified element at the tail of this queue if it is possible to do so immediately without exceeding the queue's capacity, 
// returning true upon success and throwing an IllegalStateException if this queue is full.
template<typename T> bool ArrayBlockingQueue<T>::add(const T& item)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	if(isFull())
		throw IllegalStateException();
	bool returnStatus=enqueue(item);
	if(returnStatus)
	{	
		exclusiveLock.unlock();
		m_cond.notify_all();
	}
	return(returnStatus);
}

// We implement the reset method which simply resets the queue to empty status. No actual elements are removed , only front,rear and size are reset.
template<typename T> void ArrayBlockingQueue<T>::reset()
{
	// do not take any locks inside this function. The calling method should take one.
	m_frontIdx=m_rearIdx=-1;
	m_size=0;
}

// We now implement the clear method.Atomically removes all of the elements from this queue. 
template<typename T> void ArrayBlockingQueue<T>::clear()
{
	// We actually dont need to delete anything. simply reset the tracking flags.
	{
		unique_lock<shared_mutex> exclusiveLock(m_mutex);
		reset();
	}

	m_cond.notify_all();
}

// We need to implement the two critical internal methods enqueue and dequeue methods. They implement actual queue management.
template<typename T> bool ArrayBlockingQueue<T>::enqueue(const T& item)
{
	// All wrapper methods take a lock before calling this function. Please dont lock inside again here.
	if(isFull())
		return(false);

	if(m_frontIdx==-1)
		++m_frontIdx;

	++m_rearIdx;
	m_rearIdx=m_rearIdx%m_capacity;
	m_queue[m_rearIdx]=item;
	++m_size;
	return(true);
} 

// We need to implement the dequeue method now.
template<typename T> pair<bool,T> ArrayBlockingQueue<T>::dequeue()
{
	// All wrapper methods take a lock before calling this function. Please dont lock inside again here.
	T returnItem;
	if(isEmpty())
		return(make_pair(false, T()));

	if(m_frontIdx == m_rearIdx)
	{
		returnItem = m_queue[m_frontIdx];
		m_frontIdx=m_rearIdx=-1;
	}
	else
	{	
		returnItem = m_queue[m_frontIdx];
		m_frontIdx = (m_frontIdx + 1) % m_capacity;
	}
	--m_size;
	return(make_pair(true, returnItem));
}

// Implement simple method to get semaphore name. We dont want to keep deriving this repetative calls of toString().
template<typename T> string ArrayBlockingQueue<T>::getName()
{
	const void * address = static_cast<const void*>(this);
	stringstream stream;
	stream << address;
	string result = stream.str();
	return(result);
}

// Implementation of getThreadId Method. Returns a string of thread-id number.
template<typename T> string ArrayBlockingQueue<T>::getThreadId()
{
	auto myid = this_thread::get_id();
	stringstream ss;
	ss << myid;
	string resultString = ss.str();
	return(resultString);
}

// We need to implement the contains method. Returns true if this queue contains the specified element.
template<typename T> bool ArrayBlockingQueue<T>::contains(const T& item)
{
	shared_lock<shared_mutex> readLock(m_mutex);
	if(isEmpty())
		return(false);

	bool returnStatus=false;
	if(m_rearIdx>=m_frontIdx)
	{	
		int currentIdx=m_frontIdx;
		while(currentIdx<=m_rearIdx)
		{
			if(m_queue[currentIdx] == item)
				return(true);
			++currentIdx;
		}	
	}	
	else
	{
		int currentIdx=m_frontIdx;
		while(currentIdx<=static_cast<long int>((m_capacity-1)))
		{
			if(m_queue[currentIdx] == item)
				return(true);
			++currentIdx;
		}
		currentIdx=0;
		while(currentIdx != m_rearIdx)
		{
			if(m_queue[currentIdx] == item)
				return(true);
			++currentIdx;
		}
	}	
	return(returnStatus);
}

// We need to implement the drainTo method. Removes all available elements from this queue and adds them to the given collection.
template<typename T> size_t ArrayBlockingQueue<T>::drainToInternal(vector<T>& target, const long& size)
{
	size_t returnCount=0;
	if(size == -1)
	{
		while(!isEmpty())
		{
			pair<bool, T> returnItem = dequeue();
			if(returnItem.first)
			{
				target.emplace_back(returnItem.second);
				++returnCount;
			}
		}	
	}
	else
	{
		while(!isEmpty() && static_cast<long>(returnCount)!=size)
		{
			pair<bool, T> returnItem = dequeue();
			if(returnItem.first)
			{
				target.emplace_back(returnItem.second);
				++returnCount;
			}	
		}
	}
	return(returnCount);
}

// Implement the drainTo method as wrapper around above method.
template<typename T> size_t ArrayBlockingQueue<T>::drainTo(vector<T>& target)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	size_t result=drainToInternal(target);
	exclusiveLock.unlock();
	m_cond.notify_all();
	return(result);
}

// Implement the drainTo method with a given size.
template<typename T> size_t ArrayBlockingQueue<T>::drainTo(vector<T>& target, const long& size)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	if(size<1)
		throw IllegalStateException();
	size_t result=drainToInternal(target, size);
	exclusiveLock.unlock();
	m_cond.notify_all();
	return(result);
}

// Implement the iterator method. This returns an instance of Iterator<T> which allows one to access elements of ArrayBlockingQueue in read only mode.
// It has hasNext() and getNext() methods. 

template<typename T> Iterator<T> ArrayBlockingQueue<T>::iterator()
{
	const Iterator<T> iterator(&m_queue[0], m_frontIdx, m_rearIdx, m_size, m_capacity);
	return(iterator);
}

// Implement the method offer. Inserts the specified element at the tail of this queue if it is possible to do so immediately without exceeding the queue's capacity, 
// returning true upon success and false if this queue is full.
template<typename T> bool ArrayBlockingQueue<T>::offer(const T& item)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	bool returnStatus = enqueue(item);
	if(returnStatus)
	{
		exclusiveLock.unlock();
		m_cond.notify_all();
	}
	return(returnStatus);	
}

// Implement the offer method with delay parameters.Inserts the specified element at the tail of this queue, waiting up to 
// the specified wait time for space to become available if the queue is full.
template<typename T> bool ArrayBlockingQueue<T>::offer(const T& item, const long& waitQuantity, const TimeUnit& timeUnit)
{
	bool returnStatus=false;
	auto duration = TimeUtils::waitDuration(waitQuantity, timeUnit).count();
	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = startTime;
	auto durationCount = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();	
	while(durationCount<duration)
	{
		returnStatus=offer(item);
		if(returnStatus)
		{
			m_cond.notify_all();
			return(true);
		}
		endTime=chrono::high_resolution_clock::now();
		durationCount = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
	}
	return(returnStatus);
}

// We implement the peek method. Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
template<typename T> pair<bool,T> ArrayBlockingQueue<T>::peek()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	pair<bool, T> returnItem=make_pair(false, T());
	if(isEmpty())
		return(returnItem);
	else
		return(make_pair(true, m_queue[m_frontIdx]));
}

// We next implement the poll method. Retrieves and removes the head of this queue, or returns null if this queue is empty.
template<typename T> pair<bool,T> ArrayBlockingQueue<T>::poll()
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	pair<bool,T> returnItem=make_pair(false, T());
	if(isEmpty())
		return(returnItem);
	else
		returnItem = dequeue();
	exclusiveLock.unlock();
	m_cond.notify_all();
	return(returnItem);
}

// We next implement the poll with timeout Method. Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element to become available.
template<typename T> pair<bool,T> ArrayBlockingQueue<T>::poll(const long& waitQuantity, const TimeUnit& timeUnit)
{
	pair<bool,T> returnItem=make_pair(false, T());
	auto duration = TimeUtils::waitDuration(waitQuantity, timeUnit).count();
	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = startTime;
	auto durationCount = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
	while(durationCount<duration)
	{
		unique_lock<shared_mutex> exclusiveLock(m_mutex, defer_lock);
		if(exclusiveLock.try_lock())
		{
			if(!isEmpty())
			{
				returnItem=dequeue();
				return(returnItem);
			}
			else
				exclusiveLock.unlock(); // Else we have a possibility of same thread trying to Lock the mutex multiple times.
		}
		endTime=chrono::high_resolution_clock::now();
		durationCount = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
	}
	return(returnItem);
}

// We next implement the put method.Inserts the specified element at the tail of this queue, waiting for space to become available if the queue is full.
template<typename T> void ArrayBlockingQueue<T>::put(const T& item)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	string threadId = getThreadId();
	if(isFull())
	{
		if(isFair())
		{
			m_putq.push_back(threadId);
			m_cond.wait(exclusiveLock, [&]() { return( (m_capacity-m_size)>0 && *m_putq.begin()==threadId); }); 
			m_putq.pop_front();
		}
		else
			m_cond.wait(exclusiveLock, [&]() { return((m_capacity-m_size)>0); });
	}
	bool returnStatus=enqueue(item);
	if(returnStatus)
	{
		exclusiveLock.unlock();
		m_cond.notify_all();
	}
}

// We next implement the remainingCapacity method.Returns the number of additional elements that this queue can ideally (in the absence of memory or resource constraints) accept without blocking.
template<typename T> size_t ArrayBlockingQueue<T>::remainingCapacity()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	return(m_capacity-m_size);
}

//Removes a single instance of the specified element from this queue, if it is present. More formally, removes an element e such that o.equals(e), 
//if this queue contains one or more such elements. Returns true if this queue contained the specified element (or equivalently, if this queue changed as a result of the call).
//Removal of interior elements in circular array based queues is an intrinsically slow and disruptive operation, so should be undertaken only in exceptional circumstances, 
//ideally only when the queue is known not to be accessible by other threads.

template<typename T> bool ArrayBlockingQueue<T>::remove(const T& item)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	// remove one instance of the item present in the queue. i.e the first occurance if any found. 
	// return true is modification was done else false.

	// case 1 : empty queue. do nothing.
	if(m_size==0)
		return(false);

	// case 2 : one item and is same as the item to be removed. simply reset the queue and return true.
	if(m_size==1&&m_queue[m_frontIdx]==item)
	{
		reset();
		exclusiveLock.unlock();
		m_cond.notify_all();
		return(true);
	}

	// case 3 : The queue has not wrapped around end , i.e rearIdx>=frontIdx.
	if(m_rearIdx>m_frontIdx)
	{
		int currentIdx=m_frontIdx;
		int itemPos=-1;
		while(currentIdx<=m_rearIdx)
		{				
			if(m_queue[currentIdx]==item)
			{
				itemPos=currentIdx;
				break;
			}
			++currentIdx;
		}

		if(itemPos!=-1)
		{
			currentIdx=itemPos;
			while(currentIdx<m_rearIdx)
			{
				m_queue[currentIdx]=m_queue[currentIdx+1];
				++currentIdx;
			}
			--m_rearIdx;
			--m_size;
			exclusiveLock.unlock();
			m_cond.notify_all();
			return(true);
		}		
	}

	// case 4 : The queue has wrapped around the end , ie rearIdx<frontIdx. This one is a bit laborious.
	if(m_rearIdx<m_frontIdx)
	{
		int currentIdx=m_frontIdx;
		int itemPos=-1;
		while(currentIdx<=static_cast<long int>(m_capacity-1))
		{
			if(m_queue[currentIdx]==item)
			{
				itemPos=currentIdx;
				break;
			}
			++currentIdx;
		}

		if(itemPos!=-1)
		{
			currentIdx=itemPos;
			while(currentIdx<static_cast<long int>(m_capacity-1))
			{
				m_queue[currentIdx]=m_queue[currentIdx+1];
				++currentIdx;
			}
		}

		if(itemPos != -1)
		{
			// We need to shift all our elements wrapped around by 1 position from end onwards.
			currentIdx=m_capacity-1;
			while(currentIdx!=m_rearIdx)
			{	
				m_queue[currentIdx]=m_queue[(currentIdx+1)%m_capacity];
				++currentIdx;
				currentIdx=currentIdx%m_capacity;
			}
			--m_rearIdx;
			if(m_rearIdx==-1)
				m_rearIdx=m_capacity-1;
			--m_size;
			exclusiveLock.unlock();
			m_cond.notify_all();
			return(true);
		}

		// There is a case where item to be removed is on the wrapped up side i.e between index 0 and rearIdx.
		currentIdx=0;
		itemPos=-1;
		while(currentIdx<=m_rearIdx)
		{	
			if(m_queue[currentIdx]==item)
			{
				itemPos=currentIdx;
				break;
			}
			++currentIdx;
		}

		// We have found the element and needs a removal.
		if(itemPos!=-1)
		{
			currentIdx=itemPos;
			while(currentIdx<m_rearIdx)
			{
				m_queue[currentIdx]=m_queue[currentIdx+1];
				++currentIdx;
			}
			--m_rearIdx;
			if(m_rearIdx==-1)
				m_rearIdx=m_capacity-1;
			--m_size;
			exclusiveLock.unlock();
			m_cond.notify_all();
			return(true);
		}
	}
	return(false);
}

// We now implement the removeall method. This is not in Java standard but adding here for sake of completion. removall has compexity of O(n). It is in place with out extra space.
// space complexity is O(1). This must be used sparigly since removing elements other than front in queue is not frequent operation and must be realized that it can have 
// performance penalty if used frequently.

template<typename T> bool ArrayBlockingQueue<T>::removeall(const T& item)
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	// case 1 : empty queue. do nothing.
	if(m_size==0)
		return(false);

	// case 2 : one item and is same as the item to be removed. simply reset the queue and return true.
	if(m_size==1 && m_queue[m_frontIdx]==item)
	{
		reset();
		exclusiveLock.unlock();
		m_cond.notify_all();
		return(true);
	}

	// case 3 : The queue has not wrapped around end , i.e rearIdx>=frontIdx.
	if(m_rearIdx>m_frontIdx)
	{
		int j=m_frontIdx;
		int skipCount=0;
		for(int i=m_frontIdx; i<=m_rearIdx; ++i)
		{
			if(m_queue[i]!=item)
			{
				m_queue[j]=m_queue[i];
				++j;

			}
			else
				++skipCount;		
		}
		--j;
		m_rearIdx=j;
		m_size-=skipCount;
		if(m_size==0)
			reset();

		if(skipCount)
		{
			exclusiveLock.unlock();
			m_cond.notify_all();
			return(true);		
		}
	}

	// case 4 : The queue is wrapped around. This can be more tricky and complicated to implement.
	if(m_rearIdx<m_frontIdx)
	{
		int j=m_frontIdx;
		int skipCount=0;

		// Front frontIdx to capacity-1.
		for(int i=m_frontIdx; i<static_cast<long int>(m_capacity);++i)
		{
			if(m_queue[i]!=item)
			{
				m_queue[j]=m_queue[i];
				++j;
				j=j%m_capacity;
			}
			else
				++skipCount;
		}

		// From index 0 to rearIdx.
		for(int i=0; i<=m_rearIdx; ++i)
		{
			if(m_queue[i]!=item)
			{
				m_queue[j]=m_queue[i];
				++j;
				j=j%m_capacity;
			}
			else
				++skipCount;
		}
		--j;
		if(j==-1)
			j=m_capacity-1;
		m_rearIdx=j;
		m_size-=skipCount;
		if(m_size==0)
			reset();

		if(skipCount)
		{
			exclusiveLock.unlock();
			m_cond.notify_all();
			return(true);
		}
	}
	return(false);
}

// We implement the size method. it returns the current size of the queue.
template<typename T> size_t ArrayBlockingQueue<T>::size()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	return(m_size);
}

// We implement the method take. Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
template<typename T> T ArrayBlockingQueue<T>::take()
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	string threadId = getThreadId();
	if(isEmpty())
	{
		if(isFair())
		{
			m_takeq.push_back(threadId);
			m_cond.wait(exclusiveLock, [&]() { return((m_size>0)&&(*m_takeq.begin()==threadId)); }); 	
			m_takeq.pop_front();
		}			
		else
			m_cond.wait(exclusiveLock, [&]() { return(m_size>0); });
	}
	pair<bool,T> item = dequeue();
	exclusiveLock.unlock();
	m_cond.notify_all();
	return(item.second);
}

// We implement the method toArray now.Returns an array containing all of the elements in this queue, in proper sequence; the runtime type of the returned array is that of the specified array.
template<typename T> vector<T> ArrayBlockingQueue<T>::toArray()
{
	unique_lock<shared_mutex> exclusiveLock(m_mutex);
	vector<T> returnVec;
	if(isEmpty())
		return(returnVec);
	else
	{
		returnVec.reserve(m_size);
		if(m_frontIdx<=m_rearIdx)
		{
			int currentIdx=m_frontIdx;
			while(currentIdx<=m_rearIdx)
			{
				returnVec.emplace_back(m_queue[currentIdx]);
				++currentIdx;
			}
		}
		else
		{
			int currentIdx=m_frontIdx;
			while(currentIdx<static_cast<long int>((m_capacity-1)))
			{
				returnVec.emplace_back(m_queue[currentIdx]);
				++currentIdx;
			}
			currentIdx=0;
			while(currentIdx<=m_rearIdx)
			{
				returnVec.emplace_back(m_queue[currentIdx]);
				++currentIdx;
			}
		}
	}
	return(returnVec);
}

// We implement the final method toString. Returns a string representation of this collection with various status details. This is helpful for debugging purpose.
template<typename T> string ArrayBlockingQueue<T>::toString()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	string returnString = " ==> [ ArrayBlockingQueue Name = ArrayBlockingQueue." + m_name;
	returnString += ", capacity = " + to_string(m_capacity);
	returnString += ", size = " + to_string(m_size);
	returnString += ", remainingCapacity = " + to_string((m_capacity-m_size));
	returnString += ", frontIdx = " + to_string(m_frontIdx);
	returnString += ", rearIdx = " + to_string(m_rearIdx);
	returnString += ", fair = " + to_string(m_fair);
	returnString += ". ]";
	return(returnString);	
}

// We implement the internal function that iterates and displays the queued threads.
template<typename T> void ArrayBlockingQueue<T>::displayQueueThreads(const queueType& qType)
{
	if(qType==queueType::putQ)
	{
		for(const auto& iter : m_putq)
			cout << "ThreadId = " << iter << '\n';
	}
	if(qType==queueType::takeQ)
	{
		for(const auto& iter : m_takeq)
			cout << "ThreadId = " << iter << '\n';
	}
}

// Implement the displayPutQThreads.
template<typename T> void ArrayBlockingQueue<T>::displayPutQThreads()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	displayQueueThreads(queueType::putQ);
}

// Implement the displayTakeQThreads.
template<typename T> void ArrayBlockingQueue<T>::displayTakeQThreads()
{
	shared_lock<shared_mutex> readLock(m_mutex);
	displayQueueThreads(queueType::takeQ);
}

// Implement the destructor that will clean the queue/array.
template<typename T> ArrayBlockingQueue<T>::~ArrayBlockingQueue()
{
	delete[] m_queue;
}

