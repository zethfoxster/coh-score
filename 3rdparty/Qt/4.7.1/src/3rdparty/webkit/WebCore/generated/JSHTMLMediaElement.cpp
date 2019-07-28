/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(VIDEO)

#include "JSHTMLMediaElement.h"

#include "HTMLMediaElement.h"
#include "JSMediaError.h"
#include "JSTimeRanges.h"
#include "KURL.h"
#include "MediaError.h"
#include "TimeRanges.h"
#include <runtime/Error.h>
#include <runtime/JSNumberCell.h>
#include <runtime/JSString.h>
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSHTMLMediaElement);

/* Hash table */

static const HashTableValue JSHTMLMediaElementTableValues[27] =
{
    { "error", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementError), (intptr_t)0 },
    { "src", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementSrc), (intptr_t)setJSHTMLMediaElementSrc },
    { "currentSrc", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementCurrentSrc), (intptr_t)0 },
    { "networkState", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNetworkState), (intptr_t)0 },
    { "preload", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementPreload), (intptr_t)setJSHTMLMediaElementPreload },
    { "buffered", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementBuffered), (intptr_t)0 },
    { "readyState", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementReadyState), (intptr_t)0 },
    { "seeking", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementSeeking), (intptr_t)0 },
    { "currentTime", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementCurrentTime), (intptr_t)setJSHTMLMediaElementCurrentTime },
    { "startTime", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementStartTime), (intptr_t)0 },
    { "duration", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementDuration), (intptr_t)0 },
    { "paused", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementPaused), (intptr_t)0 },
    { "defaultPlaybackRate", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementDefaultPlaybackRate), (intptr_t)setJSHTMLMediaElementDefaultPlaybackRate },
    { "playbackRate", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementPlaybackRate), (intptr_t)setJSHTMLMediaElementPlaybackRate },
    { "played", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementPlayed), (intptr_t)0 },
    { "seekable", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementSeekable), (intptr_t)0 },
    { "ended", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementEnded), (intptr_t)0 },
    { "autoplay", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementAutoplay), (intptr_t)setJSHTMLMediaElementAutoplay },
    { "loop", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementLoop), (intptr_t)setJSHTMLMediaElementLoop },
    { "controls", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementControls), (intptr_t)setJSHTMLMediaElementControls },
    { "volume", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementVolume), (intptr_t)setJSHTMLMediaElementVolume },
    { "muted", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementMuted), (intptr_t)setJSHTMLMediaElementMuted },
    { "webkitPreservesPitch", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementWebkitPreservesPitch), (intptr_t)setJSHTMLMediaElementWebkitPreservesPitch },
    { "webkitHasClosedCaptions", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementWebkitHasClosedCaptions), (intptr_t)0 },
    { "webkitClosedCaptionsVisible", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementWebkitClosedCaptionsVisible), (intptr_t)setJSHTMLMediaElementWebkitClosedCaptionsVisible },
    { "constructor", DontEnum|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementConstructor), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLMediaElementTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 1023, JSHTMLMediaElementTableValues, 0 };
#else
    { 69, 63, JSHTMLMediaElementTableValues, 0 };
#endif

/* Hash table for constructor */

static const HashTableValue JSHTMLMediaElementConstructorTableValues[11] =
{
    { "NETWORK_EMPTY", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_EMPTY), (intptr_t)0 },
    { "NETWORK_IDLE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_IDLE), (intptr_t)0 },
    { "NETWORK_LOADING", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_LOADING), (intptr_t)0 },
    { "NETWORK_LOADED", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_LOADED), (intptr_t)0 },
    { "NETWORK_NO_SOURCE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_NO_SOURCE), (intptr_t)0 },
    { "HAVE_NOTHING", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_NOTHING), (intptr_t)0 },
    { "HAVE_METADATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_METADATA), (intptr_t)0 },
    { "HAVE_CURRENT_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_CURRENT_DATA), (intptr_t)0 },
    { "HAVE_FUTURE_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_FUTURE_DATA), (intptr_t)0 },
    { "HAVE_ENOUGH_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_ENOUGH_DATA), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLMediaElementConstructorTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 63, JSHTMLMediaElementConstructorTableValues, 0 };
#else
    { 33, 31, JSHTMLMediaElementConstructorTableValues, 0 };
#endif

class JSHTMLMediaElementConstructor : public DOMConstructorObject {
public:
    JSHTMLMediaElementConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
        : DOMConstructorObject(JSHTMLMediaElementConstructor::createStructure(globalObject->objectPrototype()), globalObject)
    {
        putDirect(exec->propertyNames().prototype, JSHTMLMediaElementPrototype::self(exec, globalObject), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    virtual bool getOwnPropertyDescriptor(ExecState*, const Identifier&, PropertyDescriptor&);
    virtual const ClassInfo* classInfo() const { return &s_info; }
    static const ClassInfo s_info;

    static PassRefPtr<Structure> createStructure(JSValue proto) 
    { 
        return Structure::create(proto, TypeInfo(ObjectType, StructureFlags), AnonymousSlotCount); 
    }
    
protected:
    static const unsigned StructureFlags = OverridesGetOwnPropertySlot | ImplementsHasInstance | DOMConstructorObject::StructureFlags;
};

const ClassInfo JSHTMLMediaElementConstructor::s_info = { "HTMLMediaElementConstructor", 0, &JSHTMLMediaElementConstructorTable, 0 };

bool JSHTMLMediaElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLMediaElementConstructor, DOMObject>(exec, &JSHTMLMediaElementConstructorTable, this, propertyName, slot);
}

bool JSHTMLMediaElementConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSHTMLMediaElementConstructor, DOMObject>(exec, &JSHTMLMediaElementConstructorTable, this, propertyName, descriptor);
}

/* Hash table for prototype */

static const HashTableValue JSHTMLMediaElementPrototypeTableValues[15] =
{
    { "NETWORK_EMPTY", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_EMPTY), (intptr_t)0 },
    { "NETWORK_IDLE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_IDLE), (intptr_t)0 },
    { "NETWORK_LOADING", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_LOADING), (intptr_t)0 },
    { "NETWORK_LOADED", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_LOADED), (intptr_t)0 },
    { "NETWORK_NO_SOURCE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementNETWORK_NO_SOURCE), (intptr_t)0 },
    { "HAVE_NOTHING", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_NOTHING), (intptr_t)0 },
    { "HAVE_METADATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_METADATA), (intptr_t)0 },
    { "HAVE_CURRENT_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_CURRENT_DATA), (intptr_t)0 },
    { "HAVE_FUTURE_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_FUTURE_DATA), (intptr_t)0 },
    { "HAVE_ENOUGH_DATA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLMediaElementHAVE_ENOUGH_DATA), (intptr_t)0 },
    { "load", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsHTMLMediaElementPrototypeFunctionLoad), (intptr_t)0 },
    { "canPlayType", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsHTMLMediaElementPrototypeFunctionCanPlayType), (intptr_t)1 },
    { "play", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsHTMLMediaElementPrototypeFunctionPlay), (intptr_t)0 },
    { "pause", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsHTMLMediaElementPrototypeFunctionPause), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLMediaElementPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 63, JSHTMLMediaElementPrototypeTableValues, 0 };
#else
    { 33, 31, JSHTMLMediaElementPrototypeTableValues, 0 };
#endif

const ClassInfo JSHTMLMediaElementPrototype::s_info = { "HTMLMediaElementPrototype", 0, &JSHTMLMediaElementPrototypeTable, 0 };

JSObject* JSHTMLMediaElementPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSHTMLMediaElement>(exec, globalObject);
}

bool JSHTMLMediaElementPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticPropertySlot<JSHTMLMediaElementPrototype, JSObject>(exec, &JSHTMLMediaElementPrototypeTable, this, propertyName, slot);
}

bool JSHTMLMediaElementPrototype::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticPropertyDescriptor<JSHTMLMediaElementPrototype, JSObject>(exec, &JSHTMLMediaElementPrototypeTable, this, propertyName, descriptor);
}

const ClassInfo JSHTMLMediaElement::s_info = { "HTMLMediaElement", &JSHTMLElement::s_info, &JSHTMLMediaElementTable, 0 };

JSHTMLMediaElement::JSHTMLMediaElement(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<HTMLMediaElement> impl)
    : JSHTMLElement(structure, globalObject, impl)
{
}

JSObject* JSHTMLMediaElement::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSHTMLMediaElementPrototype(JSHTMLMediaElementPrototype::createStructure(JSHTMLElementPrototype::self(exec, globalObject)));
}

bool JSHTMLMediaElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLMediaElement, Base>(exec, &JSHTMLMediaElementTable, this, propertyName, slot);
}

bool JSHTMLMediaElement::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSHTMLMediaElement, Base>(exec, &JSHTMLMediaElementTable, this, propertyName, descriptor);
}

JSValue jsHTMLMediaElementError(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->error()));
    return result;
}

JSValue jsHTMLMediaElementSrc(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->src());
    return result;
}

JSValue jsHTMLMediaElementCurrentSrc(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->currentSrc());
    return result;
}

JSValue jsHTMLMediaElementNetworkState(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->networkState());
    return result;
}

JSValue jsHTMLMediaElementPreload(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->preload());
    return result;
}

JSValue jsHTMLMediaElementBuffered(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->buffered()));
    return result;
}

JSValue jsHTMLMediaElementReadyState(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->readyState());
    return result;
}

JSValue jsHTMLMediaElementSeeking(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->seeking());
    return result;
}

JSValue jsHTMLMediaElementCurrentTime(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->currentTime());
    return result;
}

JSValue jsHTMLMediaElementStartTime(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->startTime());
    return result;
}

JSValue jsHTMLMediaElementDuration(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->duration());
    return result;
}

JSValue jsHTMLMediaElementPaused(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->paused());
    return result;
}

JSValue jsHTMLMediaElementDefaultPlaybackRate(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->defaultPlaybackRate());
    return result;
}

JSValue jsHTMLMediaElementPlaybackRate(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->playbackRate());
    return result;
}

JSValue jsHTMLMediaElementPlayed(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->played()));
    return result;
}

JSValue jsHTMLMediaElementSeekable(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->seekable()));
    return result;
}

JSValue jsHTMLMediaElementEnded(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->ended());
    return result;
}

JSValue jsHTMLMediaElementAutoplay(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->autoplay());
    return result;
}

JSValue jsHTMLMediaElementLoop(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->loop());
    return result;
}

JSValue jsHTMLMediaElementControls(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->controls());
    return result;
}

JSValue jsHTMLMediaElementVolume(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsNumber(exec, imp->volume());
    return result;
}

JSValue jsHTMLMediaElementMuted(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->muted());
    return result;
}

JSValue jsHTMLMediaElementWebkitPreservesPitch(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->webkitPreservesPitch());
    return result;
}

JSValue jsHTMLMediaElementWebkitHasClosedCaptions(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->webkitHasClosedCaptions());
    return result;
}

JSValue jsHTMLMediaElementWebkitClosedCaptionsVisible(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* castedThis = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->webkitClosedCaptionsVisible());
    return result;
}

JSValue jsHTMLMediaElementConstructor(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLMediaElement* domObject = static_cast<JSHTMLMediaElement*>(asObject(slotBase));
    return JSHTMLMediaElement::getConstructor(exec, domObject->globalObject());
}
void JSHTMLMediaElement::put(ExecState* exec, const Identifier& propertyName, JSValue value, PutPropertySlot& slot)
{
    lookupPut<JSHTMLMediaElement, Base>(exec, propertyName, value, &JSHTMLMediaElementTable, this, slot);
}

void setJSHTMLMediaElementSrc(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setSrc(value.toString(exec));
}

void setJSHTMLMediaElementPreload(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setPreload(value.toString(exec));
}

void setJSHTMLMediaElementCurrentTime(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    ExceptionCode ec = 0;
    imp->setCurrentTime(value.toFloat(exec), ec);
    setDOMException(exec, ec);
}

void setJSHTMLMediaElementDefaultPlaybackRate(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setDefaultPlaybackRate(value.toFloat(exec));
}

void setJSHTMLMediaElementPlaybackRate(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setPlaybackRate(value.toFloat(exec));
}

void setJSHTMLMediaElementAutoplay(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setAutoplay(value.toBoolean(exec));
}

void setJSHTMLMediaElementLoop(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setLoop(value.toBoolean(exec));
}

void setJSHTMLMediaElementControls(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setControls(value.toBoolean(exec));
}

void setJSHTMLMediaElementVolume(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    ExceptionCode ec = 0;
    imp->setVolume(value.toFloat(exec), ec);
    setDOMException(exec, ec);
}

void setJSHTMLMediaElementMuted(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setMuted(value.toBoolean(exec));
}

void setJSHTMLMediaElementWebkitPreservesPitch(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setWebkitPreservesPitch(value.toBoolean(exec));
}

void setJSHTMLMediaElementWebkitClosedCaptionsVisible(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(thisObject);
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    imp->setWebkitClosedCaptionsVisible(value.toBoolean(exec));
}

JSValue JSHTMLMediaElement::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSHTMLMediaElementConstructor>(exec, static_cast<JSDOMGlobalObject*>(globalObject));
}

JSValue JSC_HOST_CALL jsHTMLMediaElementPrototypeFunctionLoad(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSHTMLMediaElement::s_info))
        return throwError(exec, TypeError);
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(asObject(thisValue));
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    ExceptionCode ec = 0;

    imp->load(processingUserGesture(exec), ec);
    setDOMException(exec, ec);
    return jsUndefined();
}

JSValue JSC_HOST_CALL jsHTMLMediaElementPrototypeFunctionCanPlayType(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSHTMLMediaElement::s_info))
        return throwError(exec, TypeError);
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(asObject(thisValue));
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());
    const UString& type = args.at(0).toString(exec);


    JSC::JSValue result = jsString(exec, imp->canPlayType(type));
    return result;
}

JSValue JSC_HOST_CALL jsHTMLMediaElementPrototypeFunctionPlay(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSHTMLMediaElement::s_info))
        return throwError(exec, TypeError);
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(asObject(thisValue));
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());

    imp->play(processingUserGesture(exec));
    return jsUndefined();
}

JSValue JSC_HOST_CALL jsHTMLMediaElementPrototypeFunctionPause(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSHTMLMediaElement::s_info))
        return throwError(exec, TypeError);
    JSHTMLMediaElement* castedThisObj = static_cast<JSHTMLMediaElement*>(asObject(thisValue));
    HTMLMediaElement* imp = static_cast<HTMLMediaElement*>(castedThisObj->impl());

    imp->pause(processingUserGesture(exec));
    return jsUndefined();
}

// Constant getters

JSValue jsHTMLMediaElementNETWORK_EMPTY(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(0));
}

JSValue jsHTMLMediaElementNETWORK_IDLE(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(1));
}

JSValue jsHTMLMediaElementNETWORK_LOADING(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(2));
}

JSValue jsHTMLMediaElementNETWORK_LOADED(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(3));
}

JSValue jsHTMLMediaElementNETWORK_NO_SOURCE(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(4));
}

JSValue jsHTMLMediaElementHAVE_NOTHING(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(0));
}

JSValue jsHTMLMediaElementHAVE_METADATA(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(1));
}

JSValue jsHTMLMediaElementHAVE_CURRENT_DATA(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(2));
}

JSValue jsHTMLMediaElementHAVE_FUTURE_DATA(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(3));
}

JSValue jsHTMLMediaElementHAVE_ENOUGH_DATA(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(4));
}


}

#endif // ENABLE(VIDEO)
