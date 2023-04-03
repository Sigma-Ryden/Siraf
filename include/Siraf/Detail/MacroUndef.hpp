// If you want to undef all external lib's macros
// you should include given file only after library files
// note that: MACRO - is public macro, _MACRO - is private

// clean up
#undef SERIALIZATION_ACCESS

#undef SERIALIZATION_ARCHIVE
#undef EXPORT_SERIALIZATION_ARCHIVE

#undef INSTANTIABLE_BODY
#undef INSTANTIABLE_TYPE

#undef SRIALIZABLE

#undef EXPORT_POLYMORPHIC_KEY
#undef EXPORT_POLYMORPHIC

#undef EXTERN_CONDITIONAL_SERIALIZATION
#undef EXTERN_SERIALIZATION

#undef CONDITIONAL_SERIALIZATION
#undef SERIALIZATION

#undef CONDITIONAL_TYPE_REGISTRY
#undef TYPE_REGISTRY

#undef SIRAF_STATIC_HASH
#undef SIRAF_STATIC_HASH_KEY_TYPE
#undef SIRAF_TYPE_HASH

#undef SIRAF_MAX_TEMPLATE_DEPTH
#undef SIRAF_ARCHIVE_MAX_TRAIT_KEY
#undef SIRAF_BYTE_STREAM_RESERVE_SIZE

// possible to undef
// #undef SIRAF_NULLPTR_DISABLE
// #undef SIRAF_PTRTRACK_DISABLE
// #undef SIRAF_TYPE_REGISTRY_DISABLE
// #undef SIRAF_EXPORT_POLYMORPHIC_DISABLE
// #undef SIRAF_DEFAULT_DISABLE

#undef SIREQUIRE
#undef SIWHEN
#undef SIWITH
