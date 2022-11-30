#ifndef SIFAR_DATA_TRACK_HPP
#define SIFAR_DATA_TRACK_HPP

#include <memory> // addressof

#include <Sifar/DataTrackBase.hpp>

#include <Sifar/NativeSaveLoad.hpp>

#include <Sifar/ApplyFunctor.hpp>

#include <Sifar/Strict.hpp>

#include <Sifar/Detail/Meta.hpp>
#include <Sifar/Detail/MetaMacro.hpp>

namespace sifar
{


namespace tracking
{

template <class WriteArchive, typename T,
          SIREQUIRE(meta::is_write_archive<WriteArchive>()
                    and meta::is_pointer<T>())>
void track(WriteArchive& archive, T& pointer)
{
    using key_type = typename WriteArchive::TrackingKeyType;

    auto success = detail::is_refer(archive, pointer); // serialize refer info
    if (not success) return;

    auto pure = memory::pure(pointer);
    auto key = reinterpret_cast<key_type>(memory::raw(pure));

    auto& is_tracking = archive.tracking()[key];

    if (not is_tracking)
    {
        archive & key;
        strict(archive, pointer); // call the strict serialization of not tracking pointer

        is_tracking = true;
    }
    else
    {
        detail::native_save(archive, pointer, key);
    }
}

template <class WriteArchive, typename T,
          SIREQUIRE(meta::is_write_archive<WriteArchive>()
                    and not meta::is_pointer<T>())>
void track(WriteArchive& archive, T& data)
{
    using key_type = typename WriteArchive::TrackingKeyType;

    auto address = memory::pure(std::addressof(data));
    auto key = reinterpret_cast<key_type>(address);

    auto& is_tracking = archive.tracking()[key];

    if (is_tracking)
        throw "the write tracking data is already tracked.";

    is_tracking = true;

    archive & key;
    archive & data;
}

template <class ReadArchive, typename T,
          SIREQUIRE(meta::is_read_archive<ReadArchive>()
                    and meta::is_pointer<T>())>
void track(ReadArchive& archive, T& pointer)
{
    using key_type   = typename ReadArchive::TrackingKeyType;
    using track_type = typename tracking::track_trait<T>::trait;

    if (pointer != nullptr)
        throw "the read track pointer must be initialized to nullptr.";

    auto success = detail::is_refer(archive, pointer); // serialize refer info
    if (not success) return;

    key_type key;
    archive & key;

    auto& track_data = archive.template tracking<track_type>()[key];

    if (not track_data.is_tracking)
    {
        strict(archive, pointer); // call the strict serialization of not tracking pointer

        track_data.address = memory::pure(pointer);
        track_data.is_tracking = true;
    }
    else
    {
        detail::native_load(archive, pointer, track_data.address);
    }
}

template <class ReadArchive, typename T,
          SIREQUIRE(meta::is_read_archive<ReadArchive>()
                    and not meta::is_pointer<T>())>
void track(ReadArchive& archive, T& data)
{
    using key_type   = typename ReadArchive::TrackingKeyType;
    using track_type = tracking::Raw;

    key_type key;
    archive & key;

    auto& track_data = archive.template tracking<track_type>()[key];

    if (track_data.is_tracking)
        throw  "the read tracking data is already tracked.";

    auto address = memory::pure(std::addressof(data));

    track_data.address = address;
    track_data.is_tracking = true;

    archive & data;
}

} // namespace traking

namespace apply
{

template <typename T>
class TrackFunctor : public ApplyFunctor
{
private:
    T& parameter_;

public:
    TrackFunctor(T& parameter) noexcept : parameter_(parameter) {}

    template <typename Archive, SIREQUIRE(meta::is_archive<Archive>())>
    void operator() (Archive& archive)
    {
        ::sifar::tracking::track(archive, parameter_);
    }
};

} // namespace apply

namespace tracking
{

template <typename T>
apply::TrackFunctor<T> track(T& parameter) { return { parameter }; }

} // namespace tracking

} // namespace sifar

#endif // SIFAR_DATA_TRACK_HPP
