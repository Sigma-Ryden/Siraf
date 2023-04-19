#include "SFTestingBase.hpp"

#include <SF/Support/shared_ptr.hpp>

TEST(TestLibrary, TestValidation)
{
    using sf::tracking::track;

    static int s_i = 745; // some garbage value

    std::vector<unsigned char> storage;
    {
        auto ar = oarchive(storage);

        int j = s_i;
        int* p_j = &j;

        ar & p_j;

        auto success = false;
        try { ar & track(j); } catch(...) { success = true; }

        EXPECT("bad data track", success);
    }

    storage.clear();
    {
        auto ar = oarchive(storage);

        int* g = nullptr; // ok - nullptr allowed
        ar & g;
    }
    {
        auto ar = iarchive(storage);

        int v = s_i;
        int* g = &v; // bad - should be nullptr

        auto success = false;
        try { ar & g; } catch(...) { success = true; }

        EXPECT("garbage ptr.read", success);
    }

    storage.clear();
    {
        auto ar = oarchive(storage);

        int* m = new int(s_i);

        int* r = m; // good
        std::shared_ptr<int> s(m); // bad - mixed tracking

        ar & r;

        auto success = false;
        try { ar & s; } catch(...) { success = true; }

        EXPECT("mixed track.raw shared", success);
    }

    storage.clear();
    {
        auto ar = oarchive(storage);

        int* m = new int(s_i);

        std::shared_ptr<int> s(m); // god
        int* r = m; // bad - mixed tracking

        ar & s;

        auto success = false;
        try { ar & r; } catch(...) { success = true; }

        EXPECT("mixed track.shared raw", success);
    }
}

TEST_MODULE()
{

class Triangle : public Instantiable
{
    SERIALIZABLE(Triangle)
};

class Cyrcle : public Instantiable
{
    SERIALIZABLE(Triangle) // oops - polymorphic key is already use
};

// is the same as below
//EXPORT_INSTANTIABLE_KEY("Triangle", Triangle) // good
//EXPORT_INSTANTIABLE_KEY("Triangle", Cyrcle) // bad - same keys

class Square // non instantiable
{
    SERIALIZABLE(Square)
};

} // TEST_MODULE

TEST(TestLibrary, TestInstantiableRegistry)
{
    using sf::dynamic::InstantiableFixture;

    {
        InstantiableFixture<Triangle> triangle;

        auto success = false;
        try { InstantiableFixture<Cyrcle> cyrcle; } catch(...) { success = true; }

        EXPECT("same polymorphic key", success);
    }

    using sf::dynamic::InstantiableRegistry;
    using sf::dynamic::ExternRegistry;

    auto& registry = InstantiableRegistry::instance();
    auto key = Serialization::trait<Square>();

    {
        auto success = false;
        try { registry.update<Square>(key); } catch(...) { success = true; }

        EXPECT("non-instantiable type", success);
    }

    using sf::Memory;

    {
        auto success = false;
        try { registry.clone<Memory::Raw>(key); } catch(...) { success = true; }

        EXPECT("clone non-instantiable raw", success);
    }
    {
        auto success = false;
        try { registry.clone<Memory::Shared>(key); } catch(...) { success = true; }

        EXPECT("clone non-instantiable shared", success);
    }

    {
        auto r = new Square;

        auto success = false;
        try { (void)registry.cast(r, key); } catch(...) { success = true; }

        EXPECT("cast non-instantiable raw", success);

        delete r; // clean up
    }
    {
        auto s = std::make_shared<Square>();

        auto success = false;
        try { (void)registry.cast(s, key); } catch(...) { success = true; }

        EXPECT("cast non-instantiable shared", success);
    }
}

TEST_MODULE()
{

struct MyStruct : Instantiable
{
    SERIALIZABLE(MyStruct)
};

class MyClass : sf::Instantiable // is same as Instantiable
{
    SERIALIZABLE(MyClass)
};

class MyDerivedClass : public MyClass
{
    SERIALIZABLE(MyDerivedClass)
};

struct MyCustomType : Instantiable
{
    SERIALIZABLE(MyCustomType)
};

} // TEST_MODULE

EXPORT_INSTANTIABLE_KEY("MyClass", MyStruct)
EXPORT_INSTANTIABLE_KEY("MyStruct", MyClass)

EXPORT_INSTANTIABLE_KEY("MyDerived", MyDerivedClass)

EXPORT_INSTANTIABLE(MyCustomType)

TEST(TestLibrary, TestExportInstantiable)
{
    static auto sv_s = SF_STATIC_HASH("MyClass");
    static auto sv_c = SF_STATIC_HASH("MyStruct");

    {
        EXPECT("export instantiable key.trait",
            Serialization::trait<MyStruct>() == sv_s &&
            Serialization::trait<MyClass>() == sv_c);

        EXPECT("export instantiable key.static_trait",
            Serialization::static_trait<MyStruct>() == sv_c &&
            Serialization::static_trait<MyClass>() == sv_s);
    }

    using sf::dynamic::ExternRegistry;

    static auto sv_ct = SF_STATIC_HASH("MyCustomType");

    {
        EXPECT("export instantiable.equivalent",
            Serialization::trait<MyCustomType>() == sv_ct &&
            Serialization::static_trait<MyCustomType>() == sv_ct);
    }

    static auto sv_dc = SF_STATIC_HASH("MyDerived");

    {
        std::shared_ptr<MyClass> b = std::make_shared<MyDerivedClass>();
        EXPECT("instantiable runtime key.trait", Serialization::trait(*b) == sv_dc);
    }
}

#include <SF/Support/string.hpp>

TEST(TestLibrary, TestStreamWrapper)
{
    static std::string s_s = "Hello, World!";

    std::vector<char> storage;
    {
        std::string s = s_s;

        auto ar = oarchive(storage);
        ar & s;
    }
    {
        std::string s;

        auto ar = iarchive(storage);
        ar & s;

        EXPECT("char storage", s == s_s);
    }

    storage.clear();
    {
        std::string s = s_s;

        auto ar = oarchive(storage);
        ar & s;

        std::ofstream file("test.bin", std::ios::binary);

        // if we use unsigned char - we should use reinterpret_cast to const char
        // or 'sf::memory::const_byte_cast' function
        file.write(storage.data(), storage.size());

        file.close();
    }
    {
        std::string s;

        std::ifstream file("test.bin", std::ios::binary);

        auto ar = iarchive<sf::wrapper::IFileStream>(file);
        ar & s;

        EXPECT("storage converting", s == s_s);
    }
}

// if you won't serialize object just omit SERIlALIZABLE(type)
// but if you are going to serialize it in the future -
// write SERIALIZABLE(type) and omit SERIALIZATION(mode, type)

TEST_MODULE()
{

struct WorldObject : Instantiable
{
    SERIALIZABLE(WorldObject)
    int wo = 0;
};

struct EnvironmentObject : virtual WorldObject
{
    SERIALIZABLE(EnvironmentObject)
    int eo = 0;
};

struct MoveableObject : virtual EnvironmentObject
{
    SERIALIZABLE(MoveableObject)
    int mo = 0;
};

struct DestructibleObject : virtual EnvironmentObject
{
    SERIALIZABLE(DestructibleObject)
    int dso = 0;
};

struct DecorativeObject : DestructibleObject, MoveableObject
{
    SERIALIZABLE(DecorativeObject)
    int dco = 0;
};

struct FoliageObject : virtual WorldObject
{
    SERIALIZABLE(FoliageObject)
    int fo = 0;
};

struct FoliageObjectInstance : FoliageObject {};

struct DecorativeFoliageObject : DecorativeObject, FoliageObjectInstance
{
    SERIALIZABLE(DecorativeFoliageObject)
    int dcfo = 0;
};

} // TEST_MODULE

SERIALIZATION(SaveLoad, WorldObject)
{
    ++self.wo;
}

SERIALIZATION(SaveLoad, EnvironmentObject)
{
    ++self.eo;
    archive & hierarchy<WorldObject>(self);
}

SERIALIZATION(SaveLoad, MoveableObject)
{
    ++self.mo;
    archive & hierarchy<EnvironmentObject>(self);
}

SERIALIZATION(SaveLoad, DestructibleObject)
{
    ++self.dso;
    archive & hierarchy<EnvironmentObject>(self);
}

SERIALIZATION(SaveLoad, DecorativeObject)
{
    ++self.dco;
    archive & hierarchy<DestructibleObject, MoveableObject>(self);
}

SERIALIZATION(SaveLoad, FoliageObject)
{
    ++self.fo;
    archive & hierarchy<WorldObject>(self);
}

SERIALIZATION(SaveLoad, DecorativeFoliageObject)
{
    ++self.dcfo;
    archive & hierarchy<DecorativeObject, FoliageObject>(self);
}

TEST(TestLibrary, TestInheritance)
{
    auto check_hierarchy_count = [](std::shared_ptr<DecorativeFoliageObject>& d)
    {
        return d->wo == 1 && d->eo == 1 && d->mo == 1 && d->dso == 1 && d->dco == 1 && d->dcfo == 1;
    };

    std::vector<unsigned char> storage;
    {
        auto d = std::make_shared<DecorativeFoliageObject>();
        std::shared_ptr<WorldObject> b = d;

        auto ar = oarchive(storage);
        ar & b;

        EXPECT("write.hierarchy count", check_hierarchy_count(d));
    }
    {
        std::shared_ptr<WorldObject> b = nullptr;

        auto ar = iarchive(storage);
        ar & b;

        auto d = std::dynamic_pointer_cast<DecorativeFoliageObject>(b);

        ASSERT("read.inited", d != nullptr);
        EXPECT("read.hierarchy count", check_hierarchy_count(d));
    }
}

TEST_MODULE()
{

class SomeObjectImpl
{
    SERIALIZABLE(SomeObjectImpl)

public:
    SomeObjectImpl() = default;
    SomeObjectImpl(int data) : data_(data) {}

protected:
    int data_;
};

class SomeObject : protected SomeObjectImpl // try protected inheritance
{
    SERIALIZABLE(SomeObject)

public:
    SomeObject(int data = 0) : SomeObjectImpl(data), inner_data_(data/2) {}

public:
    bool operator== (const SomeObject& s)
    {
        return data_ == s.data_ && inner_data_ == s.inner_data_;
    };

private:
    int inner_data_;
};

struct PolymorphicBaseImpl : public sf::Instantiable
{
    SERIALIZABLE(PolymorphicBaseImpl)
};

struct PolymorphicBase : protected PolymorphicBaseImpl // not allowed!
{
    SERIALIZABLE(PolymorphicBase)
};

struct PolymorphicDerived : public PolymorphicBase
{
    SERIALIZABLE(PolymorphicDerived)
};

} // TEST_MODULE

SERIALIZATION(SaveLoad, SomeObjectImpl)
{
    archive & self.data_;
}

SERIALIZATION(SaveLoad, SomeObject)
{
    archive & hierarchy<SomeObjectImpl>(self) & self.inner_data_;
}

SERIALIZATION(SaveLoad, PolymorphicBaseImpl) {}

SERIALIZATION(SaveLoad, PolymorphicBase)
{
    archive & hierarchy<PolymorphicBaseImpl>(self);
}

SERIALIZATION(SaveLoad, PolymorphicDerived)
{
    archive & hierarchy<PolymorphicBase>(self);
}

TEST(TestLibrary, TestAccess)
{
    static SomeObject s_so(123);

    std::vector<unsigned char> storage;
    {
        SomeObject so = s_so;

        auto ar = oarchive(storage);
        ar & so;
    }
    {
        SomeObject so;

        auto ar = iarchive(storage);
        ar & so;

        EXPECT("non-public inheritance.value", so == s_so);
    }

    using sf::dynamic::InstantiableRegistry;

    {
        // since PolymorphicBase protected inherited from PolymorphicBaseImpl, where
        // PolymorphicBaseImpl is instantiable type - we cannot use dynamic_cast for serialization
        // since we get nullptr after apply casting

        EXPECT("public instantiable", InstantiableRegistry::is_instantiable<PolymorphicBaseImpl>::value);

        EXPECT("non-public instantiable",
            !InstantiableRegistry::is_instantiable<PolymorphicBase>::value &&
            !InstantiableRegistry::is_instantiable<PolymorphicDerived>::value);
    }
}

// since we dont use SERIALIZABLE remember that:
// SERIALIZABLE contains SERIALIZATION_ACCESS() and INSTANTIABLE_BODY() macros

TEST_MODULE()
{

struct NoTraitBase : sf::Instantiable
{
    int b = 0;
};

struct NoTraitDerived : NoTraitBase
{
    int d = 0;
};

} // TEST_MODULE

SERIALIZATION(SaveLoad, NoTraitBase)
{
    archive & self.b;
}

SERIALIZATION(SaveLoad, NoTraitDerived)
{
    archive & hierarchy<NoTraitBase>(self) & self.d;
}

// if user does not SERIALIZABLE macro, then library will use typeid for type hashing,
// mixed usage SERIALIZABLE and typeid does not allowed!
// You also can specify hashing behavior with macro SF_TYPE_HASH - see Core/Hash.hpp
// Note that: EXPORT_INSTANTIABLE_KEY("some_text", some_type) - is not allowed with typeid using

TEST(TestLibrary, TestNoTrait)
{
    using sf::serializable;

    static NoTraitDerived s_d;
    s_d.b = 246253;
    s_d.d = 4895792;

    std::vector<unsigned char> storage;
    {
        serializable<NoTraitDerived>();

        std::shared_ptr<NoTraitBase> b = std::make_shared<NoTraitDerived>(s_d);

        auto ar = oarchive(storage);
        ar & b;
    }
    {
        std::shared_ptr<NoTraitBase> b = nullptr;

        auto ar = iarchive(storage);
        ar & b;

        auto d = std::dynamic_pointer_cast<NoTraitDerived>(b);

        ASSERT("no-trait.inited", d != nullptr);
        EXPECT("no-trait.value", d->b == s_d.b && d->d == s_d.d);
    }
}

#include "TestLibrary/BaseObject.hpp"
#include "TestLibrary/DerivedObject.hpp"

TEST(TestLibrary, TestPartition)
{
    static DerivedObject s_d;
    s_d.id = 1894981;
    s_d.name = "Unit";
    s_d.data = 3.14f;

    std::vector<unsigned char> storage;
    {
        std::shared_ptr<BaseObject> b = std::make_shared<DerivedObject>(s_d);

        auto ar = oarchive(storage);
        ar & b;
    }
    {
        std::shared_ptr<BaseObject> b = nullptr;

        auto ar = iarchive(storage);
        ar & b;

        auto d = std::dynamic_pointer_cast<DerivedObject>(b);

        ASSERT("saveload partition.inited", d != nullptr);
        EXPECT("saveload partition.value",
            d->id == s_d.id && d->name == s_d.name && d->data == s_d.data);
    }
}
