// #pragma once

// #include <memory>
// #include <vector>
// #include <algorithm>
// #include <utility>
// #include <string>
// #include <iostream>
// #include <map>

// class ObjectRefHandler;
// class ObjectRef;

// class Object
// {
// private:
//     std::shared_ptr<ObjectRefHandler> m_refHandler;
//     std::string m_objectName;

// public:
//     Object(const std::string& objectName);
//     ~Object();
//     void AddRef(ObjectRef* ref);
//     void RemoveRef(ObjectRef* ref);
//     void ReplaceRef(ObjectRef& from, ObjectRef& to);
//     const std::string& getName() const;
//     void debugPrint();
// };

// class ObjectRef
// {
// private:
//     Object* m_object;
//     Object* m_owner;

// public:
//     ObjectRef();
//     ObjectRef(Object* pointedObject, Object* owningObject = nullptr);
//     ObjectRef(const ObjectRef& other);
//     friend void swap(ObjectRef& first, ObjectRef& second)
//     {
//         using std::swap;

//         if(first.m_object != nullptr)
//             first.m_object->ReplaceRef(first, second);
//         if(second.m_object)
//             second.m_object->ReplaceRef(second, first);
            
//         swap(first.m_object, second.m_object);
//         swap(first.m_owner, second.m_owner);
//     }
//     ObjectRef(ObjectRef&& other);
//     ObjectRef& operator=(ObjectRef other);
//     virtual ~ObjectRef();
//     void AtomicReset();
//     void Reset();
//     Object* operator->();
//     const Object* getOwner() const;
// };

// class ObjectRefHandler
// {
// private:
//     std::vector<ObjectRef*> m_refs;
// public:
//     void AddRef(ObjectRef* ref);
//     void RemoveRef(ObjectRef* ref);
//     void ReplaceRef(ObjectRef& from, ObjectRef& to);
//     void ResetAllRefs();
//     void AtomicResetAllRefs();

//     void debugPrint();
// };

// class WorldObjectRef : public ObjectRef
// {
//     void save()
//     {
//         //<< class name
//         //<< index in container
//         //<< ptr to ObjectContainer
//     }
//     void load()
//     {
//         //>> class name
//         //>> index in container
//         //>> ptr to ObjectContainer
//         // ptr = ObjectContainer->get(name, index)
//     }
// };

// class AssetRef : public ObjectRef
// {
//     void save()
//     {
//         //<< class name
//         //<< asset GUID
//         //<< ptr to AssetContainer
//     }
//     void load()
//     {
//         //>> class name
//         //>> asset GUID
//         //>> ptr to AssetContainer
//         // ptr = AssetContainer->getByID(name, GUID)
//         // Reset(ptr)
//     }
// };

// class BaseObjectArray
// {
// public:
//     void Add(Object& object);
//     void Remove(Object& object);
//     Object& Get(unsigned int index);
//     Object& GetByID(unsigned int UniqueID);
// };

// // ObjectType must inherit from Object
// template<typename ObjectType>
// class ObjectArray : public BaseObjectArray
// {
// private:
//     std::vector<ObjectType> m_container;
// public:
//     Object* Get(unsigned int index)
//     {
//         if(index >= 0 && index < m_container.size())
//             return &m_container[index];
//         else
//             return nullptr;
//     }
//     Object& GetByID(unsigned int uniqueID)
//     {
//         auto foundIt = std::find_if(m_container.begin(), m_container.end(), [uniqueID](const ObjectType& item){ return item.GetUniqueID() == uniqueID; });
//         if(foundIt != m_container.end())
//             return &foundIt->second;
//         else
//             return nullptr;
//     }
// };

// class ObjectContainer
// {
// private:
//     std::map<std::string /*Type name*/, std::shared_ptr<BaseObjectArray>> m_mapping;

// public:
//     void Add(Object& object)
//     {
//         std::string initialClassName = object.getMetadatas()->getClassName();
        
//         auto foundIt = m_mapping.find(initialClassName);
//         if(foundIt != m_mapping.end())
//             foundIt->second.Add(object);
//         else
//             m_mapping[initialClassName] = object.getMetadatas()->makeWorldObjectArray();
//     }

//     Object* Get(const std::string& className, unsigned int objectIndex)
//     {       
//         auto foundIt = m_mapping.find(className);
//         if(foundIt != m_mapping.end())
//             return foundIt->second.Get(objectIndex);
//         else
//             return nullptr;
//     }

//     Object* GetByID(const std::string& className, unsigned int objectUniqueId)
//     {
//         auto foundIt = m_mapping.find(className);
//         if(foundIt != m_mapping.end())
//             return foundIt->second.GetByID(objectUniqueId);
//         else
//             return nullptr;
//     }

//     void save()
//     {
//         //<<m_mapping
//     }
//     void load()
//     {
//         //>>m_mapping
//     }
// };