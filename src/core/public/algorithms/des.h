#pragma once

#include "coremin.h"
#include "containers/bitarray.h"

/**
 * DES parameters
 */
struct DES
{
	/// Default DES implementation
	static const DES std;

	/// Number of rounds
	uint32 numRounds;

	/// Initial and final permutations @{
	const uint32 * ip;
	const uint32 * fp;
	/// @}

	/// Expansion table
	const uint32 * xpn;

	/// Permutation table
	const uint32 * perm;

	/// Sbox tables
	const uint32 * subs[8];

	/// Sbox pre-computed differential tables
	/// (only required for differential cryptanalysis)
	const uint32 * difs[8];

	/**
	 * DES key schedule
	 * 
	 * @param [out] roundKeys scheduled round keys
	 * @param [in] key actual key
	 */
	void keySchedule(BitArray * keys, const BitArray & key) const;

	/**
	 * DES block encryption
	 * 
	 * @param [in] ptx plaintext bit array
	 * @param [in] roundKeys pre-computed round keys
	 * @return output ctx bit array
	 */
	BitArray encryptBlock(const BitArray & ptx, const BitArray * roundKeys) const;

	/**
	 * DES block decryption
	 * 
	 * @param [in] ctx ciphertext bit array
	 * @param [in] roundKeys pre-computed round keys
	 * @return output ptx bit array
	 */
	BitArray decryptBlock(const BitArray & ctx, const BitArray * roundKeys) const;
};

/**
 * Instance of a single des round
 * described in terms of differentials
 */
struct RoundInstance
{
	/// Right bitarray
	BitArray dr;

	/// Left bitarray
	BitArray dl;

	/// Sbox output
	BitArray dy;
};

/**
 * Structure used for MCTS
 */
struct RoundNode
{
	/// Instance of this round
	RoundInstance instance;

	/// Round differential probability
	float64 g;

	/// Path heuristic
	float64 h;

	/// Current round
	uint32 round;

	/// Des parameters
	DES * params;

public:
	/**
	 * Init with plaintext
	 * 
	 * @param [in] ptx plaintext bitarray
	 */
	FORCE_INLINE void init(const BitArray & ptx)
	{
		// Set differentials
		instance.dr = BitArray(32);
		instance.dl = BitArray(32);

		ptx.permute(instance.dr, params->ip);
		ptx.permute(instance.dl, params->ip + 32);

		// Set probability and round
		g = 0., h = 0., round = 1;
	}

	//////////////////////////////////////////////////
	// Monte carlo interface
	//////////////////////////////////////////////////
	
	/// Returns round score
	FORCE_INLINE float64 getScore() const
	{
		return g;
	}

	/// Returns heuristic value
	FORCE_INLINE float64 getHeuristicScore() const
	{
		return h;
	}

	/// Compute node heuristics
	FORCE_INLINE float64 computeH()
	{
		float64 h = 0.0;
		BitArray dx(48);

		// Compute sbox input
		instance.dr.permute(dx, params->xpn);

		const uint32 numLeftRounds = params->numRounds - round - 1;
		if (numLeftRounds)
		{
			for (uint32 s = 0; s < 8; ++s)
			{
				const uint32 * row = params->difs[s] + (dx(s * 6, (s + 1) * 6) << 4);

				// Find max probability
				uint32 max = row[0]; for (uint32 i = 1; i < 16; ++i) max = Math::max(max, row[i]);
				h -= log2(max / 64.0);
			}
		}

		return h;
	}

protected:
	/**
	 * Expand node internal
	 * 
	 * @param [out] out out list of expanded nodes
	 * @param [in] dx,dy input and output of sbox
	 * @param [in] params des parameters
	 * @param [in] s sbox index
	 */
	void expand_internal(List<RoundNode> & out, const BitArray & dx, const BitArray & dy, float64 g0 = 0., uint32 s = 0) const
	{
		if (s < 8)
		{
			uint32 x = dx(s * 6, (s + 1) * 6);
			if (x != 0)
				// For each possible output
				for (ubyte y = 0; y < 16; ++y)
				{
					const uint32 d = params->difs[s][(x << 4) + y];

					if (d > 0)
					{
						const float64 p = d / 64.f;
						const ubyte _y = y << 4;
						expand_internal(out, dx, dy.merge(BitArray(&_y, 4)), g0 - log2(p), s + 1);
					}
				}
			else
				expand_internal(out, dx, dy.merge(BitArray((const ubyte[]){0x0}, 4)), g0, s + 1);
		}
		else
		{
			// Compute next round node
			BitArray u(32);
			RoundNode nextRound{
				instance : {
					dr : dy.permute(u, params->perm) ^= instance.dl,
					dl : instance.dr,
					dy : dy
				},
				g : g0,
				round : round + 1,
				params : params
			};

			// Compute heuristic
			nextRound.computeH();

			out.push(move(nextRound));
		}
	}

public:
	/// Expand this node
	List<RoundNode> expand()
	{
		// Out list of expanded rounds
		List<RoundNode> out;
		
		// Don't expand last round
		if (round < params->numRounds - 1)
		{
			// Compute sbox input
			BitArray dx(48), dy(0);

			// Recursive expansion
			expand_internal(out, instance.dr.permute(dx, params->xpn), dy);
		}

		return out;
	}
};

