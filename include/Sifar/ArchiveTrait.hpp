#ifndef SIFAR_ARCHIVE_TRAIT_HPP
#define SIFAR_ARCHIVE_TRAIT_HPP

#include <Sifar/ArchiveBase.hpp>

#define _EXPORT_SERIALIZATION_ARCHIVE_TYPE_TRAIT(archive_type, ...)                                     \
    namespace sifar { namespace core {                                                                  \
        template <> struct archive_type##Trait<ArchiveTraitKey<archive_type<__VA_ARGS__>>::key>         \
        { using type = archive_type<__VA_ARGS__>; };                                                    \
    }}

#define _EXPORT_SERIALIZATION_ARCHIVE_TRAIT_KEY(archive_key, archive_type, ...)                         \
    namespace sifar { namespace core {                                                                  \
        template <> struct ArchiveTraitKey<archive_type<__VA_ARGS__>>                                   \
        { static constexpr ArchiveCore::key_type key = archive_key; };                                  \
    }}

#define EXPORT_SERIALIZATION_ARCHIVE(archive_key, archive_type, ...)                                    \
    _EXPORT_SERIALIZATION_ARCHIVE_TRAIT_KEY(archive_key, archive_type, __VA_ARGS__)                     \
    _EXPORT_SERIALIZATION_ARCHIVE_TYPE_TRAIT(archive_type, __VA_ARGS__)

namespace sifar
{

namespace core
{

template <ArchiveCore::key_type I> struct ReadArchiveTrait { using type = ArchiveBase; };
template <ArchiveCore::key_type I> struct WriteArchiveTrait { using type = ArchiveBase; };

} // namespace core

} // namespace sifar

#endif // SIFAR_ARCHIVE_TRAIT_HPP
