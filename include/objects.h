#ifndef _ICM_OBJECTS_H_
#define _ICM_OBJECTS_H_

#include "basic.h"
#include "objectmanager.h"
#include "file.h"

namespace ICM
{
	namespace Objects
	{
		//=======================================
		// * Declarations
		//=======================================
		class Object;
		class Error;
		class Nil;
		class Boolean;
		class Number;
		class String;
		class Symbol;
		class List;
		class Disperse;
		class Identifier;
		class Keyword;
		class Function;
		class TypeClass;
	}

	// Types
	using DataList = std::vector<ObjectPtr>;
	using CallFunc = ObjectPtr(const DataList&);
	using FuncPtr = std::function<CallFunc>;
	using LDataList = Common::lightlist<Objects::Object*>;
	using LFuncPtr = std::function<void(Objects::Object &, const LDataList &)>;
	// Functions
	template <typename T, typename... Args>
	inline ObjectPtr createObject(Args... args) {
		return ObjectPtr(new T(args...));
	}
	Objects::Object* createObject(DefaultType type);
	ObjectPtr createError(const string &errinfo);
	const ObjectPtr& adjustObjectPtr(const ObjectPtr &ptr);
	std::string to_string(const Objects::Object &obj);
	DataList::iterator begin(Objects::Disperse *disp);
	DataList::iterator end(Objects::Disperse *disp);

	namespace Objects
	{
		//=======================================
		// * Class Object
		//=======================================
		class Object
		{
		public:
			virtual ~Object() {}
			virtual string to_string() const {
				return "Object";
			}
			virtual string to_output() const {
				return to_string();
			}
			virtual string to_string_code() const {
				return to_string();
			}
			virtual DefaultType getType() const {
				return Type;
			}
			virtual Object* clone() const {
				return new Object(*this);
			}
			// Method
			virtual Boolean* equ(const ObjectPtr &obj) const;
			virtual void write(File &file) const {}
			virtual void read(File &file) {}
			// Const
			static const DefaultType Type = T_Object;

		protected:
			bool type_equal(const ObjectPtr &obj) const {
				return this->getType() == obj->getType();
			}
		};
	}
}

#endif
