/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

WI.RecordingActionTreeElement = class RecordingActionTreeElement extends WI.GeneralTreeElement
{
    constructor(representedObject, index, recordingType)
    {
        console.assert(representedObject instanceof WI.RecordingAction);

        let {titleFragment, copyText} = WI.RecordingActionTreeElement._generateDOM(representedObject, recordingType);
        let classNames = WI.RecordingActionTreeElement._getClassNames(representedObject);

        const subtitle = null;
        super(classNames, titleFragment, subtitle, representedObject);

        this._index = index;
        this._copyText = copyText;
    }

    // Static

    static _generateDOM(recordingAction, recordingType)
    {
        function createParameterElement(parameter, swizzleType) {
            let parameterElement = document.createElement("span");
            parameterElement.classList.add("parameter");

            switch (swizzleType) {
            case WI.Recording.Swizzle.Number:
                parameterElement.textContent = parameter.maxDecimals(2);
                break;

            case WI.Recording.Swizzle.Boolean:
                parameterElement.textContent = parameter;
                break;

            case WI.Recording.Swizzle.String:
                parameterElement.textContent = JSON.stringify(parameter);
                break;

            case WI.Recording.Swizzle.Array:
                parameterElement.classList.add("swizzled");
                parameterElement.textContent = JSON.stringify(parameter);
                break;

            case WI.Recording.Swizzle.TypedArray:
            case WI.Recording.Swizzle.Image:
            case WI.Recording.Swizzle.ImageData:
            case WI.Recording.Swizzle.DOMMatrix:
            case WI.Recording.Swizzle.Path2D:
            case WI.Recording.Swizzle.CanvasGradient:
            case WI.Recording.Swizzle.CanvasPattern:
            case WI.Recording.Swizzle.WebGLBuffer:
            case WI.Recording.Swizzle.WebGLFramebuffer:
            case WI.Recording.Swizzle.WebGLRenderbuffer:
            case WI.Recording.Swizzle.WebGLTexture:
            case WI.Recording.Swizzle.WebGLShader:
            case WI.Recording.Swizzle.WebGLProgram:
            case WI.Recording.Swizzle.WebGLUniformLocation:
                parameterElement.classList.add("swizzled");
                parameterElement.textContent = WI.Recording.displayNameForSwizzleType(swizzleType);
                break;
            }

            if (!parameterElement.textContent) {
                parameterElement.classList.add("invalid");
                parameterElement.textContent = swizzleType === WI.Recording.Swizzle.None ? parameter : WI.Recording.displayNameForSwizzleType(swizzleType);
            }

            return parameterElement;
        }

        let titleFragment = document.createDocumentFragment();
        let copyText = recordingAction.name;

        let nameContainer = titleFragment.appendChild(document.createElement("span"));
        nameContainer.classList.add("name");
        nameContainer.textContent = recordingAction.name;

        if (!recordingAction.parameters.length)
            return {titleFragment, copyText};

        let parametersContainer = titleFragment.appendChild(document.createElement("span"));
        parametersContainer.classList.add("parameters");

        if (recordingAction.isFunction)
            copyText += "(";
        else
            copyText += " = ";

        for (let i = 0; i < recordingAction.parameters.length; ++i) {
            let parameter = recordingAction.parameters[i];
            let swizzleType = recordingAction.swizzleTypes[i];
            let parameterElement = createParameterElement(parameter, swizzleType);
            parametersContainer.appendChild(parameterElement);

            if (i)
                copyText += ", ";

            copyText += parameterElement.textContent;
        }

        if (recordingAction.isFunction)
            copyText += ")";

        let colorParameters = recordingAction.getColorParameters();
        if (colorParameters.length) {
            let swatch = WI.RecordingActionTreeElement._createSwatchForColorParameters(colorParameters);
            let insertionIndex = recordingAction.parameters.indexOf(colorParameters[0]);
            let parameterElement = parametersContainer.children[insertionIndex];
            parametersContainer.insertBefore(swatch.element, parameterElement);

            if (recordingAction.swizzleTypes[insertionIndex] === WI.Recording.Swizzle.String) {
                parameterElement.textContent = swatch.value.toString();
                parameterElement.classList.add("color");
            }
        }

        return {titleFragment, copyText};
    }

    static _createSwatchForColorParameters(parameters)
    {
        let rgb = [];
        let color = null;

        switch (parameters.length) {
        case 1:
        case 2:
            if (typeof parameters[0] === "string")
                color = WI.Color.fromString(parameters[0]);
            else
                rgb = WI.Color.normalized2rgb(parameters[0], parameters[0], parameters[0]);
            break;

        case 4:
            rgb = WI.Color.normalized2rgb(parameters[0], parameters[1], parameters[2]);
            break;

        case 5:
            rgb = WI.Color.cmyk2rgb(...parameters);
            break;

        default:
            console.error("Unexpected number of color parameters.");
            return null;
        }

        if (!color) {
            let alpha = parameters.length === 1 ? 1 : parameters.lastValue;
            color = new WI.Color(WI.Color.Format.RGBA, [...rgb, alpha]);
            if (!color)
                return null;
        }

        const readOnly = true;
        return new WI.InlineSwatch(WI.InlineSwatch.Type.Color, color, readOnly);
    }

    static _getClassNames(recordingAction)
    {
        let classNames = ["action"];

        if (recordingAction instanceof WI.RecordingInitialStateAction) {
            classNames.push("initial-state");
            return classNames;
        }

        if (!recordingAction.isFunction)
            classNames.push("attribute");

        let actionClassName = WI.RecordingActionTreeElement._classNameForAction(recordingAction);
        if (actionClassName.length)
            classNames.push(actionClassName);

        if (recordingAction.isVisual)
            classNames.push("visual");

        if (!recordingAction.valid)
            classNames.push("invalid");

        return classNames;
    }

    static _classNameForAction(recordingAction)
    {
        function classNameForActionName(name) {
            switch (name) {
            case "arc":
            case "arcTo":
                return "arc";

            case "globalAlpha":
            case "globalCompositeOperation":
            case "setAlpha":
            case "setGlobalAlpha":
            case "setCompositeOperation":
            case "setGlobalCompositeOperation":
                return "composite";

            case "bezierCurveTo":
            case "quadraticCurveTo":
                return "curve";

            case "clearRect":
            case "fill":
            case "fillRect":
            case "fillText":
                return "fill";

            case "createImageData":
            case "drawFocusIfNeeded":
            case "drawImage":
            case "drawImageFromRect":
            case "filter":
            case "getImageData":
            case "imageSmoothingEnabled":
            case "imageSmoothingQuality":
            case "putImageData":
            case "webkitGetImageDataHD":
            case "webkitPutImageDataHD":
            case "webkitImageSmoothingEnabled":
                return "image";

            case "getLineDash":
            case "lineCap":
            case "lineDashOffset":
            case "lineJoin":
            case "lineWidth":
            case "miterLimit":
            case "setLineCap":
            case "setLineDash":
            case "setLineJoin":
            case "setLineWidth":
            case "setMiterLimit":
            case "webkitLineDash":
            case "webkitLineDashOffset":
                return "line-style";

            case "closePath":
            case "lineTo":
                return "line-to";

            case "beginPath":
            case "moveTo":
                return "move-to";

            case "isPointInPath":
                return "point-in-path";

            case "isPointInStroke":
                return "point-in-stroke";

            case "clearShadow":
            case "setShadow":
            case "shadowBlur":
            case "shadowColor":
            case "shadowOffsetX":
            case "shadowOffsetY":
                return "shadow";

            case "createLinearGradient":
            case "createPattern":
            case "createRadialGradient":
            case "fillStyle":
            case "setFillColor":
            case "setStrokeColor":
            case "strokeStyle":
                return "style";

            case "stroke":
            case "strokeRect":
            case "strokeText":
                return "stroke";

            case "direction":
            case "font":
            case "measureText":
            case "textAlign":
            case "textBaseline":
                return "text";

            case "getTransform":
            case "resetTransform":
            case "rotate":
            case "scale":
            case "setTransform":
            case "transform":
            case "translate":
                return "transform";

            case "clip":
            case "ellipse":
            case "rect":
            case "restore":
            case "save":
                return name;
            }

            console.warn("No class name for action " + name);
            return "";
        }

        const name = recordingAction.name;
        let className = WI.RecordingActionTreeElement._memoizedActionClassNames.get(name);
        if (!className) {
            className = classNameForActionName(name);
            WI.RecordingActionTreeElement._memoizedActionClassNames.set(name, className);
        }

        return className;
    }

    // Public

    get index() { return this._index; }

    get filterableData()
    {
        let text = [];

        function getText(stringOrElement) {
            if (typeof stringOrElement === "string")
                text.push(stringOrElement);
            else if (stringOrElement instanceof Node)
                text.push(stringOrElement.textContent);
        }

        getText(this._mainTitleElement || this._mainTitle);
        getText(this._subtitleElement || this._subtitle);

        return {text};
    }

    // Protected

    customTitleTooltip()
    {
        return this._copyText;
    }

    onattach()
    {
        super.onattach();

        this.element.dataset.index = this._index.toLocaleString();
    }

    populateContextMenu(contextMenu, event)
    {
        contextMenu.appendItem(WI.UIString("Copy Action"), () => {
            InspectorFrontendHost.copyText("context." + this._copyText + ";");
        });

        contextMenu.appendSeparator();

        let callFrame = this.representedObject.trace[0];
        if (callFrame) {
            contextMenu.appendItem(WI.UIString("Reveal in Resources Tab"), () => {
                WI.showSourceCodeLocation(callFrame.sourceCodeLocation, {
                    ignoreNetworkTab: true,
                    ignoreSearchTab: true,
                });
            });

            contextMenu.appendSeparator();
        }

        super.populateContextMenu(contextMenu, event);
    }
};

WI.RecordingActionTreeElement._memoizedActionClassNames = new Map;
