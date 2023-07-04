// Note: An arena could technically also be called a memory resource or an allocator,
// but those terms will be used to refer to abstractions from the standard library.

#include <cstddef>
#include <memory>
#include <vector>
#include <set>
#include <array>
#include <iostream>
#include <memory_resource>

template <size_t N>
class Arena
{
    static constexpr size_t alignment = alignof( std::max_align_t );
public:
    Arena() noexcept : ptr_( buffer_ )
    {}
    Arena( const Arena& ) = delete;
    Arena& operator=( const Arena& ) = delete;

    auto reset() noexcept
    {
        ptr_ = buffer_;
    }
    static constexpr auto size() noexcept
    {
        return N;
    }
    auto used() const noexcept
    {
        return static_cast<size_t>( ptr_ - buffer_ );
    }
    auto allocate( size_t n ) -> std::byte*;
    auto deallocate( std::byte* p, size_t n ) noexcept -> void;

private:
    static auto align_up( size_t n ) noexcept -> size_t
    {
        return ( n + ( alignment - 1 ) ) & ~( alignment - 1 );
    }
    auto pointer_in_buffer( const std::byte* p ) const noexcept -> bool
    {
        return std::uintptr_t( p ) >= std::uintptr_t( buffer_ ) &&
            std::uintptr_t( p ) < std::uintptr_t( buffer_ ) + N;
    }
    alignas( alignment ) std::byte buffer_[N];
    std::byte* ptr_{};
};

template<size_t N>
auto Arena<N>::allocate( size_t n ) -> std::byte*
{
    const auto aligned_n = align_up( n );
    const auto available_bytes = static_cast<decltype( aligned_n )>( buffer_ + N - ptr_ );
    if ( available_bytes >= aligned_n )
    {
        auto* r = ptr_;
        ptr_ += aligned_n;
        return r;
    }
    return static_cast<std::byte*>( ::operator new( n ) );
}

template<size_t N>
auto Arena<N>::deallocate( std::byte* p, size_t n ) noexcept -> void
{
    if ( pointer_in_buffer( p ) )
    {
        n = align_up( n );
        if ( p + n == ptr_ )
        {
            ptr_ = p;
        }
    }
    else
    {
        ::operator delete( p );
    }
}

auto user_arena = Arena<1024>{};

class CUser
{
public:

    auto operator new( size_t size ) -> void*
    {
        return user_arena.allocate( size );
    }

    auto operator delete( void* p ) -> void
    {
        user_arena.deallocate( static_cast<std::byte*>( p ), sizeof( CUser ) );
    }

    auto operator new[] ( size_t size ) -> void*
    {
        return user_arena.allocate( size );
    }

    auto operator delete[] ( void* p, size_t size ) -> void
    {
        user_arena.deallocate( static_cast<std::byte*>( p ), size );
    }
private:
    int id_{};
};

// A stateless memory allocator
template <class T>
struct Mallocator
{
    using value_type = T;
    Mallocator() = default;

    template <class U>
    Mallocator( const Mallocator<U>& ) noexcept
    {}

    template <class U>
    auto operator==( const Mallocator<U>& ) const noexcept
    {
        return true;
    }

    template <class U>
    auto operator!=( const Mallocator<U>& ) const noexcept
    {
        return false;
    }

    auto allocate( size_t n ) const -> T*
    {
        if ( n == 0 )
        {
            return nullptr;
        }
        if ( n > std::numeric_limits<size_t>::max() / sizeof( T ) )
        {
            throw std::bad_array_new_length{};
        }
        void* const pv = malloc( n * sizeof( T ) );
        if ( pv == nullptr )
        {
            throw std::bad_alloc{};
        }
        return static_cast<T*>( pv );
    }
    auto deallocate( T* p, size_t ) const noexcept -> void
    {
        free( p );
    }
};

// A stateful allocator
template <class T, size_t N>
struct ShortAlloc
{
    using value_type = T;
    using arena_type = Arena<N>;

    ShortAlloc( const ShortAlloc& ) = default;
    ShortAlloc& operator=( const ShortAlloc& ) = default;

    ShortAlloc( arena_type& arena ) noexcept : arena_{ &arena }
    {}

    template <class U>
    ShortAlloc( const ShortAlloc<U, N>& other ) noexcept
        : arena_{ other.arena_ }
    {}

    template <class U>
    struct rebind
    {
        using other = ShortAlloc<U, N>;
    };

    auto allocate( size_t n ) -> T*
    {
        return reinterpret_cast<T*>( arena_->allocate( n * sizeof( T ) ) );
    }
    auto deallocate( T* p, size_t n ) noexcept -> void
    {
        arena_->deallocate( reinterpret_cast<std::byte*>( p ), n * sizeof( T ) );
    }
    template <class U, size_t M>
    auto operator==( const ShortAlloc<U, M>& other ) const noexcept
    {
        return N == M && arena_ == other.arena_;
    }
    template <class U, size_t M>
    auto operator!=( const ShortAlloc<U, M>& other ) const noexcept
    {
        return !( *this == other );
    }
    template <class U, size_t M> friend struct ShortAlloc;
private:
    arena_type* arena_;
};

template <typename T>
using SmallVector = std::vector<T, ShortAlloc<T, 512>>;

template <typename T>
using AscendingSmallSet = std::set<T, std::less<T>, ShortAlloc<T, 512>>;
template <typename T>
using DescendingSmallSet = std::set<T, std::greater<T>, ShortAlloc<T, 512>>;

class PrintingResource : public std::pmr::memory_resource
{
public:
    PrintingResource() : res_{ std::pmr::get_default_resource() }
    {}
private:
    virtual void* do_allocate( std::size_t bytes, std::size_t alignment ) override
    {
        std::cout << "allocate: " << bytes << '\n';
        return res_->allocate( bytes, alignment );
    }
    virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override
    {
        std::cout << "deallocate: " << bytes << '\n';
        return res_->deallocate( p, bytes, alignment );
    }
    virtual bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept override
    {
        return ( this == &other );
    }
    std::pmr::memory_resource* res_;  // Default resource
};

void CustomAllocator()
{
    // No dynamic memory is allocated when we create the users 
    auto user1 = new CUser{};
    delete user1;

    auto users = new CUser[100];
    delete[] users;

    auto user2 = std::make_unique<CUser>();

    // we can _NOT_ use our custom memory allocator, since shared_ptr need more memory to store a CUser
    auto user = std::make_shared<CUser>();

    // Ditto
    auto userss = std::vector<CUser>{};
    userss.reserve( 10 );

    // Instead, we use our own allocator
    // No dynamic memory allocation!
    auto stack_arena = SmallVector<CUser>::allocator_type::arena_type{};
    auto cusers = SmallVector<CUser>{ stack_arena };
    cusers.reserve( 10 );

    std::cout << sizeof( std::vector<int> ) << '\n';
    // Possible output: 32
    std::cout << sizeof( SmallVector<int> ) << '\n';
    // Possible output: 40

    std::cout << "\n//-------------------------------------------------------------------------\n\n";

    auto v1 = std::vector<int>{};            // Uses std::allocator
    auto v2 = std::pmr::vector<int>{/*...*/ }; // Uses polymorphic_allocator

    // base class
    // std::pmr::memory_resource

    // std::pmr::monotonic_buffer_resource
    // Used in short lifetime objects, it will free the memory only when the memory resource instance is destroyed.

    // std::pmr::unsynchronized_pool_resource (something like monotonic link-list allocator)
    // Contians fixed size of memory blocks, can be used to avoid memory fragmentation within each pool. (Not thread safe)

    // std::pmr::synchronized_pool_resource
    // Thread safe version of std::pmr::unsynchronized_pool_resource.

    // Memory resources can be chained. When creating an instance of a memory resource, we can provide it with an upstream memory resource.

    std::cout << "\n//-------------------------------------------------------------------------\n\n";

    // same as our Arena
    auto buffer = std::array<std::byte, 512>{};
    auto resource = std::pmr::monotonic_buffer_resource{ buffer.data(), buffer.size(), std::pmr::new_delete_resource() }; // specified a upstream memory resource
    auto unique_numbers = std::pmr::set<int>{ &resource };
    auto n = int{};
    while ( std::cin >> n )
    {
        unique_numbers.insert( n );
    }
    for ( const auto& number : unique_numbers )
    {
        std::cout << number << '\n';
    }

    auto res = PrintingResource{};
    auto vec = std::pmr::vector<int>{ &res };
    vec.emplace_back( 1 );
    vec.emplace_back( 2 );
}