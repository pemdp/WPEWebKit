/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Samuel Weinig <sam@webkit.org>
 *  Copyright (C) 2013 Michael Pruett <michael@68k.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "JSDOMExceptionHandling.h"

#include "CachedScript.h"
#include "DOMException.h"
#include "DOMWindow.h"
#include "JSDOMException.h"
#include "JSDOMPromiseDeferred.h"
#include "JSDOMWindow.h"
#include "JSDynamicDowncast.h"
#include "ScriptExecutionContext.h"
#include <inspector/ScriptCallStack.h>
#include <inspector/ScriptCallStackFactory.h>
#include <runtime/ErrorHandlingScope.h>
#include <runtime/Exception.h>
#include <runtime/ExceptionHelpers.h>
#include <wtf/text/StringBuilder.h>

using namespace JSC;

namespace WebCore {

void reportException(ExecState* exec, JSValue exceptionValue, CachedScript* cachedScript)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    auto* exception = jsDynamicDowncast<JSC::Exception*>(vm, exceptionValue);
    if (!exception) {
        exception = vm.lastException();
        if (!exception)
            exception = JSC::Exception::create(exec->vm(), exceptionValue, JSC::Exception::DoNotCaptureStack);
    }

    reportException(exec, exception, cachedScript);
}

String retrieveErrorMessage(ExecState& state, VM& vm, JSValue exception, CatchScope& catchScope)
{
    // FIXME: <http://webkit.org/b/115087> Web Inspector: WebCore::reportException should not evaluate JavaScript handling exceptions
    // If this is a custom exception object, call toString on it to try and get a nice string representation for the exception.
    String errorMessage;
    if (auto* error = jsDynamicDowncast<ErrorInstance*>(vm, exception))
        errorMessage = error->sanitizedToString(&state);
    else
        errorMessage = exception.toWTFString(&state);

    // We need to clear any new exception that may be thrown in the toString() call above.
    // reportException() is not supposed to be making new exceptions.
    catchScope.clearException();
    vm.clearLastException();
    return errorMessage;
}

void reportException(ExecState* exec, JSC::Exception* exception, CachedScript* cachedScript, ExceptionDetails* exceptionDetails)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    if (isTerminatedExecutionException(vm, exception))
        return;

    ErrorHandlingScope errorScope(exec->vm());

    auto callStack = Inspector::createScriptCallStackFromException(exec, exception, Inspector::ScriptCallStack::maxCallStackSizeToCapture);
    scope.clearException();
    vm.clearLastException();

    auto* globalObject = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject());
    if (auto* window = jsDynamicDowncast<JSDOMWindow*>(vm, globalObject)) {
        if (!window->wrapped().isCurrentlyDisplayedInFrame())
            return;
    }

    int lineNumber = 0;
    int columnNumber = 0;
    String exceptionSourceURL;
    if (auto* callFrame = callStack->firstNonNativeCallFrame()) {
        lineNumber = callFrame->lineNumber();
        columnNumber = callFrame->columnNumber();
        exceptionSourceURL = callFrame->sourceURL();
    }

    auto errorMessage = retrieveErrorMessage(*exec, vm, exception->value(), scope);
    globalObject->scriptExecutionContext()->reportException(errorMessage, lineNumber, columnNumber, exceptionSourceURL, exception, callStack->size() ? callStack.ptr() : nullptr, cachedScript);

    if (exceptionDetails) {
        exceptionDetails->message = errorMessage;
        exceptionDetails->lineNumber = lineNumber;
        exceptionDetails->columnNumber = columnNumber;
        exceptionDetails->sourceURL = exceptionSourceURL;
    }
}

void reportCurrentException(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    auto* exception = scope.exception();
    scope.clearException();
    reportException(exec, exception);
}

JSValue createDOMException(ExecState* exec, ExceptionCode ec, const String& message)
{
    if (ec == ExistingExceptionError)
        return jsUndefined();

    // FIXME: Handle other WebIDL exception types.
    if (ec == TypeError) {
        if (message.isEmpty())
            return createTypeError(exec);
        return createTypeError(exec, message);
    }

    if (ec == RangeError) {
        if (message.isEmpty())
            return createRangeError(exec, ASCIILiteral("Bad value"));
        return createRangeError(exec, message);
    }

    if (ec == StackOverflowError)
        return createStackOverflowError(exec);

    // FIXME: All callers to createDOMException need to pass in the correct global object.
    // For now, we're going to assume the lexicalGlobalObject. Which is wrong in cases like this:
    // frames[0].document.createElement(null, null); // throws an exception which should have the subframe's prototypes.
    JSDOMGlobalObject* globalObject = deprecatedGlobalObjectForPrototype(exec);
    JSValue errorObject = toJS(exec, globalObject, DOMException::create(ec, message));
    
    ASSERT(errorObject);
    addErrorInfo(exec, asObject(errorObject), true);
    return errorObject;
}

JSValue createDOMException(ExecState& state, Exception&& exception)
{
    return createDOMException(&state, exception.code(), exception.releaseMessage());
}

void propagateExceptionSlowPath(JSC::ExecState& state, JSC::ThrowScope& throwScope, Exception&& exception)
{
    throwScope.assertNoException();
    throwException(&state, throwScope, createDOMException(state, WTFMove(exception)));
}
    
static EncodedJSValue throwTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const String& errorMessage)
{
    return throwVMTypeError(&state, scope, errorMessage);
}

static void appendArgumentMustBe(StringBuilder& builder, unsigned argumentIndex, const char* argumentName, const char* interfaceName, const char* functionName)
{
    builder.appendLiteral("Argument ");
    builder.appendNumber(argumentIndex + 1);
    builder.appendLiteral(" ('");
    builder.append(argumentName);
    builder.appendLiteral("') to ");
    if (!functionName) {
        builder.appendLiteral("the ");
        builder.append(interfaceName);
        builder.appendLiteral(" constructor");
    } else {
        builder.append(interfaceName);
        builder.append('.');
        builder.append(functionName);
    }
    builder.appendLiteral(" must be ");
}

JSC::EncodedJSValue reportDeprecatedGetterError(JSC::ExecState& state, const char* interfaceName, const char* attributeName)
{
    auto& context = *jsCast<JSDOMGlobalObject*>(state.lexicalGlobalObject())->scriptExecutionContext();
    context.addConsoleMessage(MessageSource::JS, MessageLevel::Error, makeString("Deprecated attempt to access property '", attributeName, "' on a non-", interfaceName, " object."));
    return JSValue::encode(jsUndefined());
}
    
void reportDeprecatedSetterError(JSC::ExecState& state, const char* interfaceName, const char* attributeName)
{
    auto& context = *jsCast<JSDOMGlobalObject*>(state.lexicalGlobalObject())->scriptExecutionContext();
    context.addConsoleMessage(MessageSource::JS, MessageLevel::Error, makeString("Deprecated attempt to set property '", attributeName, "' on a non-", interfaceName, " object."));
}

void throwNotSupportedError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, NotSupportedError));
}

void throwNotSupportedError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* message)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, NotSupportedError, message));
}

void throwInvalidStateError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* message)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, InvalidStateError, message));
}

void throwSecurityError(JSC::ExecState& state, JSC::ThrowScope& scope, const String& message)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, SecurityError, message));
}

JSC::EncodedJSValue throwArgumentMustBeEnumError(JSC::ExecState& state, JSC::ThrowScope& scope, unsigned argumentIndex, const char* argumentName, const char* functionInterfaceName, const char* functionName, const char* expectedValues)
{
    StringBuilder builder;
    appendArgumentMustBe(builder, argumentIndex, argumentName, functionInterfaceName, functionName);
    builder.appendLiteral("one of: ");
    builder.append(expectedValues);
    return throwVMTypeError(&state, scope, builder.toString());
}

JSC::EncodedJSValue throwArgumentMustBeFunctionError(JSC::ExecState& state, JSC::ThrowScope& scope, unsigned argumentIndex, const char* argumentName, const char* interfaceName, const char* functionName)
{
    StringBuilder builder;
    appendArgumentMustBe(builder, argumentIndex, argumentName, interfaceName, functionName);
    builder.appendLiteral("a function");
    return throwVMTypeError(&state, scope, builder.toString());
}

JSC::EncodedJSValue throwArgumentTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, unsigned argumentIndex, const char* argumentName, const char* functionInterfaceName, const char* functionName, const char* expectedType)
{
    StringBuilder builder;
    appendArgumentMustBe(builder, argumentIndex, argumentName, functionInterfaceName, functionName);
    builder.appendLiteral("an instance of ");
    builder.append(expectedType);
    return throwVMTypeError(&state, scope, builder.toString());
}

void throwArrayElementTypeError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    throwTypeError(state, scope, ASCIILiteral("Invalid Array element type"));
}

void throwAttributeTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* interfaceName, const char* attributeName, const char* expectedType)
{
    throwTypeError(state, scope, makeString("The ", interfaceName, '.', attributeName, " attribute must be an instance of ", expectedType));
}

JSC::EncodedJSValue throwRequiredMemberTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* memberName, const char* dictionaryName, const char* expectedType)
{
    StringBuilder builder;
    builder.appendLiteral("Member ");
    builder.append(dictionaryName);
    builder.append('.');
    builder.append(memberName);
    builder.appendLiteral(" is required and must be an instance of ");
    builder.append(expectedType);
    return throwVMTypeError(&state, scope, builder.toString());
}

JSC::EncodedJSValue throwConstructorScriptExecutionContextUnavailableError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* interfaceName)
{
    return throwVMError(&state, scope, createReferenceError(&state, makeString(interfaceName, " constructor associated execution context is unavailable")));
}

void throwSequenceTypeError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    throwTypeError(state, scope, ASCIILiteral("Value is not a sequence"));
}

void throwNonFiniteTypeError(ExecState& state, JSC::ThrowScope& scope)
{
    throwTypeError(&state, scope, ASCIILiteral("The provided value is non-finite"));
}

String makeGetterTypeErrorMessage(const char* interfaceName, const char* attributeName)
{
    return makeString("The ", interfaceName, '.', attributeName, " getter can only be used on instances of ", interfaceName);
}

JSC::EncodedJSValue throwGetterTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* interfaceName, const char* attributeName)
{
    return throwVMTypeError(&state, scope, makeGetterTypeErrorMessage(interfaceName, attributeName));
}

JSC::EncodedJSValue rejectPromiseWithGetterTypeError(JSC::ExecState& state, const char* interfaceName, const char* attributeName)
{
    return createRejectedPromiseWithTypeError(state, makeGetterTypeErrorMessage(interfaceName, attributeName));
}

bool throwSetterTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* interfaceName, const char* attributeName)
{
    throwTypeError(state, scope, makeString("The ", interfaceName, '.', attributeName, " setter can only be used on instances of ", interfaceName));
    return false;
}

String makeThisTypeErrorMessage(const char* interfaceName, const char* functionName)
{
    return makeString("Can only call ", interfaceName, '.', functionName, " on instances of ", interfaceName);
}

EncodedJSValue throwThisTypeError(JSC::ExecState& state, JSC::ThrowScope& scope, const char* interfaceName, const char* functionName)
{
    return throwTypeError(state, scope, makeThisTypeErrorMessage(interfaceName, functionName));
}

JSC::EncodedJSValue rejectPromiseWithThisTypeError(DeferredPromise& promise, const char* interfaceName, const char* methodName)
{
    promise.reject(TypeError, makeThisTypeErrorMessage(interfaceName, methodName));
    return JSValue::encode(jsUndefined());
}

JSC::EncodedJSValue rejectPromiseWithThisTypeError(JSC::ExecState& state, const char* interfaceName, const char* methodName)
{
    return createRejectedPromiseWithTypeError(state, makeThisTypeErrorMessage(interfaceName, methodName));
}

void throwDOMSyntaxError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, SyntaxError));
}

void throwDataCloneError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, DataCloneError));
}

void throwIndexSizeError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, IndexSizeError));
}

void throwTypeMismatchError(JSC::ExecState& state, JSC::ThrowScope& scope)
{
    scope.assertNoException();
    throwException(&state, scope, createDOMException(&state, TypeMismatchError));
}

} // namespace WebCore
