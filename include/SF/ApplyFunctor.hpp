#ifndef SF_APPLY_FUNCTOR_HPP
#define SF_APPLY_FUNCTOR_HPP

#include <SF/Core/Serialization.hpp>

#include <SF/Detail/Meta.hpp>
#include <SF/Detail/MetaMacro.hpp>

namespace sf
{

namespace apply
{

struct apply_functor_t {};

} // namespace apply

namespace meta
{

template <typename ApplyFunctorType> struct is_apply_functor : std::is_base_of<apply::apply_functor_t, ApplyFunctorType> {};

} // namespace meta

} // namespace sf

CONDITIONAL_SERIALIZATION(saveload, apply_functor, ::sf::meta::is_apply_functor<typename std::decay<S>::type>::value)
{
    apply_functor(archive);
}

#endif // SF_APPLY_FUNCTOR_HPP
