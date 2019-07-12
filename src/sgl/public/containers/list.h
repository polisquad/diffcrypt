#pragma once

#include "core_types.h"
#include "hal/platform_memory.h"
#include "hal/malloc_ansi.h"
#include "templates/reference.h"
#include "templates/is_trivially_copyable.h"
#include "templates/const_ref.h"

/**
 * @struct SingleLink containers/list.h
 * 
 * A link with reference to next and prev elements
 * 
 * @param T data type
 */
template<typename T>
struct GCC_ALIGN(16) SingleLink
{
public:
	/// Next link
	SingleLink * next;

	/// Link data
	T data;

public:
	/// Default constructor
	template<typename _T = T>
	FORCE_INLINE SingleLink(_T && _data)
		: next{nullptr}
		, data{forward<_T>(_data)} {}
	
	/// Link with next
	FORCE_INLINE void linkNext(SingleLink * _next)
	{
		SingleLink * that = next;
		if (next = _next) next->next = that;
	}
};

template<typename T>
using SingleLinkRef = SingleLink<T>*;

/**
 * @struct DoubleLink containers/list.h
 * 
 * A link with reference to next and prev elements
 * 
 * @param T data type
 */
template<typename T>
struct GCC_ALIGN(16) DoubleLink
{
public:
	/// Next link
	DoubleLink * next;

	/// Previous link
	DoubleLink * prev;

	/// Link data
	T data;

public:
	/// Default constructor
	template<typename _T = T>
	FORCE_INLINE DoubleLink(_T && _data)
		: next(nullptr)
		, prev(nullptr)
		, data(forward<_T>(_data)) {}
	
	/**
	 * Link with next
	 * 
	 * @param [in] _next next link
	 * @return self
	 */
	FORCE_INLINE void linkNext(DoubleLink * _next)
	{
		DoubleLink * that = next;
		
		// Replace next link
		next = _next;

		if (next)
		{
			next->prev = this;
			next->next = that;
		}
	}

	/**
	 * Link with prev
	 * 
	 * @param [in] _prev previous link
	 * @return self
	 */
	FORCE_INLINE void linkPrev(DoubleLink * _prev)
	{
		DoubleLink * that = prev;
		
		// Replace next link
		prev = _prev;

		if (prev)
		{
			prev->next = this;
			prev->prev = that;
		}
	}
	
	/**
	 * Unlink from list
	 * 
	 * @return self
	 */
	FORCE_INLINE void unlink()
	{
		if (next)
			next->linkPrev(prev);
		else if (prev)
			prev->linkNext(next);
	}
};

template<typename T>
using DoubleLinkRef = DoubleLink<T>*;

/**
 * @struct LinkDataType containers/list.h
 * 
 * Sets type to the underlying data type
 * @{
 */
template<typename LinkT>
struct LinkDataType {};

template<typename T> struct LinkDataType<SingleLink<T>> { using Type = T; };
template<typename T> struct LinkDataType<DoubleLink<T>> { using Type = T; };
/// @}

//////////////////////////////////////////////////
// Queue
//////////////////////////////////////////////////

/**
 * @class Queue containers/list.h
 * 
 * A singly linked queue with FIFO behaviour
 * 
 * @param T data type
 * @param AllocT allocator type
 */
template<typename T, typename AllocT = MallocAnsi>
class Queue
{
public:
	/// Link types @{
	using Link		= ::SingleLink<T>;
	using LinkRef	= ::SingleLinkRef<T>;
	/// @}

protected:
	/// Allocator in use @{
	AllocT * allocator;
	bool bHasOwnAllocator;
	/// @}

	/// Queue head
	LinkRef head;

	/// Queue tail
	LinkRef tail;

