#include "Object.hpp"

// Object::Object(const std::string& objectName)
//     : m_objectName(objectName)
// {
//     m_refHandler = std::make_shared<ObjectRefHandler>();
// }
// Object::~Object()
// {
//     m_refHandler->AtomicResetAllRefs();
// }

// void Object::AddRef(ObjectRef* ref)
// {
//     m_refHandler->AddRef(ref);
// }
// void Object::RemoveRef(ObjectRef* ref)
// {
//     m_refHandler->RemoveRef(ref);
// }
// void Object::ReplaceRef(ObjectRef& from, ObjectRef& to)
// {
//     m_refHandler->ReplaceRef(from, to);
// }

// const std::string& Object::getName() const
// {
//     return m_objectName;
// }

// void Object::debugPrint()
// {
//     std::cout<<"Debug print object : "<< m_objectName << ". Debug References : " << std::endl;
//     m_refHandler->debugPrint();
//     std::cout<<std::endl;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ObjectRef::ObjectRef()
//      : m_object(nullptr)
//      , m_owner(nullptr)
//     {}

// ObjectRef::ObjectRef(Object* pointedObject, Object* owningObject)
//     : m_object(nullptr)
//     , m_owner(owningObject)
// {
//     // Reset it if it points to another object
//     Reset();
//     // Points to the given object
//     m_object = pointedObject;
//     // make the pointed object aware of this reference
//     m_object->AddRef(this);
// }

// ObjectRef::ObjectRef(const ObjectRef& other)
// {
//     // Reset it if it points to another object
//     Reset();
//     // setup new owner
//     m_owner = other.m_owner;
//     // Points to the given object
//     m_object = other.m_object;
//     // make the pointed object aware of this reference
//     m_object->AddRef(this);
// }

// // friend void ObjectRef::swap(ObjectRef& first, ObjectRef& second)
// // {
// //     using std::swap;

// //     first.m_object->ReplaceRef(first, second);
// //     second.m_object->ReplaceRef(second, first);
// //     swap(first.m_object, second.m_object);
// // }

// ObjectRef::ObjectRef(ObjectRef&& other)
// {
//     using std::swap;
//     swap(*this, other);
// }

// ObjectRef& ObjectRef::operator=(ObjectRef other)
// {
//     using std::swap;
//     swap(*this, other);
//     return *this;
// }

// ObjectRef::~ObjectRef()
// {
//     Reset();
// }

// void ObjectRef::AtomicReset()
// {
//     m_object = nullptr;
// }

// void ObjectRef::Reset()
// {
//     if(m_object != nullptr)
//     {
//         m_object->RemoveRef(this);
//     }
//     AtomicReset();
// }

// Object* ObjectRef::operator->()
// {
//     return m_object;
// }

// const Object* ObjectRef::getOwner() const
// {
//     return m_owner;
// }


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void ObjectRefHandler::AddRef(ObjectRef* ref)
// {
//     m_refs.push_back(ref);
// }

// void ObjectRefHandler::RemoveRef(ObjectRef* ref)
// {
//     m_refs.erase(std::remove(m_refs.begin(), m_refs.end(), ref));
// }

// void ObjectRefHandler::ReplaceRef(ObjectRef& from, ObjectRef& to)
// {
//     auto foundIt = std::find(m_refs.begin(), m_refs.end(), &from);
//     if(foundIt != m_refs.end())
//     {
//         *foundIt = &to;
//     }
// }

// void ObjectRefHandler::ResetAllRefs()
// {
//     for(const auto& refIt : m_refs)
//     {
//         refIt->Reset();
//     }
// }

// void ObjectRefHandler::AtomicResetAllRefs()
// {
//     for(const auto& refIt : m_refs)
//     {
//         refIt->AtomicReset();
//     }
// }

// void ObjectRefHandler::debugPrint()
// {
//     std::cout<<"Number of references to this object : "<< m_refs.size() << "." << std::endl;
//     std::cout<<"Objects which reference it : " << std::endl;
//     const Object* owner = nullptr;
//     for(auto& ref : m_refs)
//     {
//         owner = ref->getOwner();
//         if(owner != nullptr)
//             std::cout<<"->"<<owner->getName()<<", ";
//         else
//             std::cout<<"--No valid owner for this object-- , ";
//     }
// }