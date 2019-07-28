import pypig

pypig.find_piggs()
print 'Found %d files in %d piggs' % (len(pypig.get_pigg_files()), len(pypig.get_piggs()))

try:
    f = pypig.pigopen('__missing.txt')
    assert not f, f
except pypig.PigIOError, e:
    print 'Got pig exception: %s' % e

f = pypig.pigopen('Defs/badges.def')
assert f, f
a = f.read()[:4096]
assert '\r' not in a, repr(a)
f.seek(0)
b = f.read(4096)
assert a==b, (a, b)
f.close()
print 'Got %d bytes from file in text mode' % len(a)

f = pypig.pigopen('texture_library/Refmap_01.texture', 'rb')
assert f, f
a = f.read()[:4096]
f.seek(0)
b = f.read(4096)
assert a==b, (a, b)
del f
print 'Got %d bytes from file in binary mode' % len(a)

