var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== "function" && b !== null)
            throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var GraphNode = /** @class */ (function () {
    function GraphNode(container, eventHandler) {
        this.wasMoved = false;
        this.outgoingArrows = [];
        this.incomingArrows = [];
        this.isBeingInteractedWith = false;
        this.isSelected = false;
        this.container = container;
        var self = this;
        this.position = { x: 0, y: 0 };
        this.makeSureGraphicPositionMatchesLogicalPosition();
        // Hovering
        container.addEventListener("mouseover", function (e) {
            eventHandler(self, e);
        });
        container.addEventListener("mouseout", function (e) {
            eventHandler(self, e);
        });
        // Mouse dragging
        container.addEventListener("mousedown", function (e) {
            eventHandler(self, e);
        });
        // Touch dragging
        container.addEventListener("touchstart", function (e) {
            eventHandler(self, e);
        });
        // Clicks
        container.addEventListener("click", function (e) {
            eventHandler(self, e);
        });
        container.addEventListener("contextmenu", function (e) {
            eventHandler(self, e);
        });
        // https://www.w3schools.com/jsref/obj_mouseevent.asp
    }
    GraphNode.prototype.dragByAmount = function (change) {
        this.position.x += change.x;
        this.position.y += change.y;
        this.makeSureGraphicPositionMatchesLogicalPosition();
        // Update connected arrows as well
        for (var i = 0; i < this.incomingArrows.length; i++) {
            this.incomingArrows[i].endPointHasMovedByAmount(change);
        }
        for (var i = 0; i < this.outgoingArrows.length; i++) {
            this.outgoingArrows[i].endPointHasMovedByAmount(change);
        }
    };
    GraphNode.prototype.makeSureGraphicPositionMatchesLogicalPosition = function () {
        this.container.style.transform = "translate3d(" + this.position.x + "px, " + this.position.y + "px, 0)";
    };
    GraphNode.prototype.addOutgoingArrow = function (arrow) {
        this.outgoingArrows.push(arrow);
    };
    GraphNode.prototype.addIncomingArrow = function (arrow) {
        this.incomingArrows.push(arrow);
    };
    GraphNode.prototype.removeOutgoingArrow = function (arrow) {
        removeFromArray(arrow, this.outgoingArrows);
    };
    GraphNode.prototype.removeIncomingArrow = function (arrow) {
        removeFromArray(arrow, this.incomingArrows);
    };
    GraphNode.prototype.radius = function () {
        return Math.sqrt(Math.pow(this.container.offsetWidth, 2) +
            Math.pow(this.container.offsetHeight, 2)) / 2;
    };
    GraphNode.prototype.center = function () {
        return {
            x: this.position.x + this.container.offsetWidth / 2,
            y: this.position.y + this.container.offsetHeight / 2
        };
    };
    return GraphNode;
}());
var TextNode = /** @class */ (function (_super) {
    __extends(TextNode, _super);
    function TextNode(container, eventHandler) {
        var _this = _super.call(this, container, eventHandler) || this;
        _this.ASSUMED_WIDTH_OF_CHARACTER = 20;
        _this.ASSUMED_HEIGHT_OF_LINE = 20;
        _this.MIN_WIDTH = 100;
        container.style.padding = "10px";
        container.style.border = "3px solid black";
        container.style.borderRadius = "5px";
        container.style.width = _this.MIN_WIDTH + "px";
        container.style.textAlign = "center";
        container.style.overflowWrap = "break-word";
        container.style.minHeight = "30px";
        container.style.backgroundColor = "white";
        return _this;
    }
    TextNode.prototype.startInteraction = function () {
        // Turn on active flag
        this.isBeingInteractedWith = true;
        // Turn on editing
        this.container.contentEditable = "true";
        // Keep the width constant until interaction is over
        this.container.style.width = this.container.clientWidth - 19.8 + "px";
        this.container.style.padding = "9.9px 10px";
        this.container.focus();
        // TODO: automatically select contents
    };
    TextNode.prototype.stopInteraction = function () {
        // Turn off active flag
        this.isBeingInteractedWith = false;
        // Turn off editing
        this.container.contentEditable = "false";
        // Reset the width and padding
        var assumedWidthOfText = this.container.innerHTML.length * this.ASSUMED_WIDTH_OF_CHARACTER;
        var assumedAreaOfText = assumedWidthOfText * this.ASSUMED_HEIGHT_OF_LINE;
        var desiredWidth = Math.pow(assumedAreaOfText, .5);
        this.container.style.width = (desiredWidth > this.MIN_WIDTH ? desiredWidth : this.MIN_WIDTH) + "px";
        this.container.style.padding = "10px";
    };
    TextNode.prototype.whereIsArrowEndpoint = function (arrowControlPoint) {
        // Calculate where a line from the control point to the center intersects the box
        var fromCenterToControl = vSubtract(arrowControlPoint, this.center());
        var xyRatio = Math.abs(fromCenterToControl.x) / Math.abs(fromCenterToControl.y);
        var matchDimension = xyRatio > this.container.offsetWidth / this.container.offsetHeight ? "width" : "height";
        switch (matchDimension) {
            case "width":
                return vAdd(this.center(), vScale(fromCenterToControl, (this.container.offsetWidth / 2 + 3) / Math.abs(fromCenterToControl.x)));
            case "height":
                return vAdd(this.center(), vScale(fromCenterToControl, (this.container.offsetHeight / 2 + 3) / Math.abs(fromCenterToControl.y)));
        }
    };
    return TextNode;
}(GraphNode));
