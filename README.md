# Shader Parser

## Description

A simple recursive descent parser to get important information out of a GLSL source file required by rendering engines. Currently only supports getting data associated with expressions prefixed by a layout qualifier, with the primary purpose currently being to get variable information from Uniform Buffers and Shader Storage Buffers.

Disclaimer: This is both incomplete and not fully representative of GLSL grammar. The primary purpose of this parser is to augment my personal rendering engine.