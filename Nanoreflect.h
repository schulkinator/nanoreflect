/*
MIT License

Copyright (c) 2022 Sterling Schulkins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef NANO_REFLECT_H_INCLUDED
#define NANO_REFLECT_H_INCLUDED
#include <map>
#include <iterator>
#include <vector>

// workaround for undefined type_info in msvc
#ifdef _MSC_VER
#  include "typeinfo" //cannot use angle brackets here...
#endif

#ifdef _MSC_VER
// visual studio specific compiler warnings
// pointer truncation warning
#pragma warning (push)
#pragma warning(disable : 4311)
#pragma warning(disable : 26495)
#pragma warning(disable : 4302)
#endif

namespace nanoreflect {

  template <typename T>
  struct TypeDescriptor;
  
  template <typename T>
  const TypeDescriptor<T>* GetTypeDescriptor() {
    // all type descriptors live here as static objects in memory
    static TypeDescriptor<T> type_descriptor;
    return &type_descriptor;
  }
  
  struct Member;
  struct TypeDescriptorData {
    const char* type_name; // string name of this type
    size_t size; // size in bytes of this type in memory
    unsigned int type_id; // unique type id of this type, determined at runtime only
    std::vector<Member> members;
    bool finalized; // becomes true once all members have been declared at initialization time (prevents multiple-init)
  };

  struct Member {
    size_t ordinal; // order this member appears in its containing class
    size_t offset; // byte offset of this member in memory from the start of its containing class
    TypeDescriptorData type_data;
    const char* name; // the name of the member in its containing class
    void* type_descriptor; // pointer to the type descriptor for this member, this can be cast to TypeDescriptor<T>* type
  };

  template <typename T> // T is the type this TypeDescriptor represents
  struct TypeDescriptor {
        
    const TypeDescriptorData type_data;
    // lookup that maps offsets to the corresponding Member index. since each member will have a unique offset
    std::map<size_t, size_t> offset_to_member_ordinal;
    
    TypeDescriptor() {      
// workaround for undefined type_info in msvc
#ifdef _MSC_VER
      using ::type_info;
#endif
      TypeDescriptorData* mutable_td_type_data = const_cast<TypeDescriptorData*>(&this->type_data);
      mutable_td_type_data->type_name = typeid(T).name();
      mutable_td_type_data->size = sizeof(T);
      mutable_td_type_data->type_id = reinterpret_cast<unsigned int>(this);
      mutable_td_type_data->finalized = false;
    }
    
    // Only one instance of this class should ever exist in memory. Do not allow copying at all.
    TypeDescriptor(const TypeDescriptor&) = delete;
    TypeDescriptor(const TypeDescriptor&&) = delete;
    TypeDescriptor& operator=(const TypeDescriptor&) = delete;

    template <typename TM> // TM is the Type of the Member
    const Member* GetMember(TM T::* member) {      
      static T object{}; // this can also be constexpr instead of static but that limits us to constexpr constructors
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      return &type_data.members[offset_to_member_ordinal[offset]];
    }

    // Get member by ordinal
    const Member* GetMember(int ordinal) {
      return &type_data.members[ordinal];
    }

    // must be called in the order that the members appear in the structure
    template <typename TM> // TM is the Type of the Member
    void AddMember(TM T::* member, const char* member_name) {      
      static T object{}; // this can also be constexpr instead of static but that limits us to constexpr constructors
      TypeDescriptorData* mutable_td_type_data = const_cast<TypeDescriptorData*>(&this->type_data);
      if (mutable_td_type_data->finalized) { // prevent multiple-init
        return;
      }
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      const TypeDescriptor<TM> *member_type_desc = GetTypeDescriptor<TM>();            
      TypeDescriptorData* mutable_member_type_data = const_cast<TypeDescriptorData*>(&member_type_desc->type_data);
      Member m { mutable_td_type_data->members.size(), offset, *mutable_member_type_data, member_name, const_cast<TypeDescriptor<TM>*>(member_type_desc) };
      mutable_td_type_data->members.push_back(m);
      offset_to_member_ordinal[offset] = m.ordinal;
    }

    void Finalize() {
      TypeDescriptorData* mutable_td_type_data = const_cast<TypeDescriptorData*>(&this->type_data);
      mutable_td_type_data->finalized = true;
    }
  };
  

#define REFLECTED_OBJECT_BEGIN(type_name) \
struct type_name ## _static_typedescriptor_constructor { \
  type_name ## _static_typedescriptor_constructor () { \
    const nanoreflect::TypeDescriptor<type_name>* type_desc = nanoreflect::GetTypeDescriptor<type_name>(); \
    nanoreflect::TypeDescriptor<type_name>* mutable_type_desc = const_cast<nanoreflect::TypeDescriptor<type_name>*>(type_desc);

#define REFLECTED_OBJECT_MEMBER(type_name, name) \
    mutable_type_desc->AddMember(&type_name::name , #name );

#define REFLECTED_OBJECT_END(type_name) \
    mutable_type_desc->Finalize(); \
  } \
} static Static_instance_typedescriptor_constructor_ ## type_name;

};
#if _MSC_VER >= 1200
// visual studio specific compiler warnings
#pragma warning ( pop )
#endif
#endif