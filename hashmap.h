//
// Change history:
//		02/01/10 Initial version (H. Kurosu)
//

#ifndef _HASH_MAP_H
#define _HASH_MAP_H

#include <inttypes.h>
#include <assert.h>

//
//  T4 Hash table implementation
//
namespace cpphashmap
{
	/**
	* Hash function
	*
	* Sometime, you need to specialize for your own type T.
	* See HashFunc<const void*> below.
	*/
	template <typename T>
	class HashFunc
	{
	public:
		HashFunc<T>() {}
		~HashFunc<T>() {}

		uint32_t operator() (const T& val) const
		{
			// needs to implement yur hash function
		}
	};

	/**
	* Hash function for pointers
	*
	* T4hash to a little expensive for pointer value's hashing.
	*/
	template <>
	class HashFunc<const void*>
	{
	public:
		uint32_t operator() (const void* val) const
		{
			uint32_t x = static_cast<uint32_t>(reinterpret_cast<size_t>(val));
			return x + (x >> 3) + (x >> 13) + (x >> 23);
		}
	};

	/**
	* Hash table element data structore
	*
	* Internally, Hash table saves element as bi-directional
	* linked list.
	*/
	template <typename K, typename T>
	struct Node
	{
		typedef K	 key_type;
		typedef T	 mapped_type;

		K			 key;
		T			 value;
		Node<K, T>*	 _next;
		Node<K, T>*	 _prev;

		Node()
			: key(), value(), _next(NULL), _prev(NULL)
		{}

		Node(const K& k, const T& v)
			: key(k), value(v), _next(NULL), _prev(NULL)
		{}

		~Node()
		{}

		Node<K, T>* next() const { return _next; }
		Node<K, T>* prev() const { return _prev; }

		/**
		* Insert new node to next position of this node
		* @return self
		*/
		Node<K, T>& insertNext(Node<K, T>* node)
		{
			Node<K, T>* next = _next;
			this->_next = node;
			if (next)
			{
				next->_prev = node;
			}
			if (node)
			{
				node->_prev = this;
				node->_next = next;
			}
			return *this;
		}

		/**
		* append new node
		*/
		Node<K, T>& append(Node<K, T>* node)
		{
			_next = node;
			if (node) node->_prev = this;
			return *this;
		}

		/**
		* remove self from the linked list, but
		* dooen't change self at all.
		*
		* @return self
		*/
		const Node<K, T>& removeSelf() const
		{
			Node<K, T>* n = _next;
			Node<K, T>* p = _prev;
			if (n) n->_prev = p;
			if (p) p->_next = n;
			return *this;
		}
	};

	/**
	* Iterator for Hash table Node
	*/
	template <typename N>
	class NodeIterator
	{
	public:
		typedef N node_type;

		NodeIterator(node_type* node) : _cur(node) {}
		~NodeIterator() {}

		NodeIterator<N>& operator++()
		{
			if (_cur)
			{
				_cur = _cur->_next;
			}
			return *this;
		}

		NodeIterator<N>& operator=(const NodeIterator<N>& right)
		{
			_cur = right._cur;
			return *this;
		}

		node_type* operator->() const { return _cur; }
		node_type* operator*()  const { return _cur; }

		bool operator==(const NodeIterator<N>& right) const
		{
			return _cur == right._cur;
		}

		bool operator!=(const NodeIterator<N>& right) const
		{
			return _cur != right._cur;
		}

	private:
		node_type* _cur;
	};

	/**
	* Const iterator for Hash table node
	*/
	template <typename N>
	class ConstNodeIterator
	{
	public:
		typedef N node_type;

		ConstNodeIterator(node_type* node) : _cur(node) {}
		~ConstNodeIterator() {}

		ConstNodeIterator<N>& operator++()
		{
			if (_cur)
			{
				_cur = _cur->_next;
			}
			return *this;
		}

		ConstNodeIterator<N>& operator=(const ConstNodeIterator<N>& right)
		{
			_cur = right->_cur;
			return *this;
		}

		node_type const* operator->() const { return _cur; }
		node_type const* operator*()  const { return _cur; }

		bool operator==(const ConstNodeIterator<N>& right) const
		{
			return _cur == right._cur;
		}

		bool operator!=(const ConstNodeIterator<N>& right) const
		{
			return _cur != right._cur;
		}

	private:
		node_type* _cur;
	};

	/**
	* Chunk of Hash table Node
	*
	*/
	template <typename N, typename A>
	struct NodeChunk
	{
		N*				data;
		int				size;
		NodeChunk<N, A>	*next;

		NodeChunk(N* data, int s);
		~NodeChunk();
	};

	/**
	* Hash table element pool
	*/
	template <typename N, typename A>
	class NodePool
	{
	public:
		typedef N							node_type;
		typedef typename N::key_type		key_type;
		typedef typename N::mapped_type		mapped_type;
		typedef A							allocator_type;
		typedef NodeChunk<N, A>				chunk_type;

		node_type* next(const key_type& k, const mapped_type& v);
		void release(node_type* v);

		NodePool()
			: _chunk(NULL)
			, _next(NULL)
		{}

		~NodePool();

	private:
		enum { _chunkSize = 256, };
		void addchunk();

		chunk_type*	_chunk;
		node_type*  _next;
	};

	/**
	* Hash table bucket
	*
	* In a bucket, elements that has same masked hash value
	* are stored. Normally, the element count stored in one
	* bucket should be very small.
	*
	*/
	template <typename V>
	class Bucket
	{
	public:
		typedef V						value_type;
		typedef typename V::key_type	key_type;
		typedef typename V::mapped_type	mapped_type;

		value_type* first() const { return _first; }
		value_type* get(const key_type& k) const;
		void insert(value_type* v);
		value_type* remove(const key_type& k);
		int count() const { return _count; }
		bool empty() const { return _count == 0; }

		Bucket() : _first(NULL), _count(0) {}
		~Bucket() {}

	private:
		value_type*	_first;
		int			_count;
	};

	/**
	* Simple allocator
	*/
	class Allocator
	{
	public:
		void* allocate(unsigned int size)
		{
			return ::malloc(size);
		}

		void deallocate(void* ptr) {
			::free(ptr);
		}
	};

}


/**
* hash table implementation
*/
template <typename K,
	typename T,
	typename H = cpphashmap::HashFunc<K>,
	typename A = cpphashmap::Allocator>
class HashMap
{
public:
	// type declarations
	typedef K key_type;
	typedef T mapped_type;
	typedef H hasher;
	typedef A allocator_type;
	typedef cpphashmap::Node<K, T>					node_type;
	typedef cpphashmap::NodeIterator<node_type>		iterator;
	typedef cpphashmap::ConstNodeIterator<node_type> const_iterator;
	typedef cpphashmap::NodePool<node_type, A>		pool_type;
	typedef cpphashmap::Bucket<node_type>			bucket_type;

	/**
	* Constructor - SHOULD NOT trigger any heap allocation
	*/
	HashMap()
		: _pool()
		, _buckets(NULL)
		, _bucketSize(0)
		, _count(0)
		, _mask(0)
		, _initSize(1 << 6)
		, _first(NULL)
	{
	}

	/**
	* Constructor with initial table size
	*
	* Even though it takes initial size, the internal data allocation
	* is delayed until the first element is inserted.
	*
	*/
	HashMap(int initSize)
		: _pool()
		, _buckets(NULL)
		, _bucketSize(0)
		, _count(0)
		, _mask(0)
		, _initSize(1 << 6)
		, _first(NULL)
	{
		while (_initSize < initSize)
		{
			_initSize = _initSize << 1;
		}
	}

	/**
	* Destruction - clear all internal data
	*/
	~HashMap() { clear(); }

	iterator begin() { return _first; }
	const_iterator begin() const { return _first; }
	iterator end() { return NULL; }
	const_iterator end() const { return NULL; }
	bool empty() const { return _count == 0; }
	size_t count() const { return _count; }

	/**
	* Find the element
	*
	* @param k key value
	* @return Returns iterator type. end() is returned when data
	*  is not found
	*/
	iterator find(const key_type& k)
	{
		return _find(k);
	}

	/**
	* Find the element
	*
	* @param k key value
	* @return Returns const_iterator type. end() is returned when data
	*  is not found
	*/
	const_iterator find(const key_type& k) const
	{
		return _find(k);
	}

	/**
	* Insert new element
	* If element is already stored, the value is overriden
	* @param k key value
	* @param v element value
	*/
	iterator insert(const key_type& k, const mapped_type& v);

	/**
	* Erase the element
	* If the element is not found, nothing happens.
	* @param k key value
	*/
	void erase(const key_type& k);

	/**
	* Element the element of iterator's value
	* @param i iterator to delete
	*/
	void erase(iterator& i);

	/**
	* operator[]
	*
	* Similar to find(), but return empty value reference when
	* the element is not found by key.
	*
	* @param k key value
	*/
	mapped_type& operator[](const key_type& k);

	/**
	* Re-hash the table with new size
	*/
	void rehash(int newSize);

	/**
	* Clear all element
	*/
	void clear();

private:

	/**
	* calculate masked hash
	*/
	uint32_t mask(uint32_t hash) const { return hash & _mask; }

	// internal implementation of find()
	node_type* _find(const key_type& k) const;

	// insert helper
	void _insert(bucket_type& bucket, node_type* node);

	// debugging
	bool check() const;

	// construct table from the existing elments
	void construct(node_type* node);

	pool_type		_pool;
	bucket_type*	_buckets;
	int				_bucketSize;
	int				_count;
	uint32_t		_mask;
	int				_initSize;
	node_type*		_first;
};

template <typename K, typename T, typename H, typename A>
inline bool HashMap<K, T, H, A>::check() const
{
	int count = 0;
	node_type* node = _first;
	while (node)
	{
		++count;
		node = node->next();
	}
	return _count == count;
}

// find the element, return the pointer of value_type
template <typename K, typename T, typename H, typename A>
inline typename HashMap<K, T, H, A>::node_type*
HashMap<K, T, H, A>::_find(const key_type& k) const
{
	uint32_t pos = mask(hasher()(k));
	if (!_buckets || pos >= (uint32_t)_bucketSize)
	{
		return NULL;
	}
	return _buckets[pos].get(k);
}

// insert new elment
template <typename K, typename T, typename H, typename A>
inline typename HashMap<K, T, H, A>::iterator
HashMap<K, T, H, A>::insert(const key_type& k, const mapped_type& v)
{
	// make sure the table size is big enough to store new data
	rehash(_count + 1);

	// look for the data in the bucket
	uint32_t pos = mask(hasher()(k));
	node_type* node = _buckets[pos].get(k);

	// no data in the bucket
	if (node == NULL)
	{
		// get the node from the pool
		node = _pool.next(k, v);

		// insert node into bucket
		_insert(_buckets[pos], node);
	}
	else
	{
		// data is found. just override the element value
		node->value = v;
	}
	return iterator(node);
}

// insert helper - insert node into the bucket
// keep the integrity of global list
template <typename K, typename T, typename H, typename A>
inline void
HashMap<K, T, H, A>::_insert(bucket_type& bucket, node_type* node)
{
	// if the node is the first entry of the bucket,
	// add it into the top of linkage
	if (bucket.empty())
	{
		node->append(_first);
		_first = node;
	}

	// put it into the current bucket
	bucket.insert(node);
	// increment element counter
	++_count;

	// pointer consistency check
	assert(check());
}

// erase the existing data
template <typename K, typename T, typename H, typename A>
inline void HashMap<K, T, H, A>::erase(const key_type& k)
{
	if (!_buckets) return;

	// try to remove from the bucket
	uint32_t pos = mask(hasher()(k));
	node_type* node = _buckets[pos].remove(k);
	if (node)
	{
		if (_first == node)
		{
			_first = node->next();
		}

		// node was deleted from the bucket
		// then, return the node to the pool
		_pool.release(node);

		// decrease element counter
		--_count;

		// pointer consistency check
		assert(check());
	}
}

// erase the element from iterator
template <typename K, typename T, typename H, typename A>
inline void HashMap<K, T, H, A>::erase(iterator& i)
{
	erase(i->key);
}

// array style accessor
template <typename K, typename T, typename H, typename A>
inline T& HashMap<K, T, H, A>::operator[](const key_type& k)
{
	// try to find the element
	node_type* node = _find(k);
	if (node)
	{
		return node->value;
	}
	// if data doesn't exist, create empty element
	// then return it.
	iterator i = insert(k, mapped_type());
	return i->value;
}

// increase the table size if necessary
template <typename K, typename T, typename H, typename A>
inline void HashMap<K, T, H, A>::rehash(int newSize)
{
	if (!_buckets)
	{
		// bucket list initialization (normally, this is 
		// done at the first insertion of data)
		_bucketSize = _initSize;
		_buckets = (bucket_type*)allocator_type().allocate(sizeof(bucket_type)* _bucketSize);

		// make sure constructor of each bucket is called
		for (int i = 0; i < _bucketSize; ++i)
		{
			new (_buckets + i) bucket_type();
		}

		// set mask value
		_mask = _bucketSize - 1;
	}
	else if (newSize >(_bucketSize << 1))
	{
		// caluclate the new size to align with 2^n
		int sz = _bucketSize << 1;
		while ((sz << 1) < newSize) sz = sz << 1;

		// deallocate the current container
		for (int i = 0; i < _bucketSize; ++i)
		{
			_buckets[i].~bucket_type();
		}
		allocator_type().deallocate(_buckets);

		// allocate with new size
		_bucketSize = sz;
		_buckets = (bucket_type*)allocator_type().allocate(sizeof(bucket_type)* _bucketSize);
		for (int i = 0; i < _bucketSize; ++i)
		{
			new (_buckets + i) bucket_type();
		}
		_mask = _bucketSize - 1;

		// construct table from the exising elments
		construct(_first);
	}
	return;
}

// construct table from the existing linked list
template <typename K, typename T, typename H, typename A>
inline void HashMap<K, T, H, A>::construct(node_type* node)
{
	// clear the list counter, pointers
	_first = NULL;
	_count = 0;

	while (node)
	{
		node_type* next = node->_next;

		// disconnect the working list
		node->_prev = NULL;
		node->_next = NULL;

		// insert node into the new bucket
		uint32_t pos = mask(hasher()(node->key));
		_insert(_buckets[pos], node);

		// proceed to next node
		node = next;
	}
}

// clear all internal data
template <typename K, typename T, typename H, typename A>
inline void HashMap<K, T, H, A>::clear()
{
	if (_buckets)
	{
		// deallocate the current container
		for (int i = 0; i < _bucketSize; ++i)
		{
			_buckets[i].~bucket_type();
		}
		allocator_type().deallocate(_buckets);

		_buckets = NULL;
		_bucketSize = 0;
		_mask = 0;
		_count = 0;
	}

	if (_first)
	{
		node_type* node = _first;
		while (node)
		{
			node_type* next = node->next();
			_pool.release(node);
			node = next;
		}
		_first = NULL;
	}
}

namespace cpphashmap
{
	//
	// Hash table node chunk
	//
	template <typename N, typename A>
	inline 	NodeChunk<N, A>::~NodeChunk()
	{
		// make sure node's destructor is called
		// before deallocate().
		for (int i = 0; i < size; ++i)
		{
			data[i].~N();
		}
	}

	// create new chunk with given size
	template <typename N, typename A>
	inline 	NodeChunk<N, A>::NodeChunk(N* data, int s)
		: data(data), size(s), next(NULL)
	{
		for (int i = 0; i < size; ++i)
		{
			// new operator in place to make sure N's
			// constructor is called.
			new (data + i) N();
			if (i > 0)
			{
				// make the next and prev linkage
				data[i]._prev = data + i - 1;
				data[i - 1]._next = data + i;
			}
		}
	}

	//
	// Hash table node pool
	//
	template <typename N, typename A>
	inline NodePool<N, A>::~NodePool()
	{
		// deallocate all the node chunk in this 
		// pool
		chunk_type* c = _chunk;
		while (c)
		{
			chunk_type* next = c->next;
			c->~chunk_type();
			allocator_type().deallocate(c);
			c = next;
		}
	}

	// get next available node from the pool
	// If no availablibity, it creates next node chunk
	template <typename N, typename A>
	inline N* NodePool<N, A>::next(const key_type& k, const mapped_type& v)
	{
		// if no more available node, create new chunk
		if (!_next || !_next->next())
		{
			addchunk();
		}

		// get next available
		node_type* node = _next;

		// proceeed to next availables
		_next = _next->next();
		assert(_next != NULL);

		// set values
		node->key = k;
		node->value = v;

		// clear linkages
		node->_next = NULL;
		node->_prev = NULL;

		return node;
	}

	// remove the node from active list, restore
	// to available list
	template <typename N, typename A>
	inline void NodePool<N, A>::release(node_type* v)
	{
		if (!v) return;

		// return v to available list
		assert(_next != NULL);

		// put the node at the beginning of availiable list
		v->append(_next);
		_next = v;
	}

	// create new chunk and add them into available list
	template <typename N, typename A>
	inline void NodePool<N, A>::addchunk()
	{
		if (!_chunk)
		{
			// initial chunk allocation
			chunk_type* ptr = (chunk_type*)allocator_type().allocate(sizeof(chunk_type)+sizeof(node_type)* _chunkSize);
			_chunk = ptr++;
			new (_chunk)chunk_type((node_type*)ptr, _chunkSize);

			_next = _chunk->data;
			return;
		}

		assert(_next != NULL);

		// find the last chunk node
		chunk_type* chunk = _chunk;
		while (chunk->next) chunk = chunk->next;

		// create new chunk and save to next pointer
		chunk_type* ptr = (chunk_type*)allocator_type().allocate(sizeof(chunk_type)+sizeof(node_type)* _chunkSize);
		chunk->next = ptr++;

		// call constructor
		chunk = chunk->next;
		new (chunk)chunk_type((node_type*)ptr, _chunkSize);

		// make a link to new chunk
		_next->_next = chunk->data;
	}

	//
	// Hash table bucket
	//

	// find the element in the bucket
	template <typename V>
	inline V* Bucket<V>::get(const key_type& k) const
	{
		if (_count == 0)
		{
			return NULL;
		}
		int pos = 0;
		value_type* node = _first;
		while (node && pos < _count)
		{
			if (k == node->key)
			{
				return node;
			}
			++pos;
			node = node->_next;
		}
		return NULL;
	}

	// put new element into the bucket
	template <typename V>
	inline void Bucket<V>::insert(value_type* v)
	{
		if (_count == 0)
		{
			_first = v;
		}
		else
		{
			assert(_first != NULL);
			// insert node to next positio of the _first
			_first->insertNext(v);

		}
		++_count;
		return;
	}

	// remove the element from the bucket if exist
	// Otherwise, do nothing.
	template <typename V>
	inline V* Bucket<V>::remove(const key_type& k)
	{
		value_type* v = get(k);
		if (!v) return NULL;
		if (v == _first)
		{
			if (_count == 1)
			{
				_first = NULL;
			}
			else
			{
				_first = v->_next;
			}
		}
		v->removeSelf();
		--_count;
		return v;
	}

} // end of namespace cpphashmap

#endif
