#pragma once

#include "coremin.h"

/**
 * DES parameters
 */
struct DesParams
{
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
	DesParams * params;

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

		LOG(INFO, "expanded nodes: %llu", out.getCount());

		return out;
	}
};

/**
 * A path represent a differential
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
	const DesParams * params;

public:
	/// Default constructor
	FORCE_INLINE Path(const DesParams * _params)
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
	void init(const BitArray & ptx)
	{
		// Create first round from ptx
		RoundInstance & inputRound = rounds.push(RoundInstance{
			dr : BitArray(32),
			dl : BitArray(32)
		});

		ptx.permute(inputRound.dl, params->ip);
		ptx.permute(inputRound.dr, params->ip + 32);

		// Init cost
		g = 0.;
		computeH();
	}

	/// Compute node heuristics
	void computeH()
	{
		// Out probability
		float64 h0 = 0.;

		// Round differential inputs
		auto & lastRound = *rounds.last();
		BitArray dx(48), dr = lastRound.dr, dl = lastRound.dl;
		lastRound.dr.permute(dx, params->xpn);

		const uint32 numLeftRounds = params->numRounds - rounds.getCount() - 1;
		const uint32 lookAhead = 1;

		uint32 r = 0; for (; r < Math::min(lookAhead, numLeftRounds); ++r)
		{
			BitArray dy(0), du(32);

			// For each sbox
			for (uint32 s = 0; s < 8; ++s)
			{
				const uint32 * row = params->difs[s] + (dx(s * 6, (s + 1) * 6) << 4);

				// Find max probability
				ubyte y = 0;
				uint32 max = row[0];
				for (uint32 j = 1; j < 16 && max < 64; ++j)
					if (row[j] > max)
						max = row[j], y = j << 4;
					
				// Append to dy
				dy.append(BitArray(&y, 4));

				// Update best probability
				h0 -= log2(max / 64.);
			}

			// Compute next round differentials
			du = dl;
			dl = dr;
			dy.permute(dr, params->perm) ^= du;
			dr.permute(dx, params->xpn);
		}

		for (; r < numLeftRounds; ++r)
			h0 += 2;

		h = h0;
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
	void expand_internal(List<Path> & out, const BitArray & dx, const BitArray & dy, float64 p0 = 1.f, uint32 s = 0) const
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
						expand_internal(out, dx, dy.merge(BitArray(&_y, 4)), p0 * p, s + 1);
					}
				}
			else
				expand_internal(out, dx, dy.merge(BitArray((const ubyte[]){0x0}, 4)), p0, s + 1);
		}
		else
		{
			// Copy path
			Path path(*this);

			// Update current round
			path.rounds.last()->dy = dy;

			// Compute next round
			BitArray u(32);
			path.rounds.push(RoundInstance{
				dr : dy.permute(u, params->perm) ^= path.rounds.last()->dl,
				dl : path.rounds.last()->dr
			});

			// Update path probability
			path.g += -log2(p0);
			path.computeH();

			// Move into output list
			out.push(move(path));
		}
	}

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
	void dfSearch_internal(Path & optimalPath, float64 & cost, const BitArray & dx, const BitArray & dy, float64 c0 = 0., uint32 s = 0)
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
						const ubyte _y = y << 4;
						dfSearch_internal(optimalPath, cost, dx, dy.merge(BitArray(&_y, 4)), c0 - log2(d / 64.), s + 1);
					}
				}
			else
				dfSearch_internal(optimalPath, cost, dx, dy.merge(BitArray((const ubyte[]){0x0}, 4)), c0, s + 1);
		}
		else
		{
			Path path(*this);

			// Update current round
			auto & lastRound = *path.rounds.last();
			lastRound.dy = dy;

			// Compute next round
			BitArray u(32);
			path.rounds.push(RoundInstance{
				dr : dy.permute(u, params->perm) ^= lastRound.dl,
				dl : lastRound.dr
			});

			// Update path probability
			path.g += c0;
			path.computeH();
			
			if (path.isComplete())
			{
				if (path.g < cost)
				{
					LOG(INFO, "optimal path cost: %f", path.g);

					cost = path.g;
					optimalPath = path;
				}
			}
			else if (path.getTotalCost() < cost)
				path.dfSearch(optimalPath, cost);
		}
	}

public:
	/**
	 * Performs a depth first search starting
	 * from this node
	 * 
	 * @param [out] optimalPath found path with minimum cost
	 * @param [out] cost minimum cost
	 */
	FORCE_INLINE void dfSearch(Path & optimalPath, float64 & cost)
	{
		LOG(INFO, "current cost: %f {depth: %llu}", cost, rounds.getCount() + 1ull);

		BitArray dx(48), dy(0);
		dfSearch_internal(optimalPath, cost, rounds.last()->dr.permute(dx, params->xpn), dy);
	}

protected:
	/**
	 * Internal sample code
	 * 
	 * @param [in] dx sbox differential input
	 * @param [in] p0 out differential probability
	 * @param [in] s simulated sbox index
	 */
	float64 sample_internal(BitArray & dy, const BitArray & dx, float64 p0 = 0., uint32 s = 0)
	{
		if (s < 8)
		{
			uint32 x = dx(s * 6, (s + 1) * 6);
			if (x != 0)
			{
				uint32 y = 0, d = 0;
				while (d == 0) // ! Assuming there's no null row
				{
					y = rand() & 0xf;
					d = params->difs[s][(x << 4) + y];
				}

				uint32 _y = y << 4;
				return sample_internal(dy.append(BitArray(&_y, 4)), dx, p0 - log2(d / 64.), s + 1);
			}
			else
				return sample_internal(dy.append(BitArray((const ubyte[]){0x0}, 4)), dx, p0, s + 1);
		}

		return p0;
	}

public:
	/**
	 * Simulate a random walk starting from this partial path
	 * 
	 * @return simualated path cost
	 */
	float64 sample()
	{
		// Sbox input
		BitArray dx(48), dr = rounds.last()->dr, dl = rounds.last()->dl;
		dr.permute(dx, params->xpn);

		// Number of rounds to simulate
		const uint32 numLeftRounds = params->numRounds - rounds.getCount() - 1;

		float64 g0 = 0.;
		for (uint32 r = 0; r < numLeftRounds; ++r)
		{
			// Sample sboxes output
			BitArray dy(0);
			g0 += sample_internal(dy, dx);

			// Compute next round
			BitArray t = dl;
			dl = dr;
			dy.permute(dr, params->perm) ^= t;
			dr.permute(dx, params->xpn);
		}

		return g + g0;
	}
};