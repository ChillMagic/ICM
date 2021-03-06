#pragma once
#include "runtime/object.h"
#include "runtime/typeobject.h"

namespace ICM
{
	//=======================================
	// * Namespace Function
	//=======================================
	namespace Function
	{
		//=======================================
		// * Class Signature
		//=======================================
		class Signature
		{
		public:
			using List = lightlist<TypeObject>;
			using InitList = std::initializer_list<TypeObject>;

			Signature() = default;
			Signature(const InitList &intype, const TypeObject& outtype, bool last_is_args = false);

			const List& getInType() const { return InType; }
			const TypeObject& getOutType() const { return OutType; }
			bool isLastArgs() const { return last_is_args; }
			bool checkType(const Signature &sign) const;
			bool checkType(const lightlist<TypeObject> &argT) const;

			bool checkType(const vector<TypeObject>& argT) const;

		private:
			List InType;
			TypeObject OutType;
			bool last_is_args;
		};
	}

	string to_string(const ICM::Function::Signature& sign);

	namespace Function
	{
		//=======================================
		// * Class FuncObject
		//=======================================
		class FuncObject
		{
		public:
			FuncObject() = default;
			FuncObject(const FuncPtr &func, const Signature &sign, const LFuncPtr &Lfunc = nullptr)
				: func(func), sign(sign), Lfunc(Lfunc) {}

			template <typename... Args>
			bool checkType(Args... args) const {
				return sign.checkType(args...);
			}
			ObjectPtr call(const DataList &dl) const {
				// If there was a crash, make sure that the pointer 'func' is effective.
				return func(dl);
			}

			string to_string() const {
				return ICM::to_string(sign);
			}
			const Signature& getSign() const {
				return sign;
			}

		private:
			FuncPtr func;
			Signature sign;
			LFuncPtr Lfunc;
		};

		//=======================================
		// * Class FuncInitObject
		//=======================================
		class FuncInitObject
		{
			using LFP = ICM::Object*(const Common::lightlist<Object*> &); // TODO
		public:
			FuncObject get_f() {
				const auto &f1 = std::bind(&FuncInitObject::func, this, std::placeholders::_1);
				const auto &f2 = std::bind(&FuncInitObject::funcL, this, std::placeholders::_1, std::placeholders::_2);
				return FuncObject(f1, sign(), f2);
			}

		protected:
			virtual Signature sign() const = 0;
			virtual ObjectPtr func(const DataList &list) const = 0;
			virtual void funcL(Object* &, const LDataList &) const {
				println("Error : Not find funcL.");
			}
		};

		//=======================================
		// * Class SignTree
		//=======================================
		class SignTree
		{
		public:
			struct Node;
			using NodePtr = unique_ptr<Node>;
			using TypeObjectPtr = TypeObject*;
		public:
			struct Node
			{
			public:
				Node() : data(nullptr), parent(nullptr) {}
				Node(const TypeObjectPtr &top, Node* parent)
					: data(top), parent(parent) {}

				Node* find(const TypeObject &to) const {
					for (auto &p : children) {
						if (p->data && *p->data == to)
							return p.get();
					}
					return nullptr;
				}
				Node* find(const TypeObjectPtr &to) const {
					for (auto &p : children) {
						if (p->data == nullptr && to == nullptr)
							return p.get();
						else if (p->data == to)
							return p.get();
					}
					return nullptr;
				}
				Node* push(const TypeObjectPtr &top) {
					children.push_back(NodePtr(new Node(top, this)));
					return children.back().get();
				}
				Node* pushEnd(size_t id) {
					NodePtr node(new Node(nullptr, this));
					node->index = id;
					children.push_back(std::move(node));
					return children.back().get();
				}
				const vector<NodePtr>& getChildren() const {
					return children;
				}
				bool checkType(const TypeObject &type) const {
					return data->checkType(type);
				}
				bool isNull() const {
					return data == nullptr;
				}
				bool isFunc() const {
					return data->isFunc();
				}
				const Signature& getSign() const {
					return data->getSign();
				}
				size_t getIndex() const {
					return index;
				}
				bool canEnd() const {
					for (const NodePtr &np : children) {
						if (np->isNull())
							return true;
					}
					return false;
				}
				bool canEnd(size_t &index) const {
					for (const NodePtr &np : children) {
						if (np->isNull()) {
							index = np->index;
							return true;
						}
					}
					return false;
				}
				bool isArgs() const {
					return isargs;
				}
				void setArgs() {
					isargs = true;
				}
				Node* getParent() const {
					return parent;
				}

			private:
				TypeObjectPtr data;
				Node* parent;
				vector<NodePtr> children;
				size_t index;
				bool isargs = false;
			};
		public:
			SignTree() : root(new Node()), level(1) {}

			void insert(const FuncObject &funcO) {
				Node* currptr = root.get();
				size_t count = 1;
				for (const TypeObject &to : funcO.getSign().getInType()) {
					Node* ptr = currptr->find(to);
					if (ptr) {
						currptr = ptr;
					}
					else {
						currptr = currptr->push(TypeObjectPtr(new TypeObject(to)));
						if (count == level.size())
							level.push_back(vector<const vector<NodePtr>*>());
						level[count].push_back(&currptr->getChildren());
					}
					count++;
				}
				if (funcO.getSign().isLastArgs()) {
					currptr->setArgs();
				}
				currptr->pushEnd(funcdata.size());
				funcdata.push_back(&funcO);
			}
			const Node* getRoot() const {
				return root.get();
			}
			const FuncObject* getFunc(size_t index) const {
				return funcdata[index];
			}

		private:
			NodePtr root;
			vector<vector<const vector<NodePtr>*>> level;
			vector<const FuncObject*> funcdata;
		};

		class SignTreeMatch
		{
			using Node = SignTree::Node;
		public:
			SignTreeMatch(const SignTree &st)
				: ST(st) {}

			const Node* checkSingle(const Node *data, const Node *p, const TypeObject &type, bool &change);
			const Node* checkSingle(const Node *data, const TypeObject &type, size_t &index);
			const FuncObject* match(const lightlist<TypeObject> &argT);

		private:
			const SignTree& ST;
		};
	}

	//=======================================
	// * Functions
	//=======================================
	size_t getCallID(const FuncTableUnit &ftu, const DataList &dl);
	ObjectPtr checkCall(const ICM::FuncTableUnit &ftb, const ICM::DataList &dl);
	string to_string(const ICM::Function::Signature &sign);
}
