# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: config.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


import api_communication.engine_pb2 as engine__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0c\x63onfig.proto\x12\x1b\x63om.wazuh.api.engine.config\x1a\x0c\x65ngine.proto\"0\n\x12RuntimeGet_Request\x12\x11\n\x04name\x18\x01 \x01(\tH\x00\x88\x01\x01\x42\x07\n\x05_name\"\x89\x01\n\x13RuntimeGet_Response\x12\x32\n\x06status\x18\x01 \x01(\x0e\x32\".com.wazuh.api.engine.ReturnStatus\x12\x12\n\x05\x65rror\x18\x02 \x01(\tH\x00\x88\x01\x01\x12\x14\n\x07\x63ontent\x18\x03 \x01(\tH\x01\x88\x01\x01\x42\x08\n\x06_errorB\n\n\x08_content\"R\n\x12RuntimePut_Request\x12\x11\n\x04name\x18\x01 \x01(\tH\x00\x88\x01\x01\x12\x14\n\x07\x63ontent\x18\x02 \x01(\tH\x01\x88\x01\x01\x42\x07\n\x05_nameB\n\n\x08_content\"1\n\x13RuntimeSave_Request\x12\x11\n\x04path\x18\x01 \x01(\tH\x00\x88\x01\x01\x42\x07\n\x05_pathb\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'config_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _RUNTIMEGET_REQUEST._serialized_start=59
  _RUNTIMEGET_REQUEST._serialized_end=107
  _RUNTIMEGET_RESPONSE._serialized_start=110
  _RUNTIMEGET_RESPONSE._serialized_end=247
  _RUNTIMEPUT_REQUEST._serialized_start=249
  _RUNTIMEPUT_REQUEST._serialized_end=331
  _RUNTIMESAVE_REQUEST._serialized_start=333
  _RUNTIMESAVE_REQUEST._serialized_end=382
# @@protoc_insertion_point(module_scope)
