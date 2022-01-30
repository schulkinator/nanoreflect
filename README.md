# nanoreflect
An extremely simple C++ reflection library. Mostly useful for vertex struct layout descriptions in graphics programs and games.

## Example usage:
```
// Declare your structure as usual
#pragma pack(1)
struct Vertex {
  glm::vec3 pos;
  glm::vec2 uv;
  glm::vec3 normal;
};

// Then you must define your structure's type descriptor
REFLECTED_OBJECT_BEGIN(Vertex)
REFLECTED_OBJECT_MEMBER(Vertex, pos)
REFLECTED_OBJECT_MEMBER(Vertex, uv)
REFLECTED_OBJECT_MEMBER(Vertex, normal)
REFLECTED_OBJECT_END(Vertex)

// you can even use it with inherited objects
#pragma pack(1)
struct VertexWithTangent : public Vertex {
  glm::vec3 tangent;
};

// Define VertexWithTangent's type descriptor
// NOTE: You MUST list out the inherited fields again
REFLECTED_OBJECT_BEGIN(VertexWithTangent)
REFLECTED_OBJECT_MEMBER(VertexWithTangent, pos)
REFLECTED_OBJECT_MEMBER(VertexWithTangent, uv)
REFLECTED_OBJECT_MEMBER(VertexWithTangent, normal)
REFLECTED_OBJECT_MEMBER(VertexWithTangent, tangent)
REFLECTED_OBJECT_END(VertexWithTangent)

// Then to use the reflection in code you can do something like this (the Member object has the reflected field information):
nanoreflect::TypeDescriptor<Vertex>* type_desc = nanoreflect::GetTypeDescriptor<Vertex>();
nanoreflect::Member* member_pos = type_desc->GetMember(&Vertex::pos);
nanoreflect::Member* member_uv = type_desc->GetMember(&Vertex::uv);
nanoreflect::Member* member_normal = type_desc->GetMember(&Vertex::normal);
  
// Here's a real example using it with OpenGL to set up a VBO:
 void SetupBuffers() {
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glGenVertexArrays(1, &vaoId_);
  glBindVertexArray(vaoId_);

  nanoreflect::TypeDescriptor<Vertex>* type_desc = nanoreflect::GetTypeDescriptor<Vertex>();
  nanoreflect::Member* member_pos = type_desc->GetMember(&Vertex::pos);
  nanoreflect::Member* member_uv = type_desc->GetMember(&Vertex::uv);
  nanoreflect::Member* member_normal = type_desc->GetMember(&Vertex::normal);
  
  GLuint positionAttribLocation = member_pos->ordinal;
  glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(member_pos->offset)); // stride is in units of bytes, offset is also in bytes
  glEnableVertexAttribArray(positionAttribLocation);
  
  GLuint texcoordAttribLocation = member_uv->ordinal;
  glVertexAttribPointer(texcoordAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(member_uv->offset)); // stride is in units of bytes, offset is also in bytes
  glEnableVertexAttribArray(texcoordAttribLocation);
  
  GLuint normalAttribLocation = member_normal->ordinal;
  glVertexAttribPointer(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(member_normal->offset)); // stride is in units of bytes, offset is also in bytes
  glEnableVertexAttribArray(normalAttribLocation);
  
  // generate the index buffer
  glGenBuffers(1, &index_buffer_id_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
}

// Here's a different more generic and compact version of the above by iterating over the members
void SetupBuffers(std::vector<Vertex>& vertices, std::vector<uint16_t> indices, GLuint& vbo_id, GLuint& vao_id, GLuint& index_buffer_id) {
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  nanoreflect::TypeDescriptor<Vertex>* type_desc = nanoreflect::GetTypeDescriptor<Vertex>();
  nanoreflect::MemberList& members = type_desc->GetMemberList();

  for (int i = 0; i < members.size(); ++i) {
    const nanoreflect::Member& member = members[i];
    GLuint attribLocation = member.ordinal;
    size_t num_floats = member.type_info.size / sizeof(float);
    printf("field type: %s, field name: %s, type_id: %d\n", member.type_info.type_name, member.name, member.type_info.type_id);
    glVertexAttribPointer(attribLocation, num_floats, GL_FLOAT, GL_FALSE, sizeof(TV), BUFFER_OFFSET(member.offset)); // stride is in units of bytes, offset is also in bytes
    glEnableVertexAttribArray(attribLocation);
  }
    
  // generate the index buffer
  glGenBuffers(1, &index_buffer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
}

 ```
