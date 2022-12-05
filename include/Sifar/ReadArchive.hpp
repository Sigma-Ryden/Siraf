#ifndef SIFAR_READ_ARCHIVE_HPP
#define SIFAR_READ_ARCHIVE_HPP

#include <cstdint> // uintptr_t
#include <cstddef> // size_t
#include <unordered_map> // unordered_map
#include <memory> // shared_ptr

#include <Sifar/ArchiveBase.hpp>

#include <Sifar/Access.hpp>
#include <Sifar/TypeRegistry.hpp>
#include <Sifar/Registry.hpp>

#include <Sifar/Utility.hpp>
#include <Sifar/DataTrackBase.hpp>

#include <Sifar/Detail/Meta.hpp>
#include <Sifar/Detail/MetaMacro.hpp>

#define CONDITIONAL_LOAD_SERIALIZABLE_TYPE(parameter, ...)                                              \
    template <class ReadArchive, typename T,                                                            \
              SIREQUIRE(::sifar::meta::is_read_archive<ReadArchive>() and                               \
                        ::sifar::meta::is_registered<T>() and                                           \
                        (bool)(__VA_ARGS__))>                                                           \
    ReadArchive& operator& (ReadArchive& archive, T& parameter)

#define LOAD_SERIALIZABLE_TYPE(parameter, ...)                                                          \
    template <class ReadArchive,                                                                        \
              SIREQUIRE(::sifar::meta::is_read_archive<WriteArchive>() and                              \
                        ::sifar::meta::is_registered<__VA_ARGS__>())>                                   \
    ReadArchive& operator& (ReadArchive& archive, __VA_ARGS__& parameter)

namespace sifar
{

namespace wrapper
{

template <typename InStream>
class InStreamWrapper
{
private:
    InStream& stream_;

public:
    InStreamWrapper(InStream& stream) : stream_(stream) {}

    template <typename T>
    void call(T* data, std::size_t memory_size)
    {
        stream_.read(utility::byte_cast(data), memory_size);
    }

    InStream& get() noexcept { return stream_; }
};

} // namespace wrapper

template <class InStream,
          class StreamWrapper = wrapper::InStreamWrapper<InStream>,
          class Registry = dynamic::ExternRegistry>
class ReadArchive : public core::ArchiveBase
{
    SERIALIZATION_ARCHIVE(ReadArchive)

public:
    struct Raw { void* address = nullptr; };
    struct Shared { std::shared_ptr<void> address = nullptr; };

public:
    using TrackingKeyType = std::uintptr_t;

    template <typename TrackData>
    using TrackingTable = std::unordered_map<TrackingKeyType, TrackData>;

private:
    StreamWrapper archive_;

    TrackingTable<Shared> track_shared_;
    TrackingTable<Raw> track_raw_;

    Registry registry_;

public:
    ReadArchive(InStream& stream);

    auto stream() noexcept -> StreamWrapper& { return archive_; }

    template <typename TrackType,
              SIREQUIRE(meta::is_track_shared<TrackType>())>
    auto tracking() noexcept -> TrackingTable<Shared>& { return track_shared_; }

    template <typename TrackType,
              SIREQUIRE(meta::is_track_raw<TrackType>())>
    auto tracking() noexcept -> TrackingTable<Raw>& { return track_raw_; }

    auto registry() noexcept -> Registry& { return registry_; }

    template <typename T>
    auto operator>> (T&& data) -> ReadArchive&;

    template <typename T, typename... Tn>
    auto operator() (T& data, Tn&... data_n) -> ReadArchive&;

    auto operator() () -> ReadArchive& { return *this; }
};

template <class InStream>
ReadArchive<InStream> reader(InStream& stream)
{
    return { stream };
}

template <template <class, typename...> class StreamWrapper,
          class Registry = dynamic::ExternRegistry,
          class InStream>
ReadArchive<InStream, StreamWrapper<InStream>, Registry> reader(InStream& stream)
{
    return { stream };
}

template <class StreamWrapper,
          class Registry = dynamic::ExternRegistry,
          class InStream>
ReadArchive<InStream, StreamWrapper, Registry> reader(InStream& stream)
{
    return { stream };
}

template <class InStream, class StreamWrapper, class Registry>
ReadArchive<InStream, StreamWrapper, Registry>::ReadArchive(InStream& stream)
    : archive_{stream}, track_shared_(), track_raw_(), registry_()
{
}

template <class InStream, class StreamWrapper, class Registry>
template <typename T>
auto ReadArchive<InStream, StreamWrapper, Registry>::operator>> (
    T&& data) -> ReadArchive&
{
    return (*this) & data;
}

template <class InStream, class StreamWrapper, class Registry>
template <typename T, typename... Tn>
auto ReadArchive<InStream, StreamWrapper, Registry>::operator() (
    T& data, Tn&... data_n) -> ReadArchive&
{
    (*this) & data;

    return (*this)(data_n...);
}

template <class ReadArchive, typename T,
          SIREQUIRE(meta::is_read_archive<ReadArchive>()
                    and meta::is_unsupported<T>())>
ReadArchive& operator& (ReadArchive& archive, T& unsupported)
{
    static_assert(meta::to_false<T>(),
        "'T' is an unsupported type for the 'sifar::ReadArchive'.");

    return archive;
}

template <class ReadArchive, typename T,
          SIREQUIRE(meta::is_read_archive<ReadArchive>()
                    and not meta::is_registered<T>())>
ReadArchive& operator& (ReadArchive& archive, T& unregistered)
{
    static_assert(meta::to_false<T>(),
        "'T' is an unregistered type for the 'sifar::ReadArchive'. "
        "Try overload an operator& to serialize the type 'T' with the macro "
        "'SERIALIZATION_LOAD_DATA(parameter, condition)' "
        "and then register the type 'T' with the macros: "
        "'SERIALIZATION_TYPE_REGISTRY(name)' or "
        "'SERIALIZATION_TYPE_REGISTRY_IF(condition)'.");

    return archive;
}

} // namespace sifar

#endif // SIFAR_READ_ARCHIVE_HPP