	/// Num clients
	uint64 count;

public:
	/// Default constructor
	explicit FORCE_INLINE Queue(AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: allocator{_allocator}
		, bHasOwnAllocator{_allocator == nullptr}
		, head{nullptr}
		, tail{nullptr}
		, count{0}
	{
		if (bHasOwnAllocator)
			allocator = new AllocT;
	}

protected:
	/// Create new link with data
	template<typename _T = T>
	LinkRef createLink(_T && data)
	{
		return new (allocator->malloc(sizeof(Link))) Link(forward<_T>(data));	
	}

	/// Replicate queue structure
	void replicate(LinkRef _head)
	{
		if (_head)
			for (head = tail = createLink(_head->data); _head = _head->next; )
			{
				LinkRef link = createLink(_head->data);
				tail = tail->next = link;
			}
	}

public:
	/// Copy constructor
	FORCE_INLINE Queue(const Queue & other)
		: Queue()
	{
		replicate(other.head);
		count = other.count;
	}

	/// Move constructor
	FORCE_INLINE Queue(Queue && other)
		: allocator{other.allocator}
		, bHasOwnAllocator{other.bHasOwnAllocator}
		, head{other.head}
		, tail{other.tail}
		, count{other.count}
	{
		other.bHasOwnAllocator = false;
		other.head = nullptr;
	}

	/// Destroy and deallocates all clients
	FORCE_INLINE void empty()
	{
		LinkRef it = head;
		while (it)
		{
			LinkRef next = it->next;

			// Destroy link
			it->~Link();
			allocator->free(it);

			// Next link
			it = next;
		}
	}

	/// Destructor
	FORCE_INLINE ~Queue()
	{
		// Remove all clients
		empty();

		// Delete allocator
		if (bHasOwnAllocator) delete allocator;
	}

	/**
	 * Insert a new client at the end of the queue
	 * 
	 * @param [in] data new client
	 * @return inserted client
	 */
	template<typename _T = T>
	T & insert(_T && data)
	{
		LinkRef link = createLink(forward<_T>(data));

		++count;
		return tail ? (tail = tail->next = link)->data : (head = tail = link)->data;
	}

	/**
	 * Pop first client from queue
	 * 
	 * @param [out] data removed client data
	 * @return true if not empty
	 * @{
	 */
	bool pop()
	{
		if (LIKELY(head != nullptr))
		{
			LinkRef link = head;
			head = head->next;

			// Destroy link
			link->~Link();
			allocator->free(link);

			return true;
		}

		return false;
	}
	bool pop(T & data)
	{
		if (LIKELY(head != nullptr))
		{
			LinkRef link = head;
			head = head->next;

			// Move data
			data = move(link->data);

			// Destroy link
			link->~Link();
			allocator->free(link);

			return true;
		}

		return false;
	}
	/// @}
};

//////////////////////////////////////////////////
// Stack
//////////////////////////////////////////////////

/**
 * @class Stack containers/list.h
 * 
 * A singly linked stack with LIFO behaviour
 * 
 * @param T data type
 * @param AllocT allocator type
 */
template<typename T, typename AllocT = MallocAnsi>
class Stack
{
public:
	/// Link types @{
	using Link		= ::SingleLink<T>;
	using LinkRef	= ::SingleLinkRef<T>;
	/// @}

protected:
	/// Allocator in use @{
	AllocT * allocator;
	bool bHasOwnAllocator;
	/// @}

	/// Stack head
	LinkRef head;

	/// Num elements
	uint64 count;

public:
	/// Default constructor
	explicit FORCE_INLINE Stack(AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: allocator{_allocator}
		, bHasOwnAllocator{_allocator == nullptr}
		, head{nullptr}
		, count{0}
	{
		if (bHasOwnAllocator)
			allocator = new AllocT;
	}

protected:
	/// Create new link with data
	template<typename _T = T>
	LinkRef createLink(_T && data)
	{
		return new (allocator->malloc(sizeof(Link))) Link(forward<_T>(data));	
	}

	/// Replicate stack structure
	void replicate(LinkRef _head)
	{
		if (_head)
			for (LinkRef it = (head = createLink(_head->data)); _head = _head->next; )
			{
				LinkRef link = createLink(_head->data);
				it = it->next = link;
			}
	}

public:
	/// Copy constructor
	FORCE_INLINE Stack(const Stack & other)
		: Stack()
	{
		replicate(other.head);
		count = other.count;
	}

	/// Move constructor
	FORCE_INLINE Stack(Stack && other)
		: allocator{other.allocator}
		, bHasOwnAllocator{other.bHasOwnAllocator}
		, head{other.head}
		, count{other.count}
	{
		other.bHasOwnAllocator = false;
		other.head = nullptr;
	}

	/// Destroy and deallocates all clients
	FORCE_INLINE void empty()
	{
		LinkRef it = head;
		while (it)
		{
			LinkRef next = it->next;

			// Destroy link
			it->~Link();
			allocator->free(it);

			// Next link
			it = next;
		}
	}

	/// Destructor
	FORCE_INLINE ~Stack()
	{
		// Remove all clients
		empty();

		// Delete allocator
		if (bHasOwnAllocator) delete allocator;
	}

	/// Returns whether list is empty or not
	FORCE_INLINE bool isEmpty() const
	{
		return head == nullptr;
	}

	/**
	 * Push element on stack
	 * 
	 * @param [in] data element data
	 * @return inserted element
	 */
	template<typename _T = T>
	T & push(_T && data)
	{
		LinkRef link = createLink(forward<_T>(data));
		
		++count;
		return link->next = head, (head = link)->data;
	}

	/**
	 * Pop element from stack
	 * 
	 * @param [out] data removed element data
	 * @return true if stack not empty
	 * @{
	 */
	bool pop()
	{
		if (LIKELY(head != nullptr))
		{
			LinkRef link = head;
			head = head->next;

			// Destroy link
			link->~Link();
			allocator->free(link);

			return true;
		}

		return false;
	}
	bool pop(T & data)
	{
		if (LIKELY(head != nullptr))
		{
			LinkRef link = head;
			head = head->next;

			// Move data
			data = move(link->data);

			// Destroy link
			link->~Link();
			allocator->free(link);

			return true;
		}

		return false;
	}
};

//////////////////////////////////////////////////
// List
//////////////////////////////////////////////////

/**
 * @class ListIterator containers/list.h
 */
template<typename T>
class ListIterator
{
	template<typename, typename> friend class List;

public:

	/// Link types @{
	using Link		= ::DoubleLink<typename RemoveConst<T>::Type>;
	using LinkRef	= ::DoubleLinkRef<typename RemoveConst<T>::Type>;
	/// @}

protected:
	/// Current link
	LinkRef link;

public:
	/// Default constructor
	FORCE_INLINE ListIterator(LinkRef _link = nullptr)
		: link{_link} {}

	//////////////////////////////////////////////////
	// Iterator interface
	//////////////////////////////////////////////////
	
	/// Increment iterator
	/// @{
	FORCE_INLINE ListIterator & operator++()
	{
		link = link->next;
		return *this;
	}
	FORCE_INLINE ListIterator operator++(int32)
	{
		ListIterator out(*this);
		link = link->next;
		return out;
	}
	/// @}

	/// Decrement iterator
	/// @{
	FORCE_INLINE ListIterator & operator--()
	{
		link = link->prev;
		return *this;
	}
	FORCE_INLINE ListIterator operator--(int32)
	{
		ListIterator out(*this);
		link = link->prev;
		return out;
	}
	/// @}

	/**
	 * Compare iterators
	 * 
	 * @param [in] other other list iterator
	 * @return bool
	 * @{
	 */
	FORCE_INLINE bool operator==(const ListIterator & other)
	{
		return link == other.link;
	}
	FORCE_INLINE bool operator!=(const ListIterator & other)
	{
		return link != other.link;
	}
	/// @}

	/**
	 * Access link data
	 * 
	 * @return reference or pointer to data
	 * @{
	 */
	FORCE_INLINE T & operator*() const
	{
		return link->data;
	}
	FORCE_INLINE T * operator->() const
	{
		return &link->data;
	}
	/// @}
};

/**
 * @class List containers/linked_list.h
 * 
 * A doubly linked list class that uses @ref Link
 */
template<typename T, typename AllocT = MallocAnsi>
class List
{
public:
	/// Link types @{
	using Link		= ::DoubleLink<T>;
	using LinkRef	= ::DoubleLinkRef<T>;
	/// @}

	/// Iterator types @{
	using Iterator		= ListIterator<T>;
	using ConstIterator	= ListIterator<const T>;
	/// }

protected:
	/// List allocator @{
	AllocT * allocator;
	bool bHasOwnAllocator;
	/// @}

	/// List head
	LinkRef head;

	/// List tail
	LinkRef tail;

	/// Number of links
	uint64 count;

public:
	/// Default constructor
	FORCE_INLINE List(AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: allocator{_allocator}
		, bHasOwnAllocator(_allocator == nullptr)
		, head{nullptr}
		, tail{nullptr}
		, count{0}
	{
		// Create own allocator
		if (bHasOwnAllocator)
			allocator = new AllocT;
	}

protected:
	/**
	 * Allocates and construct a new link
	 */
	template<typename _T = T>
	FORCE_INLINE LinkRef createLink(_T && data)
	{
		return new (reinterpret_cast<Link*>(allocator->malloc(sizeof(Link), alignof(Link)))) Link(forward<_T>(data));
	}

	/**
	 * Replicates list by copying all links
	 * from source list
	 */
	void replicate(LinkRef _head)
	{
		if (_head)
			for (head = tail = createLink(_head->data); _head = _head->next; )
			{
				LinkRef link = createLink(_head->data);
				link->linkPrev(tail), tail = link;
			}
	}

public:
	/// Copy constructor
	FORCE_INLINE List(const List & other, AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: List(_allocator)
	{
		// Copy links
		replicate(other.head);
		count = other.count;
	}

	/// Copy constructor (different allocator)
	template<typename AllocU>
	FORCE_INLINE List(const List<T, AllocU> & other, AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: List(_allocator)
	{
		// Copy links
		replicate(other.head);
		count = other.count;
	}

	/// Move constructor
	FORCE_INLINE List(List && other)
		: allocator{other.allocator}
		, bHasOwnAllocator{other.bHasOwnAllocator}
		, head{other.head}
		, tail{other.tail}
		, count{other.count}
	{
		other.allocator = nullptr;
		other.bHasOwnAllocator = false;
		other.head = other.tail = nullptr;
		other.count = 0;
	}

	/// Copy assignment
	template<typename AllocU = AllocT>
	FORCE_INLINE List & operator=(const List<T, AllocU> & other)
	{
		// Empty this first
		empty();

		// Copy list
		replicate(other.head);
		count = other.count;

		return *this;
	}

	/// Copy assignment (same allocator)
	FORCE_INLINE List & operator=(const List & other)
	{
		// Empty this first
		empty();

		// Copy list
		replicate(other.head);
		count = other.count;

		return *this;
	}

	/// Move assignment
	FORCE_INLINE List & operator=(List && other)
	{
		// Empty this first
		empty();

		// Destroy own allocator
		if (bHasOwnAllocator) delete allocator;

		// Do move
		allocator = other.allocator;
		bHasOwnAllocator = other.bHasOwnAllocator;

		head = other.head;
		count = other.count;

		other.bHasOwnAllocator = false;
		other.head = nullptr;

		return *this;
	}

	/**
	 * Destroys list, deallocates all links
	 * and calls destructors
	 */
	void empty()
	{
		LinkRef it = head;
		while (it)
		{
			LinkRef next = it->next;

			// Dealloc link
			it->~Link();
			allocator->free(it);

			// Next link
			it = next;
		}

		head = tail = nullptr;
		count = 0;
	}

	/// Destructor
	FORCE_INLINE ~List()
	{
		// Free list
		empty();

		// Delete allocator
		if (bHasOwnAllocator) delete allocator;
	}

	/// Get count
	FORCE_INLINE uint64 getCount() const
	{
		return count;
	}

	/// Returns whether list is empty or not
	FORCE_INLINE bool isEmpty() const
	{
		return head == nullptr;
	}

	/**
	 * Returns list iterator
	 * 
	 * @return begin, last or end list iterator
	 * @{
	 */
	FORCE_INLINE Iterator		begin()			{ return Iterator(head); }
	FORCE_INLINE ConstIterator	begin() const	{ return ConstIterator(head); }

	FORCE_INLINE Iterator		last()			{ return Iterator(tail); }
	FORCE_INLINE ConstIterator	last() const	{ return ConstIterator(tail); }

	FORCE_INLINE Iterator		end()		{ return Iterator(); }
	FORCE_INLINE ConstIterator	end() const	{ return ConstIterator(); }
	/// @}

	/**
	 * Insert item at the beginning of the list
	 * 
	 * @param [in] item inserted item
	 * @return inserted item
	 */
	template<typename _T = T>
	T & insert(_T && data)
	{
		LinkRef link = createLink(forward<_T>(data));
		link->linkNext(head);
		return tail = (tail ? tail : link), (head = link)->data;
	}

	/**
	 * Insert item at the end of the list
	 * 
	 * @param [in] item inserted item
	 * @return inserted item
	 */
	template<typename _T = T>
	T & push(_T && data)
	{
		++count;
		
		LinkRef link = createLink(forward<_T>(data));
		link->linkPrev(tail);
		return head = (head ? head : link), (tail = link)->data;
	}

	/**
	 * Remove link from list
	 * 
	 * @param [in] link link to remove
	 * @param [in] it list iterator
	 * @{
	 */
	void remove(LinkRef link)
	{
		--count;

		// Fix head and tail
		head = link == head ? head->next : head;
		tail = link == tail ? tail->prev : tail;

		// Unlink
		link->unlink();
	}
	FORCE_INLINE void remove(Iterator it)
	{
		if (it.link) remove(it.link);
	}
	FORCE_INLINE void remove(ConstIterator it)
	{
		if (it.link) remove(it.link);
	}
	/// @}
};