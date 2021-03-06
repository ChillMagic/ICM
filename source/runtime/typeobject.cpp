#include "basic.h"
#include "parser/keyword.h"
#include "runtime/typeobject.h"
#include "runtime/functable.h"

namespace ICM
{
	TypeObject::TypeObject(const DefaultType &type, const Function::Signature &valuetype)
		: type(type), sign(new Function::Signature(valuetype)) {}

	bool TypeObject::checkType(const TypeObject &argT) const
	{
		const TypeObject &parT = *this;

		if (parT.isVary())
			return true;
		if (parT.isFunc()) {
			if (argT.isFunc()) {
				if (parT.haveSign()) {
					auto &ps = parT.getSign();
					if (argT.haveSign()) {
						auto &as = argT.getSign();
						return ps.checkType(as);
					}
					else {
						auto &ft = argT.getFuncTableUnit();
						for (size_t i : Range<size_t>(0, ft.size()))
							if (ps.checkType(ft[i].getSign()))
								return true;
						return false;
					}
				}
				else {
					return true;
				}
			}
		}
		return parT.getType() == argT.getType();
	}
}
