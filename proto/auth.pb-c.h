/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: auth.proto */

#ifndef PROTOBUF_C_auth_2eproto__INCLUDED
#define PROTOBUF_C_auth_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1001001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Authpb__User Authpb__User;
typedef struct _Authpb__Permission Authpb__Permission;
typedef struct _Authpb__Role Authpb__Role;


/* --- enums --- */

typedef enum _Authpb__Permission__Type {
  AUTHPB__PERMISSION__TYPE__READ = 0,
  AUTHPB__PERMISSION__TYPE__WRITE = 1,
  AUTHPB__PERMISSION__TYPE__READWRITE = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(AUTHPB__PERMISSION__TYPE)
} Authpb__Permission__Type;

/* --- messages --- */

/*
 * User is a single entry in the bucket authUsers
 */
struct  _Authpb__User
{
  ProtobufCMessage base;
  protobuf_c_boolean has_name;
  ProtobufCBinaryData name;
  protobuf_c_boolean has_password;
  ProtobufCBinaryData password;
  size_t n_roles;
  char **roles;
};
#define AUTHPB__USER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&authpb__user__descriptor) \
    , 0,{0,NULL}, 0,{0,NULL}, 0,NULL }


/*
 * Permission is a single entity
 */
struct  _Authpb__Permission
{
  ProtobufCMessage base;
  protobuf_c_boolean has_key;
  ProtobufCBinaryData key;
  protobuf_c_boolean has_permtype;
  Authpb__Permission__Type permtype;
};
#define AUTHPB__PERMISSION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&authpb__permission__descriptor) \
    , 0,{0,NULL}, 0,0 }


/*
 * Role is a single entry in the bucket authRoles
 */
struct  _Authpb__Role
{
  ProtobufCMessage base;
  protobuf_c_boolean has_name;
  ProtobufCBinaryData name;
  size_t n_keypermission;
  Authpb__Permission **keypermission;
};
#define AUTHPB__ROLE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&authpb__role__descriptor) \
    , 0,{0,NULL}, 0,NULL }


/* Authpb__User methods */
void   authpb__user__init
                     (Authpb__User         *message);
size_t authpb__user__get_packed_size
                     (const Authpb__User   *message);
size_t authpb__user__pack
                     (const Authpb__User   *message,
                      uint8_t             *out);
size_t authpb__user__pack_to_buffer
                     (const Authpb__User   *message,
                      ProtobufCBuffer     *buffer);
Authpb__User *
       authpb__user__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   authpb__user__free_unpacked
                     (Authpb__User *message,
                      ProtobufCAllocator *allocator);
/* Authpb__Permission methods */
void   authpb__permission__init
                     (Authpb__Permission         *message);
size_t authpb__permission__get_packed_size
                     (const Authpb__Permission   *message);
size_t authpb__permission__pack
                     (const Authpb__Permission   *message,
                      uint8_t             *out);
size_t authpb__permission__pack_to_buffer
                     (const Authpb__Permission   *message,
                      ProtobufCBuffer     *buffer);
Authpb__Permission *
       authpb__permission__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   authpb__permission__free_unpacked
                     (Authpb__Permission *message,
                      ProtobufCAllocator *allocator);
/* Authpb__Role methods */
void   authpb__role__init
                     (Authpb__Role         *message);
size_t authpb__role__get_packed_size
                     (const Authpb__Role   *message);
size_t authpb__role__pack
                     (const Authpb__Role   *message,
                      uint8_t             *out);
size_t authpb__role__pack_to_buffer
                     (const Authpb__Role   *message,
                      ProtobufCBuffer     *buffer);
Authpb__Role *
       authpb__role__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   authpb__role__free_unpacked
                     (Authpb__Role *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Authpb__User_Closure)
                 (const Authpb__User *message,
                  void *closure_data);
typedef void (*Authpb__Permission_Closure)
                 (const Authpb__Permission *message,
                  void *closure_data);
typedef void (*Authpb__Role_Closure)
                 (const Authpb__Role *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor authpb__user__descriptor;
extern const ProtobufCMessageDescriptor authpb__permission__descriptor;
extern const ProtobufCEnumDescriptor    authpb__permission__type__descriptor;
extern const ProtobufCMessageDescriptor authpb__role__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_auth_2eproto__INCLUDED */
