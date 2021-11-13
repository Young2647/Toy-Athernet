#include<chrono>
#include<climits>
#include<condition_variable>
#include<deque>
#include<iostream>
#include<shared_mutex>
#include<sstream>
#include<string>
#include<utility>
#include<vector>

#include "TimeUtils.h"
#include "ArrayBlockingQueueExceptions.h"
#include "Iterator.h"

#ifndef ArrayBlockingQueue_H
#define ArrayBlockingQueue_H

enum class queueType { putQ, takeQ };

template<typename T> class ArrayBlockingQueue
{
	private:
		size_t m_capacity;                  // capacity is the size of the allocated array and it cant be changed after initialization.
        bool m_fair;                        // This tracks if the implementation is fair to waiting threads i.e FIFO implementation. reduces the performance.
		long int m_frontIdx;				// This tracks the front of the queue in the array. Initially when queue is created it is set to -1.
		long int m_rearIdx;					// This tracks the end of the queue in the array. Initially when the queue is created it is set to -1.
		size_t m_size;						// We track size in a variable.
		T* m_queue;                         // This is pointer pointing to the array allocated for this queue. We implement a circular buffer using this array.
		std::shared_mutex m_mutex;			// We need mutex to ensure thread safety since this class is ThreadSafe as per Java implementation.
		std::condition_variable_any m_cond;	// This is required for signalling purpose in the code when there are waiting threads either to enqueue/dequeue the elements.		
	
		std::deque<std::string> m_putq;		// This queue manages the put threads waiting for queue to have some space.
		std::deque<std::string> m_takeq;	// This queue manages the take threads waiting for the queue to have some space.
	
		bool isEmpty();						// returns true/false to indicate if the queue is empty.
		bool isFull();						// returns if the queue is full to capacity.
		bool isFair();						// check if the fairness policy is set.
		bool enqueue(const T&);				// Internal method to add item to the queue.
		void reset();						// reset the queue as if it has no elements.
		std::pair<bool,T> dequeue();		// Internal method to remove item from the queue.
		size_t drainToInternal(std::vector<T>&, const long& =-1);	// Internal private implementation to cover both public functions of drainTo.
		std::string getName();				// This returns the unique memory address as string for toString method.
		std::string getThreadId();			// This returns the threadId of the executing thread.
		std::string m_name;					// The unique name of the ArrayBlockingQueue derived from its memory location.		
		void displayQueueThreads(const queueType&);// Display the threads are are queued up.Internal implementation used by wrapper methods of put and take queues. 		
			
	public:
		ArrayBlockingQueue(const size_t&, const bool& = false);	// This creates an empty queue with given capacity and fairness mode [ default false ].
		ArrayBlockingQueue(const size_t&, const bool&, const std::vector<T>&);	// This creates queue with given capacity, fairness mode and items are populated from input vector collection.
		
		ArrayBlockingQueue(const ArrayBlockingQueue&)=delete;
		ArrayBlockingQueue& operator=(const ArrayBlockingQueue&)=delete;
	
		bool add(const T&);					// Inserts the specified element at the tail of this queue if it is possible to do so immediately 
											//without exceeding the queue's capacity, returning true upon success and throwing an IllegalStateException if this queue is full.
		void clear();						// Atomically removes all of the elements from this queue.
		bool contains(const T&);			// Returns true if this queue contains the specified element.
		size_t drainTo(std::vector<T>&);		// Removes all available elements from this queue and adds them to the given collection.
		size_t drainTo(std::vector<T>&, const long&);	// Removes given size elements or less [ what ever is available till given size ] and adds them into the collection and returns the items added.
		Iterator<T> iterator();				// Returns an iterator over the elements in this queue in proper sequence.
		bool offer(const T&);			// Inserts the specified element at the tail of this queue if it is possible to do so immediately without exceeding the queue's capacity, 
											// returning true upon success and false if this queue is full.
		bool offer(const T&, const long&, const TimeUnit&);	// Inserts the specified element at the tail of this queue, 
															// waiting up to the specified wait time for space to become available if the queue is full.
		std::pair<bool, T> peek();			// Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
		std::pair<bool, T> poll();			// Retrieves and removes the head of this queue, or returns null if this queue is empty.
		std::pair<bool,T> poll(const long&, const TimeUnit&);	// Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element to become available.
		void put(const T&);					// Inserts the specified element at the tail of this queue, waiting for space to become available if the queue is full.
		size_t remainingCapacity();			// Returns the number of additional elements that this queue can ideally (in the absence of memory or resource constraints) accept without blocking.
		bool remove(const T&);				// Removes a single instance of the specified element from this queue, if it is present.
		bool removeall(const T&);			// Remove all instances of the specificed element from this queue, if present. returns true is queue was modified else false.
		size_t size();						// Returns the number of elements in this queue.
		T take();							// Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
		std::vector<T> toArray();				// Returns an array containing all of the elements in this queue, in proper sequence.
		std::string toString();				// Returns a string representation of this collection.
		void displayPutQThreads();			// Get a list of threads waiting to put items into a queue.
		void displayTakeQThreads();			// Get a list of threads waiting to take items from a queue.
		~ArrayBlockingQueue();				// deallocate the array.
};

#endif

