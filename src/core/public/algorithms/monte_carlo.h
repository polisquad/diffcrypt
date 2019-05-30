#pragma once

#include "coremin.h"

template<typename T>
struct GCC_ALIGN(16) MCNode
{
	template<typename, typename> friend class MCTree;

protected:
	/// Parent node
	MCNode * parent;

	/// Children nodes
	List<MCNode> children;

	/// Visitors count
	uint32 visited;

	/// Average score
	float64 score;

	/// Node data
	T data;

public:
	/// Default constructor
	template<typename _T = T>
	FORCE_INLINE MCNode(_T && _data, MCNode * _parent = nullptr)
		: parent{_parent}
		, visited{0}
		, score{0.}
		, data{forward<_T>(_data)} {}

	/// Returns true if node is already expanded
	FORCE_INLINE bool isExpanded() const
	{
		return !children.isEmpty();
	}

	/// Returns node characteristic value
	FORCE_INLINE float64 getValue() const
	{
		const float32 k = 0.2f;
		return visited ? (score / visited) + k * Math::sqrt(2.f * log(parent->visited) / visited) : FLT_MAX;
	}

	/// Returns node characteristic with heuristic value
	FORCE_INLINE float64 getHeuristicValue() const
	{
		return visited ? getValue() : data.getHeuristicScore();
	}

	/// Returns pointer to next node to explore
	MCNode * getNextNode()
	{
		MCNode * out = nullptr;
		float64 maxScore = -FLT_MAX;

		for (auto & child : children)
		{
			float32 score = child.getValue();
			if (score > maxScore) maxScore = score, out = &child;
		}

		return out;
	}

	/**
	 * Expand node if possible
	 * 
	 * @return true if node is expanded
	 */
	bool expand()
	{
		// Do not expand again
		if (isExpanded()) return true;

		// Add children
		for (auto & _data : data.expand())
			children.push(MCNode(move(_data), this));
		
		return !children.isEmpty();
	}

	/**
	 * Update node statistics
	 * 
	 * @param [in] _score score of a single run
	 */
	FORCE_INLINE void update(float32 _score)
	{
		score *= (visited / (visited + 1.f));
		score += _score / ++visited;
	}
};

template<typename T>
using MCNodeRef = MCNode<T>*;

/**
 * 
 */
template<typename T, typename AllocT = MallocAnsi>
struct MCTree
{
public:
	/// Node types @{
	using Node		= MCNode<T>;
	using NodeRef	= MCNodeRef<T>;
	/// @}

protected:
	/// Allocator in use @{
	AllocT * allocator;
	bool bHasOwnAllocator;
	/// }

	/// Root node
	NodeRef root;

public:
	/// Default constructor
	FORCE_INLINE MCTree(AllocT * _allocator = reinterpret_cast<AllocT*>(gMalloc))
		: allocator{_allocator}
		, bHasOwnAllocator{_allocator == nullptr}
		, root{nullptr}
	{
		if (bHasOwnAllocator) allocator = new AllocT;
	}

	/// Get best node
	
	/// Init tree with initial node
	template<typename _T = T>
	FORCE_INLINE void init(_T && data)
	{
		root = new (allocator->malloc(sizeof(Node))) Node(forward<_T>(data));
	}

	/// Sample a run
	void sample()
	{
		float32 score = 0.f;

		NodeRef it = root;
		while (it)
		{
			// Expand node if possible
			if (it->expand())
			{
				// Get next node
				it = it->getNextNode();
				
				// Update score
				score += it->data.getScore();

				printf("partial score: %.3f\n", score);
			}
			else break;
		}

		printf("score: %.3f\n", score);

		// Backtrack and update
		while (it)
		{
			printf("  |- node %p (visited: %u, score: %.3f)\n", it, it->visited, it->score);

			it->update(score);
			it = it->parent;
		}
	}
};