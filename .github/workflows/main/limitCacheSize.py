import os
import collections

# 2Gb. In practice, this compresses down to more like 400Mb when it gets
# archived for upload to GitHub. The limit for all caches is 5Gb.
sizeLimit = 2 * 1024 * 1024 * 1024

CacheEntry = collections.namedtuple( "CacheEntry", [ "file", "size", "mtime" ] )

totalSize = 0
cacheEntries = []

for root, dirs, files in os.walk( "./sconsCache" ) :
	for file in files :
		fileName = os.path.join( root, file )
		size = os.path.getsize( fileName )
		totalSize += size
		cacheEntries.append(
			CacheEntry(
				fileName,
				size,
				os.path.getmtime( fileName ),
			)
		)

cacheEntries = sorted( cacheEntries, key = lambda x : x.mtime )
for c in cacheEntries :
	if totalSize < sizeLimit :
		break
	os.remove( c.file )
	totalSize -= c.size
