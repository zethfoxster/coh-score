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

#if ENABLE(SVG) && ENABLE(FILTERS)

#include "JSSVGComponentTransferFunctionElement.h"

#include "JSSVGAnimatedEnumeration.h"
#include "JSSVGAnimatedNumber.h"
#include "JSSVGAnimatedNumberList.h"
#include "SVGComponentTransferFunctionElement.h"
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSSVGComponentTransferFunctionElement);

/* Hash table */

static const HashTableValue JSSVGComponentTransferFunctionElementTableValues[9] =
{
    { "type", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementType), (intptr_t)0 },
    { "tableValues", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementTableValues), (intptr_t)0 },
    { "slope", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSlope), (intptr_t)0 },
    { "intercept", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementIntercept), (intptr_t)0 },
    { "amplitude", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementAmplitude), (intptr_t)0 },
    { "exponent", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementExponent), (intptr_t)0 },
    { "offset", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementOffset), (intptr_t)0 },
    { "constructor", DontEnum|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementConstructor), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGComponentTransferFunctionElementTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 31, JSSVGComponentTransferFunctionElementTableValues, 0 };
#else
    { 19, 15, JSSVGComponentTransferFunctionElementTableValues, 0 };
#endif

/* Hash table for constructor */

static const HashTableValue JSSVGComponentTransferFunctionElementConstructorTableValues[7] =
{
    { "SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_IDENTITY), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_TABLE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_TABLE), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_DISCRETE), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_LINEAR", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_LINEAR), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_GAMMA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_GAMMA), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGComponentTransferFunctionElementConstructorTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 63, JSSVGComponentTransferFunctionElementConstructorTableValues, 0 };
#else
    { 17, 15, JSSVGComponentTransferFunctionElementConstructorTableValues, 0 };
#endif

class JSSVGComponentTransferFunctionElementConstructor : public DOMConstructorObject {
public:
    JSSVGComponentTransferFunctionElementConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
        : DOMConstructorObject(JSSVGComponentTransferFunctionElementConstructor::createStructure(globalObject->objectPrototype()), globalObject)
    {
        putDirect(exec->propertyNames().prototype, JSSVGComponentTransferFunctionElementPrototype::self(exec, globalObject), None);
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

const ClassInfo JSSVGComponentTransferFunctionElementConstructor::s_info = { "SVGComponentTransferFunctionElementConstructor", 0, &JSSVGComponentTransferFunctionElementConstructorTable, 0 };

bool JSSVGComponentTransferFunctionElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGComponentTransferFunctionElementConstructor, DOMObject>(exec, &JSSVGComponentTransferFunctionElementConstructorTable, this, propertyName, slot);
}

bool JSSVGComponentTransferFunctionElementConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGComponentTransferFunctionElementConstructor, DOMObject>(exec, &JSSVGComponentTransferFunctionElementConstructorTable, this, propertyName, descriptor);
}

/* Hash table for prototype */

static const HashTableValue JSSVGComponentTransferFunctionElementPrototypeTableValues[7] =
{
    { "SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_IDENTITY), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_TABLE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_TABLE), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_DISCRETE), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_LINEAR", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_LINEAR), (intptr_t)0 },
    { "SVG_FECOMPONENTTRANSFER_TYPE_GAMMA", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_GAMMA), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGComponentTransferFunctionElementPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 63, JSSVGComponentTransferFunctionElementPrototypeTableValues, 0 };
#else
    { 17, 15, JSSVGComponentTransferFunctionElementPrototypeTableValues, 0 };
#endif

const ClassInfo JSSVGComponentTransferFunctionElementPrototype::s_info = { "SVGComponentTransferFunctionElementPrototype", 0, &JSSVGComponentTransferFunctionElementPrototypeTable, 0 };

JSObject* JSSVGComponentTransferFunctionElementPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSSVGComponentTransferFunctionElement>(exec, globalObject);
}

bool JSSVGComponentTransferFunctionElementPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGComponentTransferFunctionElementPrototype, JSObject>(exec, &JSSVGComponentTransferFunctionElementPrototypeTable, this, propertyName, slot);
}

bool JSSVGComponentTransferFunctionElementPrototype::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGComponentTransferFunctionElementPrototype, JSObject>(exec, &JSSVGComponentTransferFunctionElementPrototypeTable, this, propertyName, descriptor);
}

const ClassInfo JSSVGComponentTransferFunctionElement::s_info = { "SVGComponentTransferFunctionElement", &JSSVGElement::s_info, &JSSVGComponentTransferFunctionElementTable, 0 };

JSSVGComponentTransferFunctionElement::JSSVGComponentTransferFunctionElement(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<SVGComponentTransferFunctionElement> impl)
    : JSSVGElement(structure, globalObject, impl)
{
}

JSObject* JSSVGComponentTransferFunctionElement::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSSVGComponentTransferFunctionElementPrototype(JSSVGComponentTransferFunctionElementPrototype::createStructure(JSSVGElementPrototype::self(exec, globalObject)));
}

bool JSSVGComponentTransferFunctionElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGComponentTransferFunctionElement, Base>(exec, &JSSVGComponentTransferFunctionElementTable, this, propertyName, slot);
}

bool JSSVGComponentTransferFunctionElement::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGComponentTransferFunctionElement, Base>(exec, &JSSVGComponentTransferFunctionElementTable, this, propertyName, descriptor);
}

JSValue jsSVGComponentTransferFunctionElementType(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedEnumeration> obj = imp->typeAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementTableValues(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumberList> obj = imp->tableValuesAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementSlope(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumber> obj = imp->slopeAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementIntercept(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumber> obj = imp->interceptAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementAmplitude(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumber> obj = imp->amplitudeAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementExponent(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumber> obj = imp->exponentAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementOffset(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* castedThis = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGComponentTransferFunctionElement* imp = static_cast<SVGComponentTransferFunctionElement*>(castedThis->impl());
    RefPtr<SVGAnimatedNumber> obj = imp->offsetAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGComponentTransferFunctionElementConstructor(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGComponentTransferFunctionElement* domObject = static_cast<JSSVGComponentTransferFunctionElement*>(asObject(slotBase));
    return JSSVGComponentTransferFunctionElement::getConstructor(exec, domObject->globalObject());
}
JSValue JSSVGComponentTransferFunctionElement::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSSVGComponentTransferFunctionElementConstructor>(exec, static_cast<JSDOMGlobalObject*>(globalObject));
}

// Constant getters

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(0));
}

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_IDENTITY(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(1));
}

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_TABLE(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(2));
}

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_DISCRETE(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(3));
}

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_LINEAR(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(4));
}

JSValue jsSVGComponentTransferFunctionElementSVG_FECOMPONENTTRANSFER_TYPE_GAMMA(ExecState* exec, JSValue, const Identifier&)
{
    return jsNumber(exec, static_cast<int>(5));
}


}

#endif // ENABLE(SVG) && ENABLE(FILTERS)
