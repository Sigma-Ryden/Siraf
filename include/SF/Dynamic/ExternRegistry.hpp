#ifndef SF_DYNAMIC_REGISTRY_HPP
#define SF_DYNAMIC_REGISTRY_HPP

#include <SF/Core/Serialization.hpp>
#include <SF/Core/Memory.hpp>

#include <SF/Dynamic/InstantiableRegistry.hpp>
#include <SF/Dynamic/InstantiableTraits.hpp>
#include <SF/Dynamic/AnyRegistry.hpp>

#include <SF/Detail/Meta.hpp>
#include <SF/Detail/MetaMacro.hpp>

namespace sf
{

namespace dynamic
{

class extern_registry_t
{
public:
    template <class ArchiveType, typename PointerType,
              SF_REQUIRES(meta::is_pointer_to_polymorphic<PointerType>::value)>
    static ::xxsf_instantiable_traits_key_type save_key(ArchiveType& archive, PointerType& pointer)
    {
        if (pointer == nullptr)
            throw "The write pointer was not allocated.";

        auto const hash = SF_EXPRESSION_HASH(*pointer);

        auto key = instantiable_registry_t::instance().rtti_all.at(hash).key;
        archive & key;

        return key;
    }

    template <class ArchiveType, typename PointerType,
              SF_REQUIRES(meta::is_pointer_to_polymorphic<PointerType>::value)>
    static ::xxsf_instantiable_traits_key_type load_key(ArchiveType& archive, PointerType& pointer)
    {
    #ifndef SF_GARBAGE_CHECK_DISABLE
        if (pointer != nullptr)
            throw "The read pointer must be initialized to nullptr.";
    #endif // SF_GARBAGE_CHECK_DISABLE

        ::xxsf_instantiable_traits_key_type key{};
        archive & key;

        return key;
    }

private:
    template <typename PointerType> struct is_pointer_to_instantiable
        : meta::all<instantiable_registry_t::is_instantiable<typename memory::ptr_traits<PointerType>::item>,
                    meta::is_pointer_to_polymorphic<PointerType>> {};

public:
    template <typename PointerType,
              SF_REQUIRES(is_pointer_to_instantiable<PointerType>::value)>
    static void save(core::ioarchive_t& archive, PointerType& pointer, ::xxsf_instantiable_traits_key_type key)
    {
        if (pointer == nullptr)
            throw "The write pointer was not allocated.";

        auto raw_pointer = memory::raw(pointer);
        instantiable_registry_t::instance().save(archive, raw_pointer);
    }

    template <typename PointerType,
              SF_REQUIRES(meta::is_pointer_to_polymorphic<PointerType>::value)>
    static void load(core::ioarchive_t& archive, PointerType& pointer, ::xxsf_instantiable_traits_key_type key, memory::void_ptr<PointerType>& cache)
    {
        using PointerTraitsType = memory::ptr_traits<PointerType>;

    #ifndef SF_GARBAGE_CHECK_DISABLE
        if (pointer != nullptr)
            throw "The read pointer must be initialized to nullptr.";
    #endif // SF_GARBAGE_CHECK_DISABLE

        auto& registry = instantiable_registry_t::instance();

        auto cloned = registry.clone<typename PointerTraitsType::traits>(key);

        pointer = memory::dynamic_pointer_cast<typename PointerTraitsType::item>(cloned);
        cache = memory::pure(pointer);

        auto raw_pointer = memory::raw(pointer);
        registry.load(archive, raw_pointer);
    }

    template <typename PointerType,
              SF_REQUIRES(meta::is_pointer_to_polymorphic<PointerType>::value)>
    static void assign(PointerType& pointer, memory::void_ptr<PointerType> const& address, ::xxsf_instantiable_traits_key_type key)
    {
    #ifndef SF_GARBAGE_CHECK_DISABLE
        if (pointer != nullptr)
            throw "The read pointer must be initialized to nullptr.";
    #endif // SF_GARBAGE_CHECK_DISABLE

        auto casted = instantiable_registry_t::instance().cast(address, key);
        pointer = memory::dynamic_pointer_cast<memory::ptr_traits<PointerType>::item>(casted);
    }
};

} // namespace dynamic

} // namespace sf

#endif // SF_DYNAMIC_REGISTRY_HPP

