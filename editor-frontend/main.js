// ### #   # ### #####
//  #  ##  #  #    #
//  #  # # #  #    #
//  #  #  ##  #    #
// ### #   # ###   #
// Initialize environment
var divStack = document.getElementById("stack"); // TODO: allow clicking on another node while interacting with one node
var surface = document.createElement("div"); // TODO: alter canvas drawing order
surface.classList.add("surface");
divStack.appendChild(surface);
var canvas = document.createElement("canvas");
canvas.width = surface.offsetWidth;
canvas.height = surface.offsetHeight;
surface.appendChild(canvas);
var ctx = canvas.getContext("2d");
// Settings
var ARROW_HANDLE_MARGIN = 20;
var VIEW_MARGIN = 400;
var SCROLL_SENSITIVITY = 0.3;
;
// Globals
var _state = "waiting";
var _nodeThatIsBeingInteractedWith;
var _nodeThatIsDirectTargetOfDrag;
var _userMightBeTryingToInteractWithNode;
var _nodeThatIsBeingHovered;
var _nodeThatHasArrowHandle;
var _arrowHandleSystemIsOn = true;
var _arrowThatIsBeingCreated;
var _nodeThatArrowMightConnectTo;
var _arrowThatIsBeingInteractedWith;
var nodes = [];
var selectedNodes = [];
var arrows = [];
var mouse = { x: 0, y: 0 };
var prevMouse = { x: 0, y: 0 };
// Allow the user to pan the view, but don't let them pan too far
var _pan = { x: 0, y: 0 };
var _topLeftOfView = { x: 0, y: 0 };
var _bottomRightOfView = { x: 0, y: 0 };
redrawCanvas();
// ##### #   # ##### #   # #####  ###
// #     #   # #     ##  #   #   #
// ####  #   # ####  # # #   #    ###
// #      # #  #     #  ##   #       #
// #####   #   ##### #   #   #    ###
// Context Menu
window.addEventListener("contextmenu", function (e) {
    e.preventDefault();
    e.stopPropagation();
    // For now, right clicking creates a new node
    var newNode = new TextNode(getNewNodeContainer(), mouseEventFromNode);
    nodes.push(newNode);
    switchToStateInteracting(newNode);
    // TODO: create a context menu, and show that instead
}, false);
// Key Down
window.addEventListener("keydown", function (e) {
    if (_state === "waiting") {
        e.stopPropagation();
        switch (e.keyCode) {
            case 8: // Backspace
                deleteSelectedItems(); // TODO: add undo button
                break;
        }
    }
    if (_state === "interacting with arrow") {
        if (e.keyCode === 8 && !_arrowThatIsBeingInteractedWith.labelIsBeingEdited) {
            var arrowThatNeedsToBeDeleted = _arrowThatIsBeingInteractedWith;
            switchToStateWaiting();
            arrowThatNeedsToBeDeleted.prepareForDeletion();
            removeFromArray(arrowThatNeedsToBeDeleted, arrows);
            redrawCanvas();
        }
        else if (true) { // TODO: check if it's a character
            if (_arrowThatIsBeingInteractedWith.onKeyPressed(e))
                redrawCanvas();
        }
    }
}, false);
// Scrolling
window.addEventListener("wheel", function (e) {
    var change = { x: -e.deltaX * SCROLL_SENSITIVITY, y: -e.deltaY * SCROLL_SENSITIVITY };
    // Prevent user from scrolling too far away
    // TODO
    // Pan view
    _pan.x -= change.x;
    _pan.y -= change.y;
    // Update positions of all objects
    for (var i = 0; i < nodes.length; i++) {
        nodes[i].dragByAmount(change);
    }
    // Update canvas
    redrawCanvas();
}, true);
// Mouse Down
surface.addEventListener("mousedown", onMouseDown, false);
surface.addEventListener("touchstart", onMouseDown, false);
function onMouseDown(e) {
    clearSelection();
    if (_arrowHandleSystemIsOn) {
        if (_nodeThatHasArrowHandle != null) {
            // Make a new arrow
            switchToStateNewArrow(_nodeThatHasArrowHandle);
        }
    }
    if (_state === "interacting") {
        // The user clicked away from the node being interacted with
        switchToStateWaiting();
    }
    else if (_state === "interacting with arrow") {
        // User has clicked away from the arrow
        switchToStateWaiting();
    }
    else if (_state === "waiting") {
        var _a = getArrowAt(mouse), arrow = _a.arrow, t = _a.t;
        if (arrow != null) {
            switchToStateDraggingArrow(arrow, t);
        }
    }
}
// Mouse Move
surface.addEventListener("mousemove", onMouseMove, true);
surface.addEventListener("touchmove", onMouseMove, true);
function onMouseMove(e) {
    e.stopPropagation();
    if (_arrowHandleSystemIsOn) {
        if (_nodeThatHasArrowHandle != null && vDistance(_nodeThatHasArrowHandle.center(), mouse) > _nodeThatHasArrowHandle.radius() + ARROW_HANDLE_MARGIN) {
            // Mouse has left the arrow handle
            _nodeThatHasArrowHandle = null;
            redrawCanvas();
        }
    }
    // Collect mouse position
    if (e.type === "touchstart") {
        var et = e;
        mouse.x = et.touches[0].clientX;
        mouse.y = et.touches[0].clientY;
    }
    else {
        var em = e;
        mouse.x = em.clientX;
        mouse.y = em.clientY;
    }
    if (_state === "dragging") {
        _userMightBeTryingToInteractWithNode = false;
        // Find the amount the mouse moved
        var change = { x: 0, y: 0 };
        change.x = mouse.x - prevMouse.x;
        change.y = mouse.y - prevMouse.y;
        // Add the change to all the selected nodes
        for (var i = 0; i < selectedNodes.length; i++) {
            selectedNodes[i].dragByAmount(change);
        }
        // Redraw the canvas so that arrows move as well
        redrawCanvas();
    }
    else if (_state === "waiting") {
    }
    else if (_state === "new arrow") {
        if (_nodeThatArrowMightConnectTo == null) {
            _arrowThatIsBeingCreated.updateEndPointFromMouse(mouse);
            redrawCanvas();
        }
    }
    else if (_state === "dragging arrow") {
        var change = { x: 0, y: 0 };
        change.x = mouse.x - prevMouse.x;
        change.y = mouse.y - prevMouse.y;
        _arrowThatIsBeingDragged.bendArrow(change, _tValueOfArrowBeingDragged);
        redrawCanvas();
    }
    // Update prevMouse for the next time this function runs
    prevMouse.x = mouse.x;
    prevMouse.y = mouse.y;
}
// Mouse Up
surface.addEventListener("mouseup", onMouseUp, true);
surface.addEventListener("touchend", onMouseUp, true);
function onMouseUp(e) {
    e.stopPropagation();
    if (_state === "dragging") {
        if (_userMightBeTryingToInteractWithNode) {
            switchToStateInteracting(_nodeThatIsDirectTargetOfDrag);
        }
        else {
            switchToStateWaiting();
        }
    }
    else if (_state === "new arrow") {
        if (_nodeThatArrowMightConnectTo != null) {
            // Connect arrow to the node
            _arrowThatIsBeingCreated.setTargetNode(_nodeThatArrowMightConnectTo);
            // Start interaction with the arrow
            switchToStateInteractingWithArrow(_arrowThatIsBeingCreated);
        }
        else {
            // Delete the arrow
            _arrowThatIsBeingCreated.prepareForDeletion();
            removeFromArray(_arrowThatIsBeingCreated, arrows);
            redrawCanvas();
            switchToStateWaiting();
        }
    }
    else if (_state === "dragging arrow") {
        switchToStateInteractingWithArrow(_arrowThatIsBeingDragged);
    }
}
// Node Events
function mouseEventFromNode(node, e) {
    if (e.type === "mouseover") {
        _nodeThatIsBeingHovered = node;
        if (_arrowHandleSystemIsOn) {
            _nodeThatHasArrowHandle = _nodeThatIsBeingHovered;
            redrawCanvas();
        }
    }
    if (e.type === "mouseout") {
        if (node === _nodeThatIsBeingHovered) {
            _nodeThatIsBeingHovered = null;
        }
    }
    // Make sure to stop propagation of mousedown events so that the editor doesn't get them
    if (e.type === "mousedown" || e.type === "touchstart")
        e.stopPropagation();
    if (e.type === "contextmenu")
        e.preventDefault();
    if (_state === "interacting") {
    }
    else if (_state === "waiting") {
        if (e.type === "touchstart" || e.type === "mousedown") {
            switchToStateDragging(node, e.shiftKey);
        }
    }
    else if (_state === "dragging") {
        console.log("I didn't think this was possible: mouseEventFromNode, _state === 'dragging', e.type === '" + e.type + "'");
    }
    else if (_state === "selecting") {
    }
    else if (_state === "context menu") {
    }
    else if (_state === "new arrow") {
        if (e.type === "mouseover") {
            // Connect new arrow to this node
            _nodeThatArrowMightConnectTo = node;
            _arrowThatIsBeingCreated.updateEndPointFromMouse(_nodeThatArrowMightConnectTo.center());
            redrawCanvas();
        }
        if (e.type === "mouseout") {
            if (node !== _nodeThatArrowMightConnectTo)
                return;
            _nodeThatArrowMightConnectTo = null;
            _arrowThatIsBeingCreated.updateEndPointFromMouse(mouse);
            redrawCanvas();
        }
    }
    else if (_state === "interacting with arrow") {
        if (e.type === "mousedown" || e.type === "touchstart") {
            switchToStateDragging(node, e.shiftKey);
        }
    }
}
//  ###  #####  ###  ##### #####
// #       #   #   #   #   #
//  ###    #   #####   #   ####
//     #   #   #   #   #   #
//  ###    #   #   #   #   #####
function switchToStateWaiting() {
    resetState();
    _state = "waiting";
    console.log("waiting");
    turnOnArrowHandleSystem();
}
function switchToStateDragging(node, shiftButtonIsPressed) {
    resetState();
    _state = "dragging";
    console.log("dragging");
    // If you try to drag a node that's not selected, other nodes will get deselected
    if (!node.isSelected) {
        if (!shiftButtonIsPressed)
            clearSelection();
    }
    // Otherwise, the user might be trying to interact with the node
    else {
        _userMightBeTryingToInteractWithNode = true;
    }
    // Add current node to the selection, or at least make sure it's selected
    addNodeToSelection(node);
    // Record this node just in case
    _nodeThatIsDirectTargetOfDrag = node;
    // Bring it to the front
    surface.appendChild(node.container);
}
function switchToStateSelecting() {
    resetState();
    _state = "selecting";
    console.log("selecting");
}
function switchToStateInteracting(node) {
    resetState();
    _state = "interacting";
    console.log("interacting");
    _nodeThatIsBeingInteractedWith = node;
    node.startInteraction();
    turnOnArrowHandleSystem();
}
function switchToStateContextMenu() {
    resetState();
    _state = "context menu";
    console.log("context menu");
}
function switchToStateNewArrow(sourceNode) {
    resetState();
    _state = "new arrow";
    console.log("new arrow");
    _arrowThatIsBeingCreated = new Arrow(sourceNode, mouse);
    arrows.push(_arrowThatIsBeingCreated);
    redrawCanvas();
}
var _arrowThatIsBeingDragged = null;
var _tValueOfArrowBeingDragged = null;
function switchToStateDraggingArrow(arrow, t) {
    resetState();
    _state = "dragging arrow";
    console.log("dragging arrow");
    _arrowThatIsBeingDragged = arrow;
    _tValueOfArrowBeingDragged = t;
}
function switchToStateInteractingWithArrow(arrow) {
    resetState();
    _state = "interacting with arrow";
    console.log("interacting with arrow");
    _arrowThatIsBeingInteractedWith = arrow;
    _arrowThatIsBeingInteractedWith.startInteraction();
    turnOnArrowHandleSystem();
    redrawCanvas();
}
function resetState() {
    switch (_state) {
        case "waiting":
            turnOffArrowHandleSystem();
            break;
        case "dragging":
            resizeScreen();
            _nodeThatIsDirectTargetOfDrag = null;
            _userMightBeTryingToInteractWithNode = false;
            break;
        case "selecting":
            break;
        case "interacting":
            _nodeThatIsBeingInteractedWith.stopInteraction();
            _nodeThatIsBeingInteractedWith = null;
            turnOffArrowHandleSystem();
            break;
        case "context menu":
            break;
        case "new arrow":
            _arrowThatIsBeingCreated = null;
            _nodeThatArrowMightConnectTo = null;
            break;
        case "dragging arrow":
            _arrowThatIsBeingDragged = null;
            _tValueOfArrowBeingDragged = null;
            break;
        case "interacting with arrow":
            turnOffArrowHandleSystem();
            _arrowThatIsBeingInteractedWith.stopInteraction();
            _arrowThatIsBeingInteractedWith = null;
            turnOffArrowHandleSystem();
            break;
    }
}
// #   # ##### ##### #   #  ###  ####   ###
// ## ## #       #   #   # #   # #   # #
// # # # ####    #   ##### #   # #   #  ###
// #   # #       #   #   # #   # #   #     #
// #   # #####   #   #   #  ###  ####   ###
function getNewNodeContainer() {
    var container = document.createElement("div");
    container.classList.add("node-container");
    surface.appendChild(container);
    return container;
}
function clearSelection() {
    for (var i = 0; i < selectedNodes.length; i++) {
        selectedNodes[i].isSelected = false;
        selectedNodes[i].container.classList.remove("selected");
    }
    selectedNodes = [];
}
function addNodeToSelection(node) {
    if (!node.isSelected)
        selectedNodes.push(node);
    node.isSelected = true;
    node.container.classList.add("selected");
}
function deleteSelectedItems() {
    for (var i = 0; i < selectedNodes.length; i++) {
        var nodeToDelete = selectedNodes[i];
        // Remove connections
        while (nodeToDelete.outgoingArrows.length > 0) {
            var arrow = nodeToDelete.outgoingArrows[0];
            arrow.prepareForDeletion();
            removeFromArray(arrow, arrows);
        }
        while (nodeToDelete.incomingArrows.length > 0) {
            var arrow = nodeToDelete.incomingArrows[0];
            arrow.prepareForDeletion();
            removeFromArray(arrow, arrows);
        }
        // Remove this node
        removeFromArray(nodeToDelete, nodes);
        surface.removeChild(nodeToDelete.container);
    }
    selectedNodes = [];
    // TODO: also delete selected connections and other things
    // Cleanup
    _nodeThatHasArrowHandle = null;
    redrawCanvas();
}
function removeFromArray(item, array) {
    var index = array.indexOf(item);
    if (index > -1) {
        array.splice(index, 1);
    }
}
function redrawCanvas() {
    // Draw background
    ctx.fillStyle = "white";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    // Draw arrow handle
    if (_nodeThatHasArrowHandle != null) {
        ctx.beginPath();
        var center = _nodeThatHasArrowHandle.center();
        var radius = _nodeThatHasArrowHandle.radius() + ARROW_HANDLE_MARGIN;
        ctx.arc(center.x, center.y, radius, 0, Math.PI * 2);
        ctx.globalAlpha = 0.5;
        ctx.fillStyle = "lightblue";
        ctx.fill();
        ctx.strokeStyle = "lightblue";
        ctx.lineWidth = 3;
        ctx.stroke();
        ctx.globalAlpha = 1;
    }
    // Draw arrows// TODO: update end position of dragged arrow
    for (var i = 0; i < arrows.length; i++) {
        arrows[i].draw(ctx);
    }
}
function vDistance(a, b) {
    var diffX = a.x - b.x;
    var diffY = a.y - b.y;
    return Math.sqrt(diffX * diffX + diffY * diffY);
}
;
function vAdd(a, b) {
    return { x: a.x + b.x, y: a.y + b.y };
}
function vSubtract(a, b) {
    return { x: a.x - b.x, y: a.y - b.y };
}
function vLength(v) {
    return Math.sqrt(v.x * v.x + v.y * v.y);
}
function vScale(v, s) {
    return { x: v.x * s, y: v.y * s };
}
function turnOnArrowHandleSystem() {
    _arrowHandleSystemIsOn = true;
    if (_nodeThatIsBeingHovered != null) {
        _nodeThatHasArrowHandle = _nodeThatIsBeingHovered;
        redrawCanvas();
    }
}
function turnOffArrowHandleSystem() {
    _arrowHandleSystemIsOn = false;
    _nodeThatHasArrowHandle = null;
    redrawCanvas();
}
function resizeScreen() {
    var centerOfMovedNode = _nodeThatIsDirectTargetOfDrag.center();
    if (centerOfMovedNode.x < VIEW_MARGIN) {
    }
    if (centerOfMovedNode.y < VIEW_MARGIN) {
    }
    if (centerOfMovedNode.x > canvas.width - VIEW_MARGIN) {
    }
    if (centerOfMovedNode.y > canvas.height - VIEW_MARGIN) {
    }
}
function getArrowAt(point) {
    for (var i = 0; i < arrows.length; i++) {
        var _a = arrows[i].checkForCollisionWithPoint(point), collision = _a.collision, t = _a.t;
        if (collision)
            return { arrow: arrows[i], t: t };
    }
    return null;
}
