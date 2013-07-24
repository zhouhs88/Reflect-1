#pragma once

#include "Platform/Types.h"
#include "Platform/Utility.h"

#include "Foundation/Attribute.h"
#include "Foundation/DynamicArray.h"
#include "Foundation/Set.h"

#include "Reflect/MetaType.h"
#include "Reflect/Registry.h"
#include "Reflect/Translator.h"

namespace Helium
{
	namespace Reflect
	{
		class MetaStruct;
		typedef void (*PopulateCompositeFunc)( MetaStruct& );

		namespace FieldFlags
		{
			enum MetaType
			{
				Discard     = 1 << 0,       // disposable fields are not serialized
				Force       = 1 << 1,       // forced fields are always serialized
				Share       = 1 << 2,       // shared fields are not cloned or compared deeply
				Hide        = 1 << 3,       // hidden fields are not inherently visible in UI
				ReadOnly    = 1 << 4,       // read-only fields cannot be edited in the UI inherently
			};
		}

		//
		// Field (member data of a composite)
		//

		class HELIUM_REFLECT_API Field : public PropertyCollection
		{
		public:
			Field();

			// allocate and connect to the default
			bool IsDefaultValue( void* address, Object* object, uint32_t index = 0 ) const;

			// determine if this field should be serialized
			bool ShouldSerialize( void* address, Object* object, uint32_t index = 0 ) const;

			const MetaStruct* m_Structure;  // the type we are a field of
			const char*   m_Name;       // name of this field
			uint32_t         m_Size;       // the size of this field
			uint32_t         m_Count;      // the static array size
			uint32_t         m_Offset;     // the offset to the field
			uint32_t         m_Flags;      // flags for special behavior
			uint32_t         m_Index;      // the unique id of this field
			const MetaType*      m_KeyType;    // the key type, if any, of the internal data
			const MetaType*      m_ValueType;  // the value type, if any, of the internal data
			Translator*      m_Translator; // interface to the data
		};

		//
		// Empty struct just for type deduction purposes (for stand alone structs, not Object classes)
		//  don't worry though, even though this class is non-zero in size on its own,
		//  your derived struct type can use the memory this takes due to C/C++ standard
		//  'Empty Base Optimization'
		//

		struct HELIUM_REFLECT_API StructureBase
		{
		};

		//
		// MetaStruct (C++ `struct`)
		//

		class HELIUM_REFLECT_API MetaStruct : public MetaType
		{
		public:
			REFLECT_META_DERIVED( MetaIds::MetaStruct, MetaStruct, MetaType );

		protected:
			MetaStruct();
			~MetaStruct();

		public:
			// protect external allocation to keep inlined code in this dll
			static MetaStruct* Create();

			// creator for structure types
			template< class StructureT >
			static void Create( MetaStruct const*& pointer, const char* name, const char* baseName );

			// shared logic with class types
			template< class StructureT >
			static void Create( const char* name, const char* baseName, PopulateCompositeFunc populate, MetaStruct* info );

			// overloaded functions from MetaType
			virtual void Register() const HELIUM_OVERRIDE;
			virtual void Unregister() const HELIUM_OVERRIDE;

			// inheritance hierarchy
			bool IsType(const MetaStruct* type) const;
			void AddDerived( const MetaStruct* derived ) const;
			void RemoveDerived( const MetaStruct* derived ) const;

			// Compare two composite instances of *this* type
			bool Equals( void* compositeA, Object* objectA, void* compositeB, Object* objectB ) const;

			// copies data from one instance to another by finding a common base class and cloning all of the fields from the source object into the destination object.
			void Copy( void* compositeSource, Object* objectSource, void* compositeDestination, Object* objectDestination, bool shallowCopy = false ) const;

			// find a field in this composite
			const Field* FindFieldByName(uint32_t crc) const;
			const Field* FindFieldByIndex(uint32_t index) const;
			const Field* FindFieldByOffset(uint32_t offset) const;

			// finds the field info given a pointer to a member variable on a class.
			template< class StructureT, typename FieldT >
			const Field* FindField( FieldT StructureT::* pointerToMember ) const;

			// computes the number of fields in all our base classes (the base index for our fields)
			uint32_t GetBaseFieldCount() const;

			// concrete field population functions, called from template functions below with deducted data
			Reflect::Field* AddField();

			// compute the offset from the 'this' pointer for the specified pointer-to-member
			template < class StructureT, class FieldT >
			static inline uint32_t GetOffset( FieldT StructureT::* field );

			// compute the count of a static array
			template < class T, size_t N >
			static inline uint32_t GetArrayCount( T (&array)[N] );

			// compute the count of a normal type (1)
			template < class T >
			static inline uint32_t GetCount( std::false_type /*is_array*/ );

			// compute the count of an array type (1)
			template < class T >
			static inline uint32_t GetCount( std::true_type  /*is_array*/  );

			// deduce key type
			template < class T >
			static inline const MetaType* DeduceKeyType( std::false_type /*is_array*/ );
			template < class T >
			static inline const MetaType* DeduceKeyType( std::true_type  /*is_array*/  );

