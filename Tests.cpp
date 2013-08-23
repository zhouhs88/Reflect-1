#include "ReflectPch.h"
#include "Reflect/Tests.h"

using namespace Helium;
using namespace Reflect;

#if !HELIUM_RELEASE

struct EmptyBaseCheck : Struct
{
	uint32_t variable;
};
HELIUM_COMPILE_ASSERT( sizeof( EmptyBaseCheck ) == sizeof( uint32_t ) ); // check for empty-base optimization

HELIUM_DEFINE_ENUM( TestEnumeration );

void TestEnumeration::PopulateMetaType( MetaEnum& info )
{
	info.AddElement( ValueOne, TXT( "Value One" ) );
	info.AddElement( ValueTwo, TXT( "Value Two" ) );
}

HELIUM_DEFINE_BASE_STRUCT( TestStructure );

void TestStructure::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &TestStructure::m_Uint8,  "Unsigned 8-bit Integer" );
	comp.AddField( &TestStructure::m_Uint16, "Unsigned 16-bit Integer" );
	comp.AddField( &TestStructure::m_Uint32, "Unsigned 32-bit Integer" );
	comp.AddField( &TestStructure::m_Uint64, "Unsigned 64-bit Integer" );

	comp.AddField( &TestStructure::m_Int8,  "Signed 8-bit Integer" );
	comp.AddField( &TestStructure::m_Int16, "Signed 16-bit Integer" );
	comp.AddField( &TestStructure::m_Int32, "Signed 32-bit Integer" );
	comp.AddField( &TestStructure::m_Int64, "Signed 64-bit Integer" );

	comp.AddField( &TestStructure::m_Float32, "32-bit Floating Point" );
	comp.AddField( &TestStructure::m_Float64, "64-bit Floating Point" );
	
	comp.AddField( &TestStructure::m_StdVectorUint32, "std::vector of Signed 32-bit Integers" );
	comp.AddField( &TestStructure::m_StdSetUint32, "std::vector of Unsigned 32-bit Integers" );
	comp.AddField( &TestStructure::m_StdMapUint32, "std::map of Unsigned 32-bit Integers" );
	
	comp.AddField( &TestStructure::m_FoundationDynamicArrayUint32, "Dynamic Array of Signed 32-bit Integers" );
	comp.AddField( &TestStructure::m_FoundationSetUint32, "Set of Unsigned 32-bit Integers" );
	comp.AddField( &TestStructure::m_FoundationMapUint32, "Map of Unsigned 32-bit Integers" );
}

HELIUM_DEFINE_CLASS( TestObject );

void TestObject::PopulateMetaType( Reflect::MetaClass& comp )
{
	comp.AddField( &TestObject::m_Struct, "MetaStruct" );
	comp.AddField( &TestObject::m_StructArray, "MetaStruct Array" );

	comp.AddField( &TestObject::m_Enumeration, "MetaEnum" );
	comp.AddField( &TestObject::m_EnumerationArray, "MetaEnum Array" );
}

void Reflect::RunTests()
{
	StrongPtr< TestObject > object = new TestObject ();
}

#endif