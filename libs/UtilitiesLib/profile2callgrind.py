#!/usr/bin/env python

import os
import operator
import glob
import csv
import optparse
import urllib
from ctypes import *

windll.dbghelp.SymInitialize.argtypes = [c_void_p, c_char_p, c_int]
windll.dbghelp.SymInitialize.restype = c_ulong
windll.dbghelp.SymLoadModuleEx.argtypes = [c_void_p, c_void_p, c_char_p, c_char_p, c_ulonglong, c_ulong, c_void_p, c_ulong]
windll.dbghelp.SymLoadModuleEx.restype = c_ulonglong
windll.dbghelp.SymGetModuleInfo64.argtypes = [c_void_p, c_ulonglong, c_void_p]
windll.dbghelp.SymGetModuleInfo64.restype = c_ulong
windll.dbghelp.SymFromAddr.argtypes = [c_void_p, c_ulonglong, c_void_p, c_void_p]
windll.dbghelp.SymFromAddr.restype = c_ulong

class IMAGEHLP_MODULE64(Structure):
    _fields_ = [
        ('SizeOfStruct', c_ulong),
        ('BaseOfImage', c_ulonglong),
        ('ImageSize', c_ulong),
        ('TimeDateStamp', c_ulong),
        ('CheckSum', c_ulong),
        ('NumSyms', c_ulong),
        ('SymType', c_ulong), #SYM_TYPE
        ('ModuleName', c_char*32),
        ('ImageName', c_char*256),
        ('LoadedImageName', c_char*256),
        ('LoadedPdbName', c_char*256),
        ('CVSig', c_ulong),
        ('CVData', c_char*260*3),
        ('PdbSig', c_ulong),
        ('PdbSig70', c_ulong), #GUID
        ('PdbAge', c_ulong),
        ('PdbUnmatched', c_ulong),
        ('DbgUnmatched', c_ulong),
        ('LineNumbers', c_ulong),
        ('GlobalSymbols', c_ulong),
        ('TypeInfo', c_ulong),
        ('SourceIndexed', c_ulong),
        ('Publics', c_ulong),
        ]

class SYMBOL_INFO(Structure):
    _fields_ = [
        ('SizeOfStruct', c_ulong),
        ('TypeIndex', c_ulong),
        ('Reserved', c_ulonglong * 2),
        ('Index', c_ulong),
        ('Size', c_ulong),
        ('ModBase', c_ulonglong),
        ('Flags', c_ulong),
        ('Value', c_ulonglong),
        ('Address', c_ulonglong),
        ('Register', c_ulong),
        ('Scope', c_ulong),
        ('Tag', c_ulong),
        ('NameLen', c_ulong),
        ('MaxNameLen', c_ulong),
        ('Name', c_char*1024),
        ]

SYMINDEX = 0
class Symbols:
    def __init__(self):
        global SYMINDEX

        self.modules = {}
        self.table = {}

        SYMINDEX += 1
        self.sym = SYMINDEX
        assert windll.dbghelp.SymInitialize(self.sym, None, 0), (self, self.sym,)

    def load(self, name, base, size):
        ret = windll.dbghelp.SymLoadModuleEx(self.sym, None, name, None, base, size, None, 0)
        if not ret:
            print 'Could not load symbols for %s' % name
            self.modules[base] = name
            return
        self.modules[ret] = name

        #mi = IMAGEHLP_MODULE64()
        #mi.SizeOfStruct = sizeof(mi)
        #windll.dbghelp.SymGetModuleInfo64(self.sym, ret, byref(mi))
        #assert mi.ImageSize == size, (self, mi.ImageSize, size)
        #self.modules[ret] = mi.LoadedImageName

    def find(self, addr):
        s = self.table.get(addr, None)
        if s:
            return s

        si = SYMBOL_INFO()
        si.SizeOfStruct = 88
        si.MaxNameLen = 1024
        found = windll.dbghelp.SymFromAddr(self.sym, addr, 0, byref(si))
        if not found:
            assert windll.kernel32.GetLastError() != 87, (self,)
            s = '0x%x' % addr
        elif si.Address != addr:
            if si.ModBase:
                s = '0x%x@%s' % (addr, self.modules[si.ModBase])
            else:
                s = '0x%x' % addr
        else:
            s = si.Name

        self.table[addr] = s
        return s

class Call:
    caller = None
    callee = None
    name = None
    inclusive = None
    exclusive = None
    children = None
    calls = None

    def __init__(self, caller, callee, name, inclusive=None, exclusive=None, calls=1, children=None):
        self.caller = caller
        self.callee = callee
        self.name = name
        self.inclusive = inclusive
        self.exclusive = exclusive
        self.calls = calls
        self.children = children or []

    def postcalc(self):
        childtime = 0
        for child in self.children:
            assert self.callee == child.caller, (self, self.callee, child.caller)
            child.postcalc()
            childtime += child.inclusive

        if self.inclusive is None:
            self.inclusive = childtime
            self.exclusive = 0
        else:
            assert self.inclusive >= childtime, (self, self.inclusive, childtime)
            self.exclusive = self.inclusive - childtime

    def cloneshallow(self):
        return Call(self.caller, self.callee, self.name, self.inclusive, self.exclusive, self.calls, list(self.children))

    def mergein(self, other):
        assert self.callee == other.callee, (self, self.callee, other.callee)
        assert self.name == other.name, (self, self.name, other.name)
        self.exclusive += other.exclusive
        self.inclusive += other.inclusive
        self.calls += other.calls
        self.children.extend(other.children)

    def allfunctions(self, dict):
        entry = dict.get(self.callee, None)
        if entry:
            entry.mergein(self)
        else:
            dict[self.callee] = self.cloneshallow()

        for child in self.children:
            child.allfunctions(dict)

    def writeitem(self, f):
        f.write('%10d %10d %10d %s\n' % (
            self.inclusive,
            self.exclusive,
            self.calls,
            self.name))

    def writetree(self, f, total=None, indent=0):
        if total is None:
            total = float(self.inclusive)

        f.write('%10d %10d %9.2f%% %9.2f%% %s %s\n' % (
            self.inclusive,
            self.exclusive,
            float(self.inclusive)/total*100.0,
            float(self.exclusive)/total*100.0,
            '-'*indent, self.name))

        for child in self.children:
            child.writetree(f, total, indent+1)

    def merged(self):
        childdict = {}
        for child in self.children:
            owner = childdict.get(child.callee, None)
            if owner is None:
                childdict[child.callee] = child.cloneshallow()
            else:
                assert owner.caller == child.caller, (self, owner.caller, child.caller)
                owner.mergein(child)
        return Call(self.caller, self.callee, self.name, self.inclusive, self.exclusive, self.calls, [x.merged() for x in childdict.values()])

    def gennames(self):
        self.grindname = self.name
        for child in self.children:
            child.gennames()

    def gengrindnames(self, nametable={}):
        i = nametable.get(self.name, None)
        if i is None:
            self.grindname = self.name
            nametable[self.name] = 1
        else:
            self.grindname = self.name + "'%d" % i
            nametable[self.name] = i+1

        for child in self.children:
            child.gengrindnames(nametable)

    def writegrind(self, f):
        f.write('\nfn=%s\n0 %d\n' % (self.grindname, self.exclusive))

        for child in self.children:
            f.write('cfn=%s\ncalls=%d 0\n0 %d\n' % (child.grindname, child.calls, child.inclusive))

        for child in self.children:
            child.writegrind(f)

    def __repr__(self):
        return '<Call %s from %x with %d children>' % (self.name, self.caller, len(self.children))

def convert(src, dstpath):
    basename = os.path.basename(src)

    tree = os.path.join(dstpath, basename)
    ftree = open(tree, 'w')

    c = csv.reader(open(src))

    # Load symbols
    syms = Symbols()
    line = 0
    for entry in c:
        line+=1
        if entry[0] == '----------':
            break

        name = entry[0]
        base = long(entry[1], 16)
        size = long(entry[2], 16)

        ftree.write('Loading module %s @ 0x%x (0x%x)\n' % (name, base, size))
        syms.load(name, base, size)

    # Read call data
    threads = {}
    entries = []
    for row in c:
        line+=1
        rid = int(row[0])
        parent = int(row[1])
        tid = int(row[2])
        caller = long(row[3], 16)
        callee = long(row[4], 16)
        elapsed = long(row[5])
        name = syms.find(callee)

        entry = Call(caller, callee, name, elapsed or None)

        if parent == -1:
            if callee == 0:
                # A function exited that we only caught the end of, assume the previous entry in the stack is our child
                child = threads.get(tid, None)
                if not child:
                    # This thread hasn't produced anything useful yet
                    entries.append(None)
                    continue
                assert not child.inclusive, (line, child.inclusive)
                assert not elapsed, (line, elapsed)

                assert child.callee != entry.caller, (line, child.callee, entry.caller)
                threads[tid] = entry = Call(None, caller, syms.find(caller), children=[child])

                assert not child.caller, (line, child.caller)
                child.caller = entry.callee
            else:
                parent = threads.get(tid, None)
                if not parent:
                    parent = Call(None, caller, syms.find(caller), children=[entry])
                    threads[tid] = parent
                else:
                    if parent.callee != entry.caller:
                        # insert a placeholder in the middle
                        parent.children.append(Call(parent.callee, entry.caller, syms.find(entry.caller), children=[entry]))
                    else:
                        parent.children.append(entry)
        else:
            parent = entries[parent]
            if parent.callee != entry.caller:
                # insert a placeholder in the middle
                parent.children.append(Call(parent.callee, entry.caller, syms.find(entry.caller), children=[entry]))
            else:
                parent.children.append(entry)

        assert(len(entries) == rid), (line, len(entries), rid)
        entries.append(entry)

    # Calculate any missing items
    for tid, root in threads.items():
        assert not root.caller, (tid, root.caller)
        root.postcalc()

    funcs = {}
    for tid, root in threads.items():
        root.allfunctions(funcs)
    funcs = funcs.values()

    ftree.write('\n%s top calls\n' % ('='*40))
    ftree.write('%10s %10s %10s\n\n' % ('inclusive', 'exclusive', 'calls'))
    funcs.sort(key=operator.attrgetter('calls'),reverse=True)
    for func in funcs[:100]:
        func.writeitem(ftree)

    ftree.write('\n%s top exclusive\n' % ('='*40))
    ftree.write('%10s %10s %10s\n\n' % ('inclusive', 'exclusive', 'calls'))
    funcs.sort(key=operator.attrgetter('exclusive'),reverse=True)
    for func in funcs[:100]:
        func.writeitem(ftree)

    ftree.write('\n%s top inclusive\n' % ('='*40))
    ftree.write('%10s %10s %10s\n\n' % ('inclusive', 'exclusive', 'calls'))
    funcs.sort(key=operator.attrgetter('inclusive'),reverse=True)
    for func in funcs[:100]:
        func.writeitem(ftree)

    # Write call tree data
    for tid, root in threads.items():
        ftree.write('\n%s thread %d\n' % ('='*40, tid))
        ftree.write('%10s %10s %9s%% %9s%%\n\n' % ('inclusive', 'exclusive', 'inclusive', 'exclusive'))
        root.writetree(ftree)

    # Write callgrind data
    for tid, root in threads.items():
        merged = root.merged()

        clean_name = urllib.quote_plus(root.name)

        call = os.path.join(dstpath, 'cachegrind.out.%s_%d_merged_%s' % (os.path.splitext(basename)[0], tid, clean_name))
        fcall = open(call, 'wb')
        fcall.write('events: TSC\n')
        merged.gennames()
        merged.writegrind(fcall)

        call = os.path.join(dstpath, 'cachegrind.out.%s_%d_split_%s' % (os.path.splitext(basename)[0], tid, clean_name))
        fcall = open(call, 'wb')
        fcall.write('events: TSC\n')
        merged.gengrindnames()
        merged.writegrind(fcall)

def main():
    parser = optparse.OptionParser()
    opts, args = parser.parse_args()

    for arg in args:
        for src in glob.glob(arg):
            dst = os.path.join(os.path.dirname(src), 'profiles')
            if not os.path.exists(dst):
                os.makedirs(dst)
            print 'Processing %s to %s' % (src, dst)
            convert(src, dst)

if __name__ == '__main__':
    main()
