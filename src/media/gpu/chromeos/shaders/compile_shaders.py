# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Wrapper script for compiling Vulkan shaders.

Vulkan shaders need to be compiled ahead of time rather than on the fly. This
script will do that and wrap the SPIR-V bytecode in a C++ header so it can be
included in Chrome.

Usage:
compile_shaders.py <directory to compile>

This script will automatically compile all .frag and .vert files in the target
directory and output a C header file to stdout containing all of the bytecode.
"""

from collections.abc import Sequence

from absl import app

import glob
import os
import subprocess

def execute_glslang_validator(filepath):
  return subprocess.run(
      ['glslangValidator', '-V100', '-x', '-o', '/dev/stdout', filepath],
      capture_output=True, check=True).stdout.decode()


def parse_glslang_output(output):
  word_size_bytes = 4
  word_size_hex_string = 2*word_size_bytes + 2 # includes preceding '0x'
  bytecode_words = []
  idx = 0
  while True:
    idx = output.find('0x', idx)
    if idx == -1:
      return bytecode_words
    bytecode_words.append(output[idx:idx+word_size_hex_string])
    idx += word_size_hex_string


def generate_shader_literal(filepath, bytecode_words):
  column_width = 80
  indent = '    '
  num_words_per_line = int((column_width - len(indent))
                           / (len(bytecode_words[0]) + 2))

  filename = os.path.basename(filepath)
  name, ext = os.path.splitext(filename)

  literal = 'constexpr uint32_t k' + name + ext[1:].capitalize() + '[] = {'
  for i in range(0, len(bytecode_words)):
    if i % num_words_per_line == 0:
      if i != 0:
        literal += ','
      literal += '\n' + indent + bytecode_words[i]
    else:
      literal += ', ' + bytecode_words[i]
  literal += ',\n};\n'

  # Add shader source to the end
  with open(filepath, 'r') as shader_source:
    literal += '// clang-format off\n'
    literal += '/***\n'
    literal += filename + ' source:\n'
    literal += shader_source.read()
    literal += '***/\n'
    literal += '// clang-format on\n\n'
  return literal


def get_targets(directory_path):
  if directory_path[-1] != '/':
    directory_path += '/'
  ret = [glob.glob(directory_path + e) for e in ['*.vert', '*.frag']]
  ret = ret[0] + ret[1]
  ret.sort()
  return ret


def main(argv: Sequence[str]) -> None:
  if len(argv) > 2:
    raise app.UsageError('Too many command-line arguments.')
  if len(argv) < 2:
    raise app.UsageError('Too few command-line arguments.')

  if not os.path.isdir(argv[1]):
    raise app.UsageError('No such directory: ' + argv[1])

  targets = get_targets(argv[1])

  output =  '// Copyright 2023 The Chromium Authors\n' \
            '// Use of this source code is governed by ' \
            'a BSD-style license that can be\n' \
            '// found in the LICENSE file.\n\n' \
            '// DO NOT EDIT\n' \
            '// This file is automatically generated by ' \
            'the compile_shaders.py script\n\n' \
            '#ifndef MEDIA_GPU_CHROMEOS_SHADERS_SHADERS_H_\n' \
            '#define MEDIA_GPU_CHROMEOS_SHADERS_SHADERS_H_\n\n' \
            '#include <stdint.h>\n\n' \
            'namespace {\n\n'
  for target in targets:
    compiler_output = execute_glslang_validator(target)
    bytecode = parse_glslang_output(compiler_output)
    output += generate_shader_literal(target, bytecode)
  output += '}  // namespace\n\n'
  output += '#endif'

  print(output)

if __name__ == '__main__':
  app.run(main)
