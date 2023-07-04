#include <cassert>
#include <iostream>

class OldVec3
{
public:

	OldVec3( float x, float y, float z )
	{
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		std::cout << "OldVec3 constructed!\n";
	}

	auto& operator[]( size_t index )
	{
		assert( index < 3 );
		return m_val[index];
	}

	auto operator[]( size_t index ) const
	{
		assert( index < 3 );
		return m_val[index];
	}

	friend auto operator+( OldVec3 const& lhs, OldVec3 const& rhs )
	{
		return OldVec3( lhs.m_val[0] + rhs.m_val[0], lhs.m_val[1] + rhs.m_val[1], lhs.m_val[2] + rhs.m_val[2] );
	}

	auto size() const -> size_t
	{
		return 3;
	}

private:

	float		m_val[3];
};

// CRTP
// By using this compile expression class, we recursively expand all expression at compile.
template <typename T>
class VecExpression
{
public:
	// leaf element of the expression must be operands, _NOT_ the operator!!
	constexpr static bool IS_LEAF = false;

	// static polymorphism to avoid runtime vtable
	auto& operator[]( size_t index )
	{
		return static_cast<T&>(*this)[index];
	}

	auto operator[]( size_t index ) const
	{
		return static_cast<T const&>( *this )[index];
	}

	auto size() const -> size_t
	{
		return static_cast<T const&>( *this ).size();
	}
};

class NewVec3 : public VecExpression<NewVec3>
{
public:

	// this is a operands
	constexpr static bool IS_LEAF = true;

public:

	NewVec3( float x, float y, float z )
	{
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		std::cout << "NewVec3 constructed!\n";
	}

	// any vec expression can be converted back to Vec3
	template <typename T>
	NewVec3( VecExpression<T> const& expr )
	{
		for ( auto i = 0; i < expr.size(); ++i )
		{
			m_val[i] = expr[i];
		}
		std::cout << "NewVec3 constructed (by expression)!\n";
	}

	auto& operator[]( size_t index )
	{
		assert( index < 3 );
		return m_val[index];
	}

	auto operator[]( size_t index ) const
	{
		assert( index < 3 );
		return m_val[index];
	}

	auto size() const -> size_t
	{
		return 3;
	}

private:

	float				m_val[3];
};

// Notice that by passing class itself to the template parameter of VecExpression<>,
// we abstract the 'indexing' and 'size query' operation at compile time.
// Then we can expand this expression at compile time to get rid of any other unnecessary runtime performance penalty.
template <typename LHS, typename RHS>
class VecSumation : public VecExpression<VecSumation<LHS, RHS>>
{

	using LhsType = typename std::conditional<LHS::IS_LEAF, LHS const&, LHS const>::type;
	using RhsType = typename std::conditional<RHS::IS_LEAF, RHS const&, RHS const>::type;

public:

	constexpr static bool IS_LEAF = false;

public:

	VecSumation( LHS const& lhs, RHS const& rhs )
		: m_lhs( lhs ), m_rhs( rhs )
	{
		assert( m_lhs.size() == m_rhs.size() );
	}

	auto& operator[]( size_t index )
	{
		assert( index < 3 );
		return m_lhs[index] + m_rhs[index];
	}

	auto operator[]( size_t index ) const
	{
		assert( index < 3 );
		return m_lhs[index] + m_rhs[index];
	}

	auto size() const -> size_t
	{
		return m_rhs.size();
	}

private:

	LhsType			m_lhs;
	RhsType			m_rhs;
};

// Here, we can treat VecSumation<LHS, RHS> as a proxy object used to postpond the actual expression expandation.
template <typename LHS, typename RHS>
VecSumation<LHS, RHS> operator+( VecExpression<LHS> const& lhs, VecExpression<RHS> const& rhs )
{
	return VecSumation<LHS, RHS>( *static_cast<const LHS*>( &lhs ), *static_cast<const RHS*>( &rhs ) );
}

void ExpressionTemplate()
{
	{
		auto vec0 = OldVec3{ 1.2f, 20.5f, 1.4f };
		auto vec1 = OldVec3{ 1.7f, 10.5f, 4.8f };
		auto vec2 = OldVec3{ 10.2f, 2.5f, 1.4f };
		auto vec3 = OldVec3{ 1.2f, 2.5f, 20.4f };
		auto vec4 = OldVec3{ 10.2f, 2.8f, 1.7f };

		std::cout << "\n//-------------------------------------------------------------------------\n\n";

		// OldVec3 construct four time!! what about sum of 100 vec? 99 OldVec3 constructed!
		// we want to construct a single expression in compile time by using technique called expression template
		OldVec3 sum = vec0 + vec1 + vec2 + vec3 + vec4;
		// this expand to OldVec3(OldVec3(OldVec3(OldVec3(OldVec3(OldVec3 + OldVec3) + OldVec3) + OldVec3) + OldVec3) + OldVec3)

		std::cout << "\nSumation: (" << sum[0] << ", " << sum[1] << ", " << sum[2] << ")\n";
		std::cout << "\n";
	}

	std::cout << "\n//-------------------------------------------------------------------------\n\n";

	//-------------------------------------------------------------------------

	{
		auto vec0 = NewVec3{ 1.2f, 20.5f, 1.4f };
		auto vec1 = NewVec3{ 1.7f, 10.5f, 4.8f };
		auto vec2 = NewVec3{ 10.2f, 2.5f, 1.4f };
		auto vec3 = NewVec3{ 1.2f, 2.5f, 20.4f };
		auto vec4 = NewVec3{ 10.2f, 2.8f, 1.7f };

		std::cout << "\n//-------------------------------------------------------------------------\n\n";

		// only one VecExpression to NewVec3 conversion
		NewVec3 sum = vec0 + vec1 + vec2 + vec3 + vec4;
		// this expand to NewVec3( VecExpression(NewVec3 + NewVec3 + NewVec3 + NewVec3 + NewVec3) )

		std::cout << "\nSumation: (" << sum[0] << ", " << sum[1] << ", " << sum[2] << ")\n";
	}
}