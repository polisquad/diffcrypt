#pragma once

#include "core_types.h"
#include "pair.h"
#include "binary_tree.h"
#include "hal/malloc_binned.h"

/**
 * @class Map containers/tree_map.h
 * 
 * A map built on top of a binary tree
 * 
 * A tree map has an optimal space-efficiency. Keys
 * are sorted and all basic operations (insertion,
 * deletion, search) are O(log(n))
 */
template<typename KeyT, typename ValT, typename CompareT = Compare<KeyT>, typename AllocT = MallocBinned>
class Map
{
public:
	/// Pair type
	using Pair = ::Pair<KeyT, ValT, CompareT>;

	/// Tree type
	using Tree = BinaryTree<Pair, Compare<Pair>, AllocT>;

	/// Node type
	using Node		= typename Tree::Node;
	using NodeRef	= typename Tree::NodeRef;

	/// Iterator types @{
	using Iterator		= typename Tree::Iterator;
	using ConstIterator	= typename Tree::ConstIterator;

	using MapIterator		= typename Tree::TreeIterator;
	using ConstMapIterator	= typename Tree::ConstTreeIterator;
	/// @}

protected:
	/// Binary tree used for node storage
	Tree tree;

public:
	/// Default constructor
	FORCE_INLINE Map(AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: tree{_allocator} {}
	
	/// Returns number of nodes
	FORCE_INLINE uint64 getCount() const { return tree.numNodes; }

	/// Returns empty search iterator @{
	FORCE_INLINE Iterator		nil()		{ return tree.nil(); }
	FORCE_INLINE ConstIterator	nil() const	{ return tree.nil(); }
	/// @}

	/// Returns linear iterators @{
	FORCE_INLINE MapIterator		begin()			{ return tree.begin(); }
	FORCE_INLINE ConstMapIterator	begin() const	{ return tree.begin(); }

	FORCE_INLINE MapIterator		last()			{ return tree.last(); }
	FORCE_INLINE ConstMapIterator	last() const	{ return tree.last(); }

	FORCE_INLINE MapIterator		end()		{ return tree.end(); }
	FORCE_INLINE ConstMapIterator	end() const	{ return tree.end(); }
	/// @}

	/**
	 * Find value using key
	 * 
	 * @param [in] key search key
	 * @return map iterator
	 * @{
	 */
	template<typename _KeyT = KeyT>
	FORCE_INLINE Iterator find(_KeyT && key)
	{
		return tree.find(Pair(forward<_KeyT>(key)));
	}
	template<typename _KeyT = KeyT>
	FORCE_INLINE ConstIterator find(_KeyT && key) const
	{
		return tree.find(Pair(forward<_KeyT>(key)));
	}
	/// @}

	/**
	 * Returns ref to value associated with key
	 * 
	 * If key doesn't exist, create a new one
	 * 
	 * @param [in] key search key
	 * @return ref to associated value
	 */
	template<typename _KeyT = KeyT>
	ValT & operator[](_KeyT && key)
	{
		return tree.insertUnique(Pair(forward<_KeyT>(key))).second;
	}

	/**
	 * Insert a new unique pair
	 * 
	 * @param [in] pair <key, value> pair to insert
	 * @param [in] key key value
	 * @param [in] val pair value
	 * @return inserted pair or pair that prevented insertion
	 * @{
	 */
	template<typename _Pair = Pair>
	FORCE_INLINE Pair & insert(_Pair && pair)
	{
		return tree.insertUnique(forward<_Pair>(pair));
	}
	template<typename _KeyT = KeyT, typename _ValT = ValT>
	FORCE_INLINE Pair & insert(_KeyT && key, _ValT && val)
	{
		return insert(Pair(forward<_KeyT>(key), forward<_ValT>(val)));
	}
	/// @}

	/**
	 * Remove an element from the map
	 * 
	 * @param [in] it iterator
	 */
	template<typename It>
	FORCE_INLINE typename EnableIf<
										IsSameType<It, Iterator>			::value
									||	IsSameType<It, ConstIterator>		::value
									||	IsSameType<It, MapIterator>			::value
									||	IsSameType<It, ConstMapIterator>	::value,
	void>::Type remove(It it)
	{
		tree.remove(it);
	}

public:

};

