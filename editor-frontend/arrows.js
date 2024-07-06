var SIZE_OF_ARROWHEAD = 5;
var COLLISION_MARGIN = 20;
var COLLISION_INTERVALS = 10;
var COLLISION_PRECISION = 4;
var LABEL_LINE_HEIGHT = 18;
var LABEL_BOX_PADDING = 5;
var Arrow = /** @class */ (function () {
    function Arrow(sourceNode, mouse) {
        this.isBeingInteractedWith = false;
        this.label = null;
        this.labelIsBeingEdited = false;
        this.sourceNode = sourceNode;
        this.sourceNode.addOutgoingArrow(this);
        this.mouse = { x: 0, y: 0 };
        this.updateEndPointFromMouse(mouse);
    }
    Arrow.prototype.draw = function (ctx) {
        ctx.save();
        ctx.beginPath();
        // Start at center of source node
        var center = this.sourceNode.center();
        ctx.moveTo(center.x, center.y);
        // Draw a quadratic curve to either the target node or the mouse
        if (this.targetNode === undefined) {
            ctx.quadraticCurveTo(this.control.x, this.control.y, this.mouse.x, this.mouse.y);
        }
        else {
            center = this.targetNode.whereIsArrowEndpoint(this.control);
            ctx.quadraticCurveTo(this.control.x, this.control.y, center.x, center.y);
        }
        ctx.strokeStyle = "black";
        ctx.lineWidth = 3;
        // Use a highlight when being interacted with
        if (this.isBeingInteractedWith) {
            ctx.shadowColor = "blue";
            ctx.shadowBlur = 5;
        }
        ctx.stroke();
        ctx.shadowColor = "transparent";
        // Draw arrowhead
        ctx.save();
        var d = vSubtract(center, this.control);
        var direction = Math.atan2(d.y, d.x);
        ctx.translate(center.x, center.y);
        ctx.rotate(direction);
        ctx.beginPath();
        ctx.moveTo(SIZE_OF_ARROWHEAD, 0);
        ctx.lineTo(-SIZE_OF_ARROWHEAD, SIZE_OF_ARROWHEAD);
        ctx.lineTo(-SIZE_OF_ARROWHEAD, -SIZE_OF_ARROWHEAD);
        ctx.fillStyle = "black";
        ctx.fill();
        ctx.restore();
        // Draw label
        if (this.label != null) {
            ctx.font = "16px Sans-serif";
            ctx.textAlign = "center";
            ctx.textBaseline = "top";
            // Do some measuring
            var labelWidth = null;
            for (var i = 0; i < this.label.length; i++) {
                var nextLine = ctx.measureText(this.label[i]);
                if (labelWidth === null || nextLine.width > labelWidth)
                    labelWidth = nextLine.width;
            }
            // Find midpoint of quadratic curve
            var labelPosition = quadraticInterpolation(this.sourceNode.center(), this.control, this.targetNode.center(), 0.5);
            // Move context to label position
            ctx.translate(labelPosition.x, labelPosition.y - LABEL_LINE_HEIGHT * this.label.length / 2);
            // Draw background box
            var boxWidth = labelWidth + LABEL_BOX_PADDING * 2;
            var boxHeight = this.label.length * LABEL_LINE_HEIGHT + LABEL_BOX_PADDING * 2;
            ctx.fillStyle = "white";
            ctx.fillRect(-boxWidth / 2, -LABEL_BOX_PADDING, boxWidth, boxHeight);
            // Draw each line
            ctx.fillStyle = "black";
            for (var i = 0; i < this.label.length; i++) {
                ctx.fillText(this.label[i], 0, i * LABEL_LINE_HEIGHT);
            }
        }
        ctx.restore();
    };
    Arrow.prototype.setTargetNode = function (node) {
        if (this.targetNode != null) {
            // Disconnect TODO
        }
        this.targetNode = node;
        this.targetNode.addIncomingArrow(this);
        // TODO: update control point too to keep line straight
    };
    // Only use during creation
    Arrow.prototype.updateEndPointFromMouse = function (mouse) {
        this.mouse.x = mouse.x;
        this.mouse.y = mouse.y;
        var sourceCenter = this.sourceNode.center();
        this.control = {
            x: (sourceCenter.x + mouse.x) / 2,
            y: (sourceCenter.y + mouse.y) / 2
        };
    };
    Arrow.prototype.prepareForDeletion = function () {
        // Disconnect from source node
        if (this.sourceNode != null) {
            this.sourceNode.removeOutgoingArrow(this);
        }
        // Disconnect from target node
        if (this.targetNode != null) {
            this.targetNode.removeIncomingArrow(this);
        }
    };
    Arrow.prototype.endPointHasMovedByAmount = function (change) {
        this.control.x += change.x / 2;
        this.control.y += change.y / 2;
    };
    Arrow.prototype.startInteraction = function () {
        this.isBeingInteractedWith = true;
    };
    Arrow.prototype.stopInteraction = function () {
        this.isBeingInteractedWith = false;
        this.labelIsBeingEdited = false;
        if (this.label != null && this.label.length === 1 && this.label[0].length === 0)
            this.label = null;
    };
    Arrow.prototype.checkForCollisionWithPoint = function (point) {
        var start = this.sourceNode.center();
        var end = this.targetNode.center();
        var control = this.control;
        // Do bounding box first
        var left = Math.min(start.x, control.x, end.x) - COLLISION_MARGIN;
        var right = Math.max(start.x, control.x, end.x) + COLLISION_MARGIN;
        var top = Math.min(start.y, control.y, end.y) - COLLISION_MARGIN;
        var bottom = Math.max(start.y, control.y, end.y) + COLLISION_MARGIN;
        if (point.x < left || point.x > right || point.y < top || point.y > bottom)
            return { collision: false, t: null };
        // Generate points on the line and find the closest
        var best = null;
        var bestT;
        var INCREMENT = 1 / COLLISION_INTERVALS;
        for (var i = INCREMENT; i < 1; i += INCREMENT) {
            var testPoint = quadraticInterpolation(start, control, end, i);
            var distance = vDistance(testPoint, point);
            if (best == null || distance < best) {
                best = distance;
                bestT = i;
            }
        }
        // Do a search around
        var currentT = bestT;
        var interval = INCREMENT / 2;
        for (var i = 0; i < COLLISION_PRECISION; i++) {
            var left_1 = quadraticInterpolation(start, control, end, currentT - interval);
            var right_1 = quadraticInterpolation(start, control, end, currentT + interval);
            if (vDistance(left_1, point) < vDistance(right_1, point)) {
                currentT -= interval;
            }
            else {
                currentT += interval;
            }
            interval /= 2;
        }
        // Compare to end points
        var currentBestT = currentT;
        if (vDistance(start, point) < vDistance(quadraticInterpolation(start, control, end, currentBestT), point)) {
            currentBestT = 0;
        }
        if (vDistance(end, point) < vDistance(quadraticInterpolation(start, control, end, currentBestT), point)) {
            currentBestT = 1;
        }
        // Make sure final point is within margin
        return { collision: vDistance(quadraticInterpolation(start, control, end, currentBestT), point) <= COLLISION_MARGIN, t: currentBestT };
    };
    Arrow.prototype.onKeyPressed = function (e) {
        // Check if the user wants to start editing the label
        if (!this.labelIsBeingEdited) {
            if (e.key.length === 1) {
                this.labelIsBeingEdited = true;
                this.label = [""];
            }
            else {
                return false;
            }
        }
        // Leave if the user is not editing the label
        if (!this.labelIsBeingEdited)
            return;
        // If user enters text, add it to the label
        if (e.key.length === 1) {
            this.label[this.label.length - 1] += e.key;
            return true;
        }
        // If user presses enter, add a new line
        if (e.key === "Enter") {
            this.label.push("");
            return true;
        }
        // If user presses backspace, undo the previous action
        if (e.key === "Backspace") {
            if (this.label[this.label.length - 1].length > 0) {
                // Remove most recently typed character
                var lastLine = this.label[this.label.length - 1];
                this.label[lastLine.length - 1] = lastLine.substring(0, lastLine.length - 2);
                return true;
            }
            else {
                // Remove last line
                this.label.pop();
                if (this.label.length === 0)
                    this.label.push("");
                return true;
            }
        }
    };
    Arrow.prototype.bendArrow = function (change, t) {
        var weight = 1 / (2 * t * (1 - t));
        change = vScale(change, weight);
        this.control = vAdd(this.control, change);
    };
    return Arrow;
}());
function quadraticInterpolation(start, control, end, t) {
    var a = vSubtract(control, start);
    var b = vSubtract(end, control);
    a = vAdd(start, vScale(a, t));
    b = vAdd(control, vScale(b, t));
    var c = vSubtract(b, a);
    c = vAdd(a, vScale(c, t));
    return c;
}
