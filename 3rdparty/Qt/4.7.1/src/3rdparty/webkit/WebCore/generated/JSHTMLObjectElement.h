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

#ifndef JSHTMLObjectElement_h
#define JSHTMLObjectElement_h

#include "JSHTMLElement.h"
#include <runtime/CallData.h>

namespace WebCore {

class HTMLObjectElement;

class JSHTMLObjectElement : public JSHTMLElement {
    typedef JSHTMLElement Base;
public:
    JSHTMLObjectElement(NonNullPassRefPtr<JSC::Structure>, JSDOMGlobalObject*, PassRefPtr<HTMLObjectElement>);
    static JSC::JSObject* createPrototype(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertyDescriptor&);
    bool getOwnPropertySlotDelegate(JSC::ExecState*, const JSC::Identifier&, JSC::PropertySlot&);
    bool getOwnPropertyDescriptorDelegate(JSC::ExecState*, const JSC::Identifier&, JSC::PropertyDescriptor&);
    virtual void put(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::JSValue, JSC::PutPropertySlot&);
    bool putDelegate(JSC::ExecState*, const JSC::Identifier&, JSC::JSValue, JSC::PutPropertySlot&);
    virtual const JSC::ClassInfo* classInfo() const { return &s_info; }
    static const JSC::ClassInfo s_info;

    static PassRefPtr<JSC::Structure> createStructure(JSC::JSValue prototype)
    {
        return JSC::Structure::create(prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount);
    }

    virtual JSC::CallType getCallData(JSC::CallData&);

    static JSC::JSValue getConstructor(JSC::ExecState*, JSC::JSGlobalObject*);
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
};


class JSHTMLObjectElementPrototype : public JSC::JSObject {
    typedef JSC::JSObject Base;
public:
    static JSC::JSObject* self(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual const JSC::ClassInfo* classInfo() const { return &s_info; }
    static const JSC::ClassInfo s_info;
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier&, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier&, JSC::PropertyDescriptor&);
    static PassRefPtr<JSC::Structure> createStructure(JSC::JSValue prototype)
    {
        return JSC::Structure::create(prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount);
    }
    JSHTMLObjectElementPrototype(NonNullPassRefPtr<JSC::Structure> structure) : JSC::JSObject(structure) { }
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
};

// Functions

JSC::JSValue JSC_HOST_CALL jsHTMLObjectElementPrototypeFunctionGetSVGDocument(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
// Attributes

JSC::JSValue jsHTMLObjectElementForm(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsHTMLObjectElementCode(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementCode(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementAlign(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementAlign(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementArchive(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementArchive(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementBorder(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementBorder(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementCodeBase(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementCodeBase(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementCodeType(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementCodeType(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementData(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementData(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementDeclare(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementDeclare(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementHeight(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementHeight(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementHspace(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementHspace(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementName(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementName(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementStandby(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementStandby(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementType(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementType(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementUseMap(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementUseMap(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementVspace(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementVspace(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementWidth(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSHTMLObjectElementWidth(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsHTMLObjectElementWillValidate(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsHTMLObjectElementContentDocument(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsHTMLObjectElementConstructor(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);

} // namespace WebCore

#endif
