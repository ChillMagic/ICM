#pragma once
#include "basic.h"
#include "parser/objectenum.h"
#include "objectmanager.h"

namespace ICM
{
	//===========================================================
	// * Namespace Types
	//===========================================================
	namespace Types
	{
		//===========================================================
		// * Template TType
		//===========================================================
		using TypeName = const char* const;
		template <TypeUnit> struct TType { using Type = void; static TypeName Name; };

		//===========================================================
		// * Template CType
		//===========================================================
		template <typename> struct CType { constexpr static TypeUnit index = T_Null; };
	}
	template <TypeUnit T> using TType = Types::TType<T>;

	using DataPointer = void*;
	using ConstDataPointer = const void*;

	//=============================================
	// * Struct TypeInfo
	//=============================================
	struct TypeInfo
	{
		using ConFunc = void(DataPointer);
		using DesFunc = void(DataPointer);
		using CpyFunc = void(DataPointer, ConstDataPointer);
		using EquFunc = bool(ConstDataPointer, ConstDataPointer);
		using TosFunc = std::string(ConstDataPointer);

		TypeUnit index;
		const char* name;
		size_t size;

		ConFunc* construct;
		DesFunc* destruct;
		CpyFunc* ncopy;
		EquFunc* equal;

		TosFunc* to_string;
		TosFunc* to_output;
		TosFunc* to_string_code;

		DataPointer alloc() const {
			return Memory::alloc(size);
		}
		DataPointer mcopy(ConstDataPointer src) const {
			return Memory::mcopy(alloc(), src, size);
		}
		DataPointer copy(ConstDataPointer src) const {
			void *dst = alloc();
			ncopy(dst, src);
			return dst;
		}
	};
	extern map<TypeUnit, TypeInfo> TypeInfoTable;

	//=============================================
	// * Struct Object
	//---------------------------------------------
	//   Object is a struct with a type & a data.
	//=============================================
	struct Object
	{
	public:
		Object(TypeUnit type = T_Null, DataPointer data = nullptr) : type(type), data(data) {}

#define CheckTypeInfo() assert(this && (this->type == 0 || TypeInfoTable.find(this->type) != TypeInfoTable.end()))

		string to_string() const {
			CheckTypeInfo();
			return TypeInfoTable[type].to_string(data);
		}
		string to_output() const {
			CheckTypeInfo();
			return TypeInfoTable[type].to_output(data);
		}
		string to_string_code() const {
			CheckTypeInfo();
			return TypeInfoTable[type].to_string_code(data);
		}

		Object* clone() const {
			CheckTypeInfo();
			Object *p = new Object(this->type);
			if (this->data == nullptr) return p;
			p->data = TypeInfoTable[type].copy(data);
			return p;
		}
		void set(const Object *op) {
			CheckTypeInfo();
			this->data = TypeInfoTable[type].copy(op->data);
		}
		// Method
		bool equ(const Object *obj) const {
			CheckTypeInfo();
			return this == obj || (this->type == obj->type && TypeInfoTable[type].equal(this->data, obj->data));
		}

#undef CheckTypeInfo

		// New Change (Building):
	public:
		template <TypeUnit _TU> using Ty = typename TType<_TU>::Type;

		template <TypeUnit _TU> Ty<_TU>* get() { return _ptr<Ty<_TU>>(); }
		template <TypeUnit _TU> const Ty<_TU>* get() const { return _ptr<Ty<_TU>>(); }
		template <TypeUnit _TU> Ty<_TU>& dat() { return _ref<Ty<_TU>>(); }
		template <TypeUnit _TU> const Ty<_TU>& dat() const { return _ref<Ty<_TU>>(); }
		bool isType(TypeUnit _TU) const { return type == _TU; }
		
		TypeUnit type = T_Null;
		DataPointer data = nullptr;
		// All data should be copyable, have no shared resource.

		// Methods for data
		template <typename T> T* _ptr() { return static_cast<T*>(data); }
		template <typename T> const T* _ptr() const { return static_cast<const T*>(data); }
		template <typename T> T& _ref() { return *_ptr<T>(); }
		template <typename T> const T& _ref() const { return *_ptr<T>(); }
	};

	struct StaticObject
	{
		StaticObject() :
			Null(),
			Nil(T_Nil),
			True(T_Boolean, (void*)&Value_True),
			False(T_Boolean, (void*)&Value_False),
			Zero(T_Number, (void*)&Value_Zero)
		{};

		const static bool Value_True;
		const static bool Value_False;
		const static uint64_t Value_Zero;
		
		Object Null;
		Object Nil;
		Object True;
		Object False;
		Object Zero;
	};

	extern StaticObject Static;

	// Types
	using DataList = lightlist<Object*>;
	using CallFunc = ObjectPtr(const DataList&);
	using FuncPtr = std::function<CallFunc>;
	using LDataList = Common::lightlist<Object*>;
	using LFuncPtr = std::function<void(Object*&, const LDataList &)>;
	// Functions
	template <typename T, typename... Args>
	inline ObjectPtr createObject(Args... args) {
		return ObjectPtr(new T(args...));
	}
	Object* createObject(DefaultType type);
	ObjectPtr createError(const string &errinfo);
	string to_string(const ObjectPtr &obj);
	string to_string(const DataList &obj);
	string to_string(const vector<ObjectPtr> &list);
	string to_string(const vector<Object*> &list);
}