/**
 * A path represents a differential
 * path in DES. A list of round
 * instances is mantained. It is
 * possible to begin a depth first
 * search starting from this node
 * or to simply sample a random
 * walk down to the last round
 */
class Path
{
protected:
	/// Partial differential path
	List<RoundInstance> rounds;

	/// Cost
	float64 g;

	/// Heuristic value
	float64 h;

	/// Des parameters
	const DES * params;

public:
	/// Default constructor
	FORCE_INLINE Path(const DES * _params)
		: rounds{}
		, g{0.f}
		, h{0.f}
		, params{_params} {}
	
	/// Copy constructor
	Path(const Path & other) = default;

	/// Move constructor
	Path(Path && other) = default;

	/// Copy assignment
	Path & operator=(const Path & other) = default;

	/// Move assignment
	Path & operator=(Path && other) = default;

	/// Get total path cost
	FORCE_INLINE float64 getTotalCost() const
	{
		return g + h;
	}

	/// Get actual path cost
	FORCE_INLINE float64 getActualCost() const
	{
		return g;
	}

	/// Get estimated cost
	FORCE_INLINE float64 getEstimatedCost() const
	{
		return h;
	}

	// Get current round
	FORCE_INLINE uint32 getRound() const
	{
		return rounds.getCount();
	}

	/// Returns true if path is a complete path
	FORCE_INLINE bool isComplete() const
	{
		return rounds.getCount() >= params->numRounds - 1;
	}

	/// Init path with des input (before initial permutation)
	void init(const BitArray & ptx);

	/// Compute node heuristics
	void computeH();

protected:
	/**
	 * Expand node internal
	 * @see expand
	 * 
	 * @param [out] out out list of expanded nodes
	 * @param [in] dx,dy input and output of sbox
	 * @param [in] params des parameters
	 * @param [in] s sbox index
	 */
	void expand_internal(List<Path> & out, const BitArray & dx, const BitArray & dy, float64 p0 = 1.f, uint32 s = 0) const;

public:
	/// Expand node
	FORCE_INLINE List<Path> expand()
	{
		// Out list of expanded node
		List<Path> out;

		// Compute sbox input	
		BitArray dx(48), dy(0);

		// Recursive expansion
		expand_internal(out, rounds.last()->dr.permute(dx, params->xpn), dy);

		return out;
	}

protected:
	/**
	 * Internal DFS search
	 * @see dfSearch
	 */
	void dfSearch_internal(Path & optimalPath, float64 & cost, const BitArray & dx, const BitArray & dy, float64 c0 = 0., uint32 s = 0) const;

public:
	/**
	 * Performs a depth first search starting
	 * from this node
	 * 
	 * @param [out] optimalPath found path with minimum cost
	 * @param [out] cost minimum cost
	 */
	FORCE_INLINE void dfSearch(Path & optimalPath, float64 & cost) const
	{
		BitArray dx(48), dy(0);
		dfSearch_internal(optimalPath, cost, rounds.last()->dr.permute(dx, params->xpn), dy);
	}

protected:
	/**
	 * Internal sample code
	 * @see sample
	 * 
	 * @param [in] dx sbox differential input
	 * @param [in] p0 out differential probability
	 * @param [in] s simulated sbox index
	 */
	float64 sample_internal(BitArray & dy, const BitArray & dx, float64 p0 = 0., uint32 s = 0) const;

public:
	/**
	 * Simulate a random walk starting from this partial path
	 * 
	 * @return simualated path cost
	 */
	float64 sample() const;
};