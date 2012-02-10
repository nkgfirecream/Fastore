
#include <EASTL/internal/config.h>
#include <EASTL/internal/hashtable.h>
#include <EASTL/functional.h>
#include <functional>
#include <EASTL/utility.h>





template <typename Value, typename Allocator = EASTLAllocatorType, bool bCacheHashCode = false>
class fshash_set : public hashtable<Value, Value, Allocator,  eastl::use_self<Value>, std::function<bool(Value,Value)>, std::function<size_t(Value)>, 
			mod_range_hashing, default_ranged_hash, prime_rehash_policy, bCacheHashCode, false, true> 
{
	public:
		
		typedef hashtable<Value, Value, Allocator, eastl::use_self<Value>, std::function<bool(Value,Value)>, std::function<size_t(Value)>, 
			mod_range_hashing, default_ranged_hash, prime_rehash_policy, bCacheHashCode, false, true>        base_type;

	public:	
		fshash_set(const std::function<size_t(Value)> hash, const std::function<bool(Value,Value)> pred, const allocator_type& allocator = EASTL_HASH_SET_DEFAULT_ALLOCATOR)
		: base_type(0, hash, mod_range_hashing(), default_ranged_hash(), pred, eastl::use_self<Value>(), allocator)
	{
		// Empty
	}
 };