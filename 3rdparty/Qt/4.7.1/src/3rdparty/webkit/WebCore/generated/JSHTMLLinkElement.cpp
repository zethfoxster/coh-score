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
#include "JSHTMLLinkElement.h"

#include "HTMLLinkElement.h"
#include "JSStyleSheet.h"
#include "KURL.h"
#include "StyleSheet.h"
#include <runtime/JSString.h>
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSHTMLLinkElement);

/* Hash table */

static const HashTableValue JSHTMLLinkElementTableValues[12] =
{
    { "disabled", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementDisabled), (intptr_t)setJSHTMLLinkElementDisabled },
    { "charset", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementCharset), (intptr_t)setJSHTMLLinkElementCharset },
    { "href", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementHref), (intptr_t)setJSHTMLLinkElementHref },
    { "hreflang", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementHreflang), (intptr_t)setJSHTMLLinkElementHreflang },
    { "media", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementMedia), (intptr_t)setJSHTMLLinkElementMedia },
    { "rel", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementRel), (intptr_t)setJSHTMLLinkElementRel },
    { "rev", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementRev), (intptr_t)setJSHTMLLinkElementRev },
    { "target", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementTarget), (intptr_t)setJSHTMLLinkElementTarget },
    { "type", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementType), (intptr_t)setJSHTMLLinkElementType },
    { "sheet", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementSheet), (intptr_t)0 },
    { "constructor", DontEnum|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsHTMLLinkElementConstructor), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLLinkElementTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 63, JSHTMLLinkElementTableValues, 0 };
#else
    { 33, 31, JSHTMLLinkElementTableValues, 0 };
#endif

/* Hash table for constructor */

static const HashTableValue JSHTMLLinkElementConstructorTableValues[1] =
{
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLLinkElementConstructorTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSHTMLLinkElementConstructorTableValues, 0 };
#else
    { 1, 0, JSHTMLLinkElementConstructorTableValues, 0 };
#endif

class JSHTMLLinkElementConstructor : public DOMConstructorObject {
public:
    JSHTMLLinkElementConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
        : DOMConstructorObject(JSHTMLLinkElementConstructor::createStructure(globalObject->objectPrototype()), globalObject)
    {
        putDirect(exec->propertyNames().prototype, JSHTMLLinkElementPrototype::self(exec, globalObject), None);
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

const ClassInfo JSHTMLLinkElementConstructor::s_info = { "HTMLLinkElementConstructor", 0, &JSHTMLLinkElementConstructorTable, 0 };

bool JSHTMLLinkElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLLinkElementConstructor, DOMObject>(exec, &JSHTMLLinkElementConstructorTable, this, propertyName, slot);
}

bool JSHTMLLinkElementConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSHTMLLinkElementConstructor, DOMObject>(exec, &JSHTMLLinkElementConstructorTable, this, propertyName, descriptor);
}

/* Hash table for prototype */

static const HashTableValue JSHTMLLinkElementPrototypeTableValues[1] =
{
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSHTMLLinkElementPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSHTMLLinkElementPrototypeTableValues, 0 };
#else
    { 1, 0, JSHTMLLinkElementPrototypeTableValues, 0 };
#endif

const ClassInfo JSHTMLLinkElementPrototype::s_info = { "HTMLLinkElementPrototype", 0, &JSHTMLLinkElementPrototypeTable, 0 };

JSObject* JSHTMLLinkElementPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSHTMLLinkElement>(exec, globalObject);
}

const ClassInfo JSHTMLLinkElement::s_info = { "HTMLLinkElement", &JSHTMLElement::s_info, &JSHTMLLinkElementTable, 0 };

JSHTMLLinkElement::JSHTMLLinkElement(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<HTMLLinkElement> impl)
    : JSHTMLElement(structure, globalObject, impl)
{
}

JSObject* JSHTMLLinkElement::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSHTMLLinkElementPrototype(JSHTMLLinkElementPrototype::createStructure(JSHTMLElementPrototype::self(exec, globalObject)));
}

bool JSHTMLLinkElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLLinkElement, Base>(exec, &JSHTMLLinkElementTable, this, propertyName, slot);
}

bool JSHTMLLinkElement::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSHTMLLinkElement, Base>(exec, &JSHTMLLinkElementTable, this, propertyName, descriptor);
}

JSValue jsHTMLLinkElementDisabled(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsBoolean(imp->disabled());
    return result;
}

JSValue jsHTMLLinkElementCharset(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->charset());
    return result;
}

JSValue jsHTMLLinkElementHref(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->href());
    return result;
}

JSValue jsHTMLLinkElementHreflang(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->hreflang());
    return result;
}

JSValue jsHTMLLinkElementMedia(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->media());
    return result;
}

JSValue jsHTMLLinkElementRel(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->rel());
    return result;
}

JSValue jsHTMLLinkElementRev(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->rev());
    return result;
}

JSValue jsHTMLLinkElementTarget(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->target());
    return result;
}

JSValue jsHTMLLinkElementType(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->type());
    return result;
}

JSValue jsHTMLLinkElementSheet(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* castedThis = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->sheet()));
    return result;
}

JSValue jsHTMLLinkElementConstructor(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSHTMLLinkElement* domObject = static_cast<JSHTMLLinkElement*>(asObject(slotBase));
    return JSHTMLLinkElement::getConstructor(exec, domObject->globalObject());
}
void JSHTMLLinkElement::put(ExecState* exec, const Identifier& propertyName, JSValue value, PutPropertySlot& slot)
{
    lookupPut<JSHTMLLinkElement, Base>(exec, propertyName, value, &JSHTMLLinkElementTable, this, slot);
}

void setJSHTMLLinkElementDisabled(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setDisabled(value.toBoolean(exec));
}

void setJSHTMLLinkElementCharset(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setCharset(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementHref(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setHref(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementHreflang(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setHreflang(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementMedia(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setMedia(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementRel(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setRel(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementRev(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setRev(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementTarget(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setTarget(valueToStringWithNullCheck(exec, value));
}

void setJSHTMLLinkElementType(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSHTMLLinkElement* castedThisObj = static_cast<JSHTMLLinkElement*>(thisObject);
    HTMLLinkElement* imp = static_cast<HTMLLinkElement*>(castedThisObj->impl());
    imp->setType(valueToStringWithNullCheck(exec, value));
}

JSValue JSHTMLLinkElement::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSHTMLLinkElementConstructor>(exec, static_cast<JSDOMGlobalObject*>(globalObject));
}


}
