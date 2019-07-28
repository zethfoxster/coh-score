/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef ApplicationCacheStorage_h
#define ApplicationCacheStorage_h

#if ENABLE(OFFLINE_WEB_APPLICATIONS)

#include "PlatformString.h"
#include "SQLiteDatabase.h"
#include "StringHash.h"

#include <wtf/HashCountedSet.h>

namespace WebCore {

class ApplicationCache;
class ApplicationCacheHost;
class ApplicationCacheGroup;
class ApplicationCacheResource;
class KURL;
template <class T>
class StorageIDJournal;

class ApplicationCacheStorage : public Noncopyable {
public:
    void setCacheDirectory(const String&);
    const String& cacheDirectory() const;
    
    void setMaximumSize(int64_t size);
    int64_t maximumSize() const;
    bool isMaximumSizeReached() const;
    int64_t spaceNeeded(int64_t cacheToSave);

    ApplicationCacheGroup* cacheGroupForURL(const KURL&); // Cache to load a main resource from.
    ApplicationCacheGroup* fallbackCacheGroupForURL(const KURL&); // Cache that has a fallback entry to load a main resource from if normal loading fails.

    ApplicationCacheGroup* findOrCreateCacheGroup(const KURL& manifestURL);
    void cacheGroupDestroyed(ApplicationCacheGroup*);
    void cacheGroupMadeObsolete(ApplicationCacheGroup*);
        
    bool storeNewestCache(ApplicationCacheGroup*); // Updates the cache group, but doesn't remove old cache.
    bool store(ApplicationCacheResource*, ApplicationCache*);
    bool storeUpdatedType(ApplicationCacheResource*, ApplicationCache*);

    // Removes the group if the cache to be removed is the newest one (so, storeNewestCache() needs to be called beforehand when updating).
    void remove(ApplicationCache*);
    
    void empty();
    
    static bool storeCopyOfCache(const String& cacheDirectory, ApplicationCacheHost*);

    bool manifestURLs(Vector<KURL>* urls);
    bool cacheGroupSize(const String& manifestURL, int64_t* size);
    bool deleteCacheGroup(const String& manifestURL);
    void vacuumDatabaseFile();
private:
    ApplicationCacheStorage();
    PassRefPtr<ApplicationCache> loadCache(unsigned storageID);
    ApplicationCacheGroup* loadCacheGroup(const KURL& manifestURL);
    
    typedef StorageIDJournal<ApplicationCacheResource> ResourceStorageIDJournal;
    typedef StorageIDJournal<ApplicationCacheGroup> GroupStorageIDJournal;

    bool store(ApplicationCacheGroup*, GroupStorageIDJournal*);
    bool store(ApplicationCache*, ResourceStorageIDJournal*);
    bool store(ApplicationCacheResource*, unsigned cacheStorageID);

    void loadManifestHostHashes();
    
    void verifySchemaVersion();
    
    void openDatabase(bool createIfDoesNotExist);
    
    bool executeStatement(SQLiteStatement&);
    bool executeSQLCommand(const String&);

    void checkForMaxSizeReached();
    
    String m_cacheDirectory;
    String m_cacheFile;

    int64_t m_maximumSize;
    bool m_isMaximumSizeReached;

    SQLiteDatabase m_database;

    // In order to quickly determine if a given resource exists in an application cache,
    // we keep a hash set of the hosts of the manifest URLs of all non-obsolete cache groups.
    HashCountedSet<unsigned, AlreadyHashed> m_cacheHostSet;
    
    typedef HashMap<String, ApplicationCacheGroup*> CacheGroupMap;
    CacheGroupMap m_cachesInMemory; // Excludes obsolete cache groups.

    friend ApplicationCacheStorage& cacheStorage();
};
 
ApplicationCacheStorage& cacheStorage();
    
} // namespace WebCore

#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)

#endif // ApplicationCacheStorage_h
