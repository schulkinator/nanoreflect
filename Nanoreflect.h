#ifndef NANO_REFLECT_H_INCLUDED
#define NANO_REFLECT_H_INCLUDED
#include <map>
namespace nanoreflect {

  template <typename T>
  struct TypeDescriptor;
    
  struct Member {
    size_t ordinal;
    size_t offset;
    const char* name;    
    void* type_descriptor; // this can be cast to TypeDescriptor<T>* type
  };

  template <typename T> // T is the type this TypeDescriptor represents
  struct TypeDescriptor {
    const char* name;
    size_t size;    
    // maps offsets to the corresponding Member. since each member will have a unique offset
    std::map<size_t, Member> offset_to_members;
    size_t next_member_ordinal;
    
    TypeDescriptor() : name(typeid(T).name()), size(sizeof(T)), next_member_ordinal(0) {
    }
    
    template <typename TM> // TM is the Type of the Member
    Member* GetMember(TM T::* member) {      
      static T object{}; // this can also be constexpr but that limits us to constexpr constructors
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      return &offset_to_members[offset];
    }

    // must be called in the order that the members appear in the structure
    template <typename TM> // TM is the Type of the Member
    void AddMember(TM T::* member, const char* member_name) {      
      static T object{}; // this can also be constexpr but that limits us to constexpr constructors
      size_t offset = size_t(&(object.*member)) - size_t(&object);
      Member m{ next_member_ordinal++, offset, member_name, this };
      offset_to_members[offset] = m;
    }
  };

  template <typename T>
  TypeDescriptor<T>* GetTypeDescriptor() {
    // all type descriptors live here as static objects in memory
    static TypeDescriptor<T> type_descriptor;
    return &type_descriptor;
  }

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
#endif