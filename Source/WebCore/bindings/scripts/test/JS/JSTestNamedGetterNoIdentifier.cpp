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
#include "JSTestNamedGetterNoIdentifier.h"

#include "JSDOMAbstractOperations.h"
#include "JSDOMBinding.h"
#include "JSDOMConstructorNotConstructable.h"
#include "JSDOMConvertStrings.h"
#include "JSDOMExceptionHandling.h"
#include "JSDOMWrapperCache.h"
#include <runtime/FunctionPrototype.h>
#include <runtime/JSCInlines.h>
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

// Attributes

JSC::EncodedJSValue jsTestNamedGetterNoIdentifierConstructor(JSC::ExecState*, JSC::EncodedJSValue, JSC::PropertyName);
bool setJSTestNamedGetterNoIdentifierConstructor(JSC::ExecState*, JSC::EncodedJSValue, JSC::EncodedJSValue);

class JSTestNamedGetterNoIdentifierPrototype : public JSC::JSNonFinalObject {
public:
    using Base = JSC::JSNonFinalObject;
    static JSTestNamedGetterNoIdentifierPrototype* create(JSC::VM& vm, JSDOMGlobalObject* globalObject, JSC::Structure* structure)
    {
        JSTestNamedGetterNoIdentifierPrototype* ptr = new (NotNull, JSC::allocateCell<JSTestNamedGetterNoIdentifierPrototype>(vm.heap)) JSTestNamedGetterNoIdentifierPrototype(vm, globalObject, structure);
        ptr->finishCreation(vm);
        return ptr;
    }

    DECLARE_INFO;
    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

private:
    JSTestNamedGetterNoIdentifierPrototype(JSC::VM& vm, JSC::JSGlobalObject*, JSC::Structure* structure)
        : JSC::JSNonFinalObject(vm, structure)
    {
    }

    void finishCreation(JSC::VM&);
};

using JSTestNamedGetterNoIdentifierConstructor = JSDOMConstructorNotConstructable<JSTestNamedGetterNoIdentifier>;

template<> JSValue JSTestNamedGetterNoIdentifierConstructor::prototypeForStructure(JSC::VM& vm, const JSDOMGlobalObject& globalObject)
{
    UNUSED_PARAM(vm);
    return globalObject.functionPrototype();
}

template<> void JSTestNamedGetterNoIdentifierConstructor::initializeProperties(VM& vm, JSDOMGlobalObject& globalObject)
{
    putDirect(vm, vm.propertyNames->prototype, JSTestNamedGetterNoIdentifier::prototype(vm, globalObject), JSC::PropertyAttribute::DontDelete | JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->name, jsNontrivialString(&vm, String(ASCIILiteral("TestNamedGetterNoIdentifier"))), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->length, jsNumber(0), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
}

template<> const ClassInfo JSTestNamedGetterNoIdentifierConstructor::s_info = { "TestNamedGetterNoIdentifier", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedGetterNoIdentifierConstructor) };

/* Hash table for prototype */

static const HashTableValue JSTestNamedGetterNoIdentifierPrototypeTableValues[] =
{
    { "constructor", static_cast<unsigned>(JSC::PropertyAttribute::DontEnum), NoIntrinsic, { (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsTestNamedGetterNoIdentifierConstructor), (intptr_t) static_cast<PutPropertySlot::PutValueFunc>(setJSTestNamedGetterNoIdentifierConstructor) } },
};

const ClassInfo JSTestNamedGetterNoIdentifierPrototype::s_info = { "TestNamedGetterNoIdentifierPrototype", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedGetterNoIdentifierPrototype) };

void JSTestNamedGetterNoIdentifierPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    reifyStaticProperties(vm, JSTestNamedGetterNoIdentifier::info(), JSTestNamedGetterNoIdentifierPrototypeTableValues, *this);
}

const ClassInfo JSTestNamedGetterNoIdentifier::s_info = { "TestNamedGetterNoIdentifier", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedGetterNoIdentifier) };

JSTestNamedGetterNoIdentifier::JSTestNamedGetterNoIdentifier(Structure* structure, JSDOMGlobalObject& globalObject, Ref<TestNamedGetterNoIdentifier>&& impl)
    : JSDOMWrapper<TestNamedGetterNoIdentifier>(structure, globalObject, WTFMove(impl))
{
}

void JSTestNamedGetterNoIdentifier::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

}

JSObject* JSTestNamedGetterNoIdentifier::createPrototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return JSTestNamedGetterNoIdentifierPrototype::create(vm, &globalObject, JSTestNamedGetterNoIdentifierPrototype::createStructure(vm, &globalObject, globalObject.objectPrototype()));
}

JSObject* JSTestNamedGetterNoIdentifier::prototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return getDOMPrototype<JSTestNamedGetterNoIdentifier>(vm, globalObject);
}

JSValue JSTestNamedGetterNoIdentifier::getConstructor(VM& vm, const JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestNamedGetterNoIdentifierConstructor>(vm, *jsCast<const JSDOMGlobalObject*>(globalObject));
}

void JSTestNamedGetterNoIdentifier::destroy(JSC::JSCell* cell)
{
    JSTestNamedGetterNoIdentifier* thisObject = static_cast<JSTestNamedGetterNoIdentifier*>(cell);
    thisObject->JSTestNamedGetterNoIdentifier::~JSTestNamedGetterNoIdentifier();
}

bool JSTestNamedGetterNoIdentifier::getOwnPropertySlot(JSObject* object, ExecState* state, PropertyName propertyName, PropertySlot& slot)
{
    auto* thisObject = jsCast<JSTestNamedGetterNoIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    using GetterIDLType = IDLDOMString;
    auto getterFunctor = [] (auto& thisObject, auto propertyName) -> std::optional<typename GetterIDLType::ImplementationType> {
        auto result = thisObject.wrapped().namedItem(propertyNameToAtomicString(propertyName));
        if (!GetterIDLType::isNullValue(result))
            return typename GetterIDLType::ImplementationType { GetterIDLType::extractValueFromNullable(result) };
        return std::nullopt;
    };
    if (auto namedProperty = accessVisibleNamedProperty<OverrideBuiltins::No>(*state, *thisObject, propertyName, getterFunctor)) {
        auto value = toJS<IDLDOMString>(*state, WTFMove(namedProperty.value()));
        slot.setValue(thisObject, static_cast<unsigned>(JSC::PropertyAttribute::ReadOnly), value);
        return true;
    }
    return JSObject::getOwnPropertySlot(object, state, propertyName, slot);
}

bool JSTestNamedGetterNoIdentifier::getOwnPropertySlotByIndex(JSObject* object, ExecState* state, unsigned index, PropertySlot& slot)
{
    auto* thisObject = jsCast<JSTestNamedGetterNoIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    auto propertyName = Identifier::from(state, index);
    using GetterIDLType = IDLDOMString;
    auto getterFunctor = [] (auto& thisObject, auto propertyName) -> std::optional<typename GetterIDLType::ImplementationType> {
        auto result = thisObject.wrapped().namedItem(propertyNameToAtomicString(propertyName));
        if (!GetterIDLType::isNullValue(result))
            return typename GetterIDLType::ImplementationType { GetterIDLType::extractValueFromNullable(result) };
        return std::nullopt;
    };
    if (auto namedProperty = accessVisibleNamedProperty<OverrideBuiltins::No>(*state, *thisObject, propertyName, getterFunctor)) {
        auto value = toJS<IDLDOMString>(*state, WTFMove(namedProperty.value()));
        slot.setValue(thisObject, static_cast<unsigned>(JSC::PropertyAttribute::ReadOnly), value);
        return true;
    }
    return JSObject::getOwnPropertySlotByIndex(object, state, index, slot);
}

void JSTestNamedGetterNoIdentifier::getOwnPropertyNames(JSObject* object, ExecState* state, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    auto* thisObject = jsCast<JSTestNamedGetterNoIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(object, info());
    for (auto& propertyName : thisObject->wrapped().supportedPropertyNames())
        propertyNames.add(Identifier::fromString(state, propertyName));
    JSObject::getOwnPropertyNames(object, state, propertyNames, mode);
}

EncodedJSValue jsTestNamedGetterNoIdentifierConstructor(ExecState* state, EncodedJSValue thisValue, PropertyName)
{
    VM& vm = state->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicDowncast<JSTestNamedGetterNoIdentifierPrototype*>(vm, JSValue::decode(thisValue));
    if (UNLIKELY(!prototype))
        return throwVMTypeError(state, throwScope);
    return JSValue::encode(JSTestNamedGetterNoIdentifier::getConstructor(state->vm(), prototype->globalObject()));
}

bool setJSTestNamedGetterNoIdentifierConstructor(ExecState* state, EncodedJSValue thisValue, EncodedJSValue encodedValue)
{
    VM& vm = state->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicDowncast<JSTestNamedGetterNoIdentifierPrototype*>(vm, JSValue::decode(thisValue));
    if (UNLIKELY(!prototype)) {
        throwVMTypeError(state, throwScope);
        return false;
    }
    // Shadowing a built-in constructor
    return prototype->putDirect(vm, vm.propertyNames->constructor, JSValue::decode(encodedValue));
}

bool JSTestNamedGetterNoIdentifierOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown> handle, void*, SlotVisitor& visitor)
{
    UNUSED_PARAM(handle);
    UNUSED_PARAM(visitor);
    return false;
}

void JSTestNamedGetterNoIdentifierOwner::finalize(JSC::Handle<JSC::Unknown> handle, void* context)
{
    auto* jsTestNamedGetterNoIdentifier = static_cast<JSTestNamedGetterNoIdentifier*>(handle.slot()->asCell());
    auto& world = *static_cast<DOMWrapperWorld*>(context);
    uncacheWrapper(world, &jsTestNamedGetterNoIdentifier->wrapped(), jsTestNamedGetterNoIdentifier);
}

#if ENABLE(BINDING_INTEGRITY)
#if PLATFORM(WIN)
#pragma warning(disable: 4483)
extern "C" { extern void (*const __identifier("??_7TestNamedGetterNoIdentifier@WebCore@@6B@")[])(); }
#else
extern "C" { extern void* _ZTVN7WebCore27TestNamedGetterNoIdentifierE[]; }
#endif
#endif

JSC::JSValue toJSNewlyCreated(JSC::ExecState*, JSDOMGlobalObject* globalObject, Ref<TestNamedGetterNoIdentifier>&& impl)
{

#if ENABLE(BINDING_INTEGRITY)
    void* actualVTablePointer = *(reinterpret_cast<void**>(impl.ptr()));
#if PLATFORM(WIN)
    void* expectedVTablePointer = reinterpret_cast<void*>(__identifier("??_7TestNamedGetterNoIdentifier@WebCore@@6B@"));
#else
    void* expectedVTablePointer = &_ZTVN7WebCore27TestNamedGetterNoIdentifierE[2];
#endif

    // If this fails TestNamedGetterNoIdentifier does not have a vtable, so you need to add the
    // ImplementationLacksVTable attribute to the interface definition
    static_assert(std::is_polymorphic<TestNamedGetterNoIdentifier>::value, "TestNamedGetterNoIdentifier is not polymorphic");

    // If you hit this assertion you either have a use after free bug, or
    // TestNamedGetterNoIdentifier has subclasses. If TestNamedGetterNoIdentifier has subclasses that get passed
    // to toJS() we currently require TestNamedGetterNoIdentifier you to opt out of binding hardening
    // by adding the SkipVTableValidation attribute to the interface IDL definition
    RELEASE_ASSERT(actualVTablePointer == expectedVTablePointer);
#endif
    return createWrapper<TestNamedGetterNoIdentifier>(globalObject, WTFMove(impl));
}

JSC::JSValue toJS(JSC::ExecState* state, JSDOMGlobalObject* globalObject, TestNamedGetterNoIdentifier& impl)
{
    return wrap(state, globalObject, impl);
}

TestNamedGetterNoIdentifier* JSTestNamedGetterNoIdentifier::toWrapped(JSC::VM& vm, JSC::JSValue value)
{
    if (auto* wrapper = jsDynamicDowncast<JSTestNamedGetterNoIdentifier*>(vm, value))
        return &wrapper->wrapped();
    return nullptr;
}

}
