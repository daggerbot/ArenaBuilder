#!/usr/bin/python3
# Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import getopt
import sys
import zipfile_zstd as zipfile

__all__ = [
    'Entry',
    'compile',
]

def fail(message, *args):
    assert isinstance(message, str)
    print("error: " + message.format(*args), file = sys.stderr, flush = True)
    exit(1)

class Entry:
    compression_level = None
    compression_method = None
    name = None
    path = None

    def __init__(self, path, *,
                 compression_level = None,
                 compression_method = None,
                 name = None):

        assert name is None or isinstance(name, str)
        assert isinstance(path, str)

        self.compression_level = compression_level
        self.compression_method = compression_method
        self.name = name or path
        self.path = path

    def parse(s):
        assert isinstance(s, str)

        tokens = s.split(':')

        compression_level = None
        compression_method = None
        name = None
        path = tokens[0]

        if len(tokens) >= 2:
            name = tokens[1]
        if len(tokens) >= 3:
            if tokens[2] == "zstd":
                compression_method = zipfile.ZIP_ZSTANDARD
                compression_level = 19
            else:
                raise ValueError("invalid compression method: {}".format(tokens[2]))
        if len(tokens) >= 4:
            compression_level = int(tokens[3])
        if len(tokens) >= 5:
            raise ValueError("too many pk3 entry parameters")

        return Entry(path,
            compression_level = compression_level,
            compression_method = compression_method,
            name = name)

def compile(out_path, entries):
    assert isinstance(out_path, str)
    assert hasattr(entries, '__iter__')

    with zipfile.ZipFile(out_path, 'w') as archive:
        for entry in entries:
            assert isinstance(entry, Entry)
            archive.write(entry.path, entry.name, entry.compression_method, entry.compression_level)

if __name__ == '__main__':
    out_path = None

    opts, args = getopt.getopt(sys.argv[1:], 'o:')

    for opt, param in opts:
        if opt == '-o':
            if not out_path is None:
                fail("multiple output paths specified")
            out_path = param
        else:
            fail("invalid option: {}", opt)

    if len(args) == 0:
        fail("missing input entries")

    entries = map(lambda s : Entry.parse(s), args)
    compile(out_path, entries)