			// deduce value type
			template < class T >
			static inline const MetaType* DeduceValueType( std::false_type /*is_array*/ );
			template < class T >
			static inline const MetaType* DeduceValueType( std::true_type  /*is_array*/  );

			// create translator object
			template < class T >
			static inline Translator* AllocateTranslator( std::false_type /*is_array*/ );
			template < class T >
			static inline Translator* AllocateTranslator( std::true_type  /*is_array*/  );

			// deduce and allocate the appropriate translator object and append field data to the composite
			template < class StructureT, class FieldT >
			inline Reflect::Field* AddField( FieldT StructureT::* field, const char* name, uint32_t flags = 0, Translator* translator = NULL );

		public:
			const MetaStruct*         m_Base;         // the base type name
			mutable const MetaStruct* m_FirstDerived; // head of the derived linked list, mutable since its populated by other objects
			mutable const MetaStruct* m_NextSibling;  // next in the derived linked list, mutable since its populated by other objects
			DynamicArray< Field >    m_Fields;       // fields in this composite
			PopulateCompositeFunc    m_Populate;     // function to populate this structure
			void*                    m_Default;      // default instance
		};

		template< class ClassT, class BaseT >
		class MetaStructRegistrar : public MetaTypeRegistrar
		{
		public:
			MetaStructRegistrar(const char* name);
			~MetaStructRegistrar();

			virtual void Register();
			virtual void Unregister();
		};

		template< class ClassT >
		class MetaStructRegistrar< ClassT, void > : public MetaTypeRegistrar
		{
		public:
			MetaStructRegistrar(const char* name);
			~MetaStructRegistrar();

			virtual void Register();
			virtual void Unregister();
		};
	}
}

// declares type checking functions
#define _REFLECT_DECLARE_BASE_STRUCTURE( STRUCTURE ) \
public: \
typedef STRUCTURE This; \
static const Helium::Reflect::MetaStruct* CreateStructure(); \
static const Helium::Reflect::MetaStruct* s_Structure; \
static Helium::Reflect::MetaStructRegistrar< STRUCTURE, void > s_Registrar;

#define _REFLECT_DECLARE_DERIVED_STRUCTURE( STRUCTURE, BASE ) \
public: \
typedef BASE Base; \
typedef STRUCTURE This; \
static const Helium::Reflect::MetaStruct* CreateStructure(); \
static const Helium::Reflect::MetaStruct* s_Structure; \
static Helium::Reflect::MetaStructRegistrar< STRUCTURE, BASE > s_Registrar;

// defines the static type info vars
#define _REFLECT_DEFINE_BASE_STRUCTURE( STRUCTURE ) \
const Helium::Reflect::MetaStruct* STRUCTURE::CreateStructure() \
{ \
	HELIUM_ASSERT( s_Structure == NULL ); \
	Helium::Reflect::MetaStruct::Create<STRUCTURE>( s_Structure, TXT( #STRUCTURE ), NULL ); \
	return s_Structure; \
} \
const Helium::Reflect::MetaStruct* STRUCTURE::s_Structure = NULL; \
Helium::Reflect::MetaStructRegistrar< STRUCTURE, void > STRUCTURE::s_Registrar( TXT( #STRUCTURE ) );

#define _REFLECT_DEFINE_DERIVED_STRUCTURE( STRUCTURE ) \
const Helium::Reflect::MetaStruct* STRUCTURE::CreateStructure() \
{ \
	HELIUM_ASSERT( s_Structure == NULL ); \
	HELIUM_ASSERT( STRUCTURE::Base::s_Structure != NULL ); \
	Helium::Reflect::MetaStruct::Create<STRUCTURE>( s_Structure, TXT( #STRUCTURE ), STRUCTURE::Base::s_Structure->m_Name ); \
	return s_Structure; \
} \
const Helium::Reflect::MetaStruct* STRUCTURE::s_Structure = NULL; \
Helium::Reflect::MetaStructRegistrar< STRUCTURE, STRUCTURE::Base > STRUCTURE::s_Registrar( TXT( #STRUCTURE ) );

// declares a concrete object with creator
#define REFLECT_DECLARE_BASE_STRUCTURE( STRUCTURE ) \
	_REFLECT_DECLARE_BASE_STRUCTURE( STRUCTURE )

#define REFLECT_DECLARE_DERIVED_STRUCTURE( STRUCTURE, BASE ) \
	_REFLECT_DECLARE_DERIVED_STRUCTURE( STRUCTURE, BASE )

// defines a concrete object
#define REFLECT_DEFINE_BASE_STRUCTURE( STRUCTURE ) \
	_REFLECT_DEFINE_BASE_STRUCTURE( STRUCTURE )

#define REFLECT_DEFINE_DERIVED_STRUCTURE( STRUCTURE ) \
	_REFLECT_DEFINE_DERIVED_STRUCTURE( STRUCTURE  )

#include "Reflect/MetaStruct.inl"
