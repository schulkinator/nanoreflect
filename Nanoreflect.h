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

#if _MSC_VER >= 1200
// visual studio specific compiler warnings
// pointer truncation warning
#pragma warning(disable : 4311)
#pragma warning(disable : 26495)
#endif

namespace nanoreflect {

  template <typename T>
  struct TypeDescriptor;
  
  template <typename T>
  TypeDescriptor<T>* GetTypeDescriptor() {
    // all type descriptors live here as static objects in memory
    static TypeDescriptor<T> type_descriptor;
    return &type_descriptor;
  }
  
  struct Member;
  struct TypeDescriptorInfo {
    const char* type_name; // string name of this type
    size_t size; // size in bytes of this type in memory
    unsigned int type_id; // unique type id of this type, determined at runtime only
    std::vector<Member> members;
  };

  struct Member {
    size_t ordinal; // order this member appears in its containing class
    size_t offset; // byte offset of this member in memory from the start of its containing class
    TypeDescriptorInfo type_info;
    const char* name; // the name of the member in its containing class
    void* type_descriptor; // pointer to the type descriptor for this member, this can be cast to TypeDescriptor<T>* type
  };

  template <typename T> // T is the type this TypeDescriptor represents
  struct TypeDescriptor {
        
    TypeDescriptorInfo type_info;
    // lookup that maps offsets to the corresponding Member index. since each member will have a unique offset
    std::map<size_t, size_t> offset_to_member_ordinal;
        
    TypeDescriptor() : type_info{ typeid(T).name(), sizeof(T), reinterpret_cast<unsigned int>(this), {} } {
    }
    
    // Only one instance of this class should ever exist in memory. Do not allow copying at all.
    TypeDescriptor(const TypeDescriptor&) = delete;
    TypeDescriptor(const TypeDescriptor&&) = delete;
    TypeDescriptor& operator=(const TypeDescriptor&) = delete;

    template <typename TM> // TM is the Type of the Member
    const Member* GetMember(TM T::* member) {      
      static T object{}; // this can also be constexpr instead of static but that limits us to constexpr constructors
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      return &type_info.members[offset_to_member_ordinal[offset]];
    }

    // Get member by ordinal
    const Member* GetMember(int ordinal) {
      return &type_info.members[ordinal];
    }

    // must be called in the order that the members appear in the structure
    template <typename TM> // TM is the Type of the Member
    void AddMember(TM T::* member, const char* member_name) {      
      static T object{}; // this can also be constexpr instead of static but that limits us to constexpr constructors
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      TypeDescriptor<TM> *member_type_desc = GetTypeDescriptor<TM>();      
      Member m{ this->type_info.members.size(), offset, member_type_desc->type_info, member_name, member_type_desc };
      this->type_info.members.push_back(m);
      offset_to_member_ordinal[offset] = m.ordinal;
    }
  };
  

#define REFLECTED_OBJECT_BEGIN(type_name) \
struct type_name ## _static_typedescriptor_constructor { \
  type_name ## _static_typedescriptor_constructor () { \
    nanoreflect::TypeDescriptor<type_name>* type_desc = nanoreflect::GetTypeDescriptor<type_name>(); \

#define REFLECTED_OBJECT_MEMBER(type_name, name) \
    type_desc->AddMember(&type_name::name , #name );

#define REFLECTED_OBJECT_END(type_name) \
  } \
} Static_instance_typedescriptor_constructor_ ## type_name;

};
#if _MSC_VER >= 1200
// visual studio specific compiler warnings
#pragma warning ( pop )
#pragma warning ( pop )
#endif
#endif