
// ### #   # ### #####
//  #  ##  #  #    #
//  #  # # #  #    #
//  #  #  ##  #    #
// ### #   # ###   #

// Initialize environment
let divStack = document.getElementById("stack"); // TODO: allow clicking on another node while interacting with one node
let surface = document.createElement("div"); // TODO: alter canvas drawing order
surface.classList.add("surface");
divStack.appendChild(surface);
let canvas = document.createElement("canvas");
canvas.width = surface.offsetWidth;
canvas.height = surface.offsetHeight;
surface.appendChild(canvas);
let ctx = canvas.getContext("2d");

// Settings
const ARROW_HANDLE_MARGIN = 20;
const VIEW_MARGIN = 400;
const SCROLL_SENSITIVITY = 0.3;;

// Globals
let _state: "waiting" |
            "dragging" |
            "selecting" |
            "interacting" |
            "context menu" |
            "new arrow" |
            "dragging arrow" |
            "interacting with arrow"
            = "waiting";
let _nodeThatIsBeingInteractedWith: GraphNode;
let _nodeThatIsDirectTargetOfDrag: GraphNode;
let _userMightBeTryingToInteractWithNode: boolean;
let _nodeThatIsBeingHovered: GraphNode;
let _nodeThatHasArrowHandle: GraphNode;
let _arrowHandleSystemIsOn: boolean = true;
let _arrowThatIsBeingCreated: Arrow;
let _nodeThatArrowMightConnectTo: GraphNode;
let _arrowThatIsBeingInteractedWith: Arrow;

let nodes: GraphNode[] = [];
let selectedNodes: GraphNode[] = [];
let arrows: Arrow[] = [];

let mouse = {x: 0, y: 0};
let prevMouse = {x: 0, y: 0};

// Allow the user to pan the view, but don't let them pan too far
let _pan = {x: 0, y: 0};
let _topLeftOfView = {x: 0, y: 0};
let _bottomRightOfView = {x: 0, y: 0};

redrawCanvas();

// ##### #   # ##### #   # #####  ###
// #     #   # #     ##  #   #   #
// ####  #   # ####  # # #   #    ###
// #      # #  #     #  ##   #       #
// #####   #   ##### #   #   #    ###

// Context Menu
window.addEventListener("contextmenu", function(e: MouseEvent) {
  e.preventDefault();
  e.stopPropagation();
  // For now, right clicking creates a new node
  const newNode = new TextNode(getNewNodeContainer(), mouseEventFromNode);
  nodes.push(newNode);
  switchToStateInteracting(newNode);
  // TODO: create a context menu, and show that instead
}, false);

// Key Down
window.addEventListener("keydown", function(e: KeyboardEvent) {
  if(_state === "waiting") {
    e.stopPropagation();
    switch(e.keyCode) {
      case 8: // Backspace
        deleteSelectedItems(); // TODO: add undo button
        break;
    }
  }
  if(_state === "interacting with arrow") {
    if(e.keyCode === 8 && !_arrowThatIsBeingInteractedWith.labelIsBeingEdited) {
      const arrowThatNeedsToBeDeleted = _arrowThatIsBeingInteractedWith;
      switchToStateWaiting();
      arrowThatNeedsToBeDeleted.prepareForDeletion();
      removeFromArray(arrowThatNeedsToBeDeleted, arrows);
      redrawCanvas();
    } else if(true) {
      if(_arrowThatIsBeingInteractedWith.onKeyPressed(e)) redrawCanvas();
    }
  }
}, false);

// Scrolling
window.addEventListener("wheel", function(e: WheelEvent) {
  const change = {x: -e.deltaX * SCROLL_SENSITIVITY, y: -e.deltaY * SCROLL_SENSITIVITY};
  // Prevent user from scrolling too far away
  // TODO
  // Pan view
  _pan.x -= change.x;
  _pan.y -= change.y;
  // Update positions of all objects
  for(var i = 0; i < nodes.length; i++) {
    nodes[i].dragByAmount(change);
  }
  // Update canvas
  redrawCanvas();
}, true);

// Mouse Down
surface.addEventListener("mousedown", onMouseDown, false);
surface.addEventListener("touchstart", onMouseDown, false);
function onMouseDown(e: MouseEvent | TouchEvent) {
  clearSelection();
  if(_arrowHandleSystemIsOn) {
    if(_nodeThatHasArrowHandle != null) {
      // Make a new arrow
      switchToStateNewArrow(_nodeThatHasArrowHandle);
    }
  }

  if(_state === "interacting") {
    // The user clicked away from the node being interacted with
    switchToStateWaiting();
  }
  else if(_state === "interacting with arrow") {
    // User has clicked away from the arrow
    switchToStateWaiting();
  }
  else if(_state === "waiting") {
    let {arrow, t} = getArrowAt(mouse);
    if(arrow != null) {
      switchToStateDraggingArrow(arrow, t);
    }
  }
}

// Mouse Move
surface.addEventListener("mousemove", onMouseMove, true);
surface.addEventListener("touchmove", onMouseMove, true);
function onMouseMove(e: MouseEvent | TouchEvent) {
  e.stopPropagation();

  if(_arrowHandleSystemIsOn) {
    if(_nodeThatHasArrowHandle != null && vDistance(_nodeThatHasArrowHandle.center(), mouse) > _nodeThatHasArrowHandle.radius() + ARROW_HANDLE_MARGIN) {
      // Mouse has left the arrow handle
      _nodeThatHasArrowHandle = null;
      redrawCanvas();
    }
  }

  // Collect mouse position
  if (e.type === "touchstart") {
    let et = e as TouchEvent;
    mouse.x = et.touches[0].clientX;
    mouse.y = et.touches[0].clientY;
  } else {
    let em = e as MouseEvent;
    mouse.x = em.clientX;
    mouse.y = em.clientY;
  }

  if(_state === "dragging") {
    _userMightBeTryingToInteractWithNode = false;
    // Find the amount the mouse moved
    let change = {x: 0, y: 0};
    change.x = mouse.x - prevMouse.x;
    change.y = mouse.y - prevMouse.y;
    // Add the change to all the selected nodes
    for(var i = 0; i < selectedNodes.length; i++) {
      selectedNodes[i].dragByAmount(change);
    }
    // Redraw the canvas so that arrows move as well
    redrawCanvas();
  }
  else if(_state === "waiting") {
  }
  else if(_state === "new arrow") {
    if(_nodeThatArrowMightConnectTo == null) {
      _arrowThatIsBeingCreated.updateEndPointFromMouse(mouse);
      redrawCanvas();
    }
  }
  else if(_state === "dragging arrow") {
    let change = {x: 0, y: 0};
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
function onMouseUp(e: MouseEvent | TouchEvent) {
  e.stopPropagation();
  if(_state === "dragging") {
    if(_userMightBeTryingToInteractWithNode) {
      switchToStateInteracting(_nodeThatIsDirectTargetOfDrag);
    } else {
      switchToStateWaiting();
    }
  }
  else if(_state === "new arrow") {
    if(_nodeThatArrowMightConnectTo != null) {
      // Connect arrow to the node
      _arrowThatIsBeingCreated.setTargetNode(_nodeThatArrowMightConnectTo);
      // Start interaction with the arrow
      switchToStateInteractingWithArrow(_arrowThatIsBeingCreated);
    } else {
      // Delete the arrow
      _arrowThatIsBeingCreated.prepareForDeletion();
      removeFromArray(_arrowThatIsBeingCreated, arrows);
      redrawCanvas();
      switchToStateWaiting();
    }
  }
  else if(_state === "dragging arrow") {
    switchToStateInteractingWithArrow(_arrowThatIsBeingDragged);
  }
}

// Node Events
function mouseEventFromNode(node: GraphNode, e: MouseEvent | TouchEvent) {
  if(e.type === "mouseover") {
    _nodeThatIsBeingHovered = node;
    if(_arrowHandleSystemIsOn) {
      _nodeThatHasArrowHandle = _nodeThatIsBeingHovered;
      redrawCanvas();
    }
  }
  if(e.type === "mouseout") {
    if(node === _nodeThatIsBeingHovered) {
      _nodeThatIsBeingHovered = null;
    }
  }

  // Make sure to stop propagation of mousedown events so that the editor doesn't get them
  if(e.type === "mousedown" || e.type === "touchstart") e.stopPropagation();

  if(e.type === "contextmenu") e.preventDefault();

  if(_state === "interacting") {
  }
  else if(_state === "waiting") {
    if(e.type === "touchstart" || e.type === "mousedown") {
      switchToStateDragging(node, e.shiftKey);
    }
  }
  else if(_state === "dragging") {
    console.log("I didn't think this was possible: mouseEventFromNode, _state === 'dragging', e.type === '" + e.type + "'");
  }
  else if(_state === "selecting") {
  }
  else if(_state === "context menu") {
  }
  else if(_state === "new arrow") {
    if(e.type === "mouseover") {
      // Connect new arrow to this node
      _nodeThatArrowMightConnectTo = node;
      _arrowThatIsBeingCreated.updateEndPointFromMouse(_nodeThatArrowMightConnectTo.center());
      redrawCanvas();
    }
    if(e.type === "mouseout") {
      if(node !== _nodeThatArrowMightConnectTo) return;
      _nodeThatArrowMightConnectTo = null;
      _arrowThatIsBeingCreated.updateEndPointFromMouse(mouse);
      redrawCanvas();
    }
  }
  else if(_state === "interacting with arrow") {
    if(e.type === "mousedown" || e.type === "touchstart") {
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
function switchToStateDragging(node: GraphNode, shiftButtonIsPressed: boolean) {
  resetState();
  _state = "dragging";
  console.log("dragging");
  // If you try to drag a node that's not selected, other nodes will get deselected
  if(!node.isSelected) {
    if(!shiftButtonIsPressed) clearSelection();
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
function switchToStateInteracting(node: GraphNode) {
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
function switchToStateNewArrow(sourceNode: GraphNode) { // TODO: after creating a new arrow, pressing alt will delete it
  resetState();
  _state = "new arrow";
  console.log("new arrow");
  _arrowThatIsBeingCreated = new Arrow(sourceNode, mouse);
  arrows.push(_arrowThatIsBeingCreated);
  redrawCanvas();
}
let _arrowThatIsBeingDragged: Arrow = null;
let _tValueOfArrowBeingDragged: number = null;
function switchToStateDraggingArrow(arrow: Arrow, t: number) {
  resetState();
  _state = "dragging arrow";
  console.log("dragging arrow");
  _arrowThatIsBeingDragged = arrow;
  _tValueOfArrowBeingDragged = t;
}
function switchToStateInteractingWithArrow(arrow: Arrow) {
  resetState();
  _state = "interacting with arrow";
  console.log("interacting with arrow");
  _arrowThatIsBeingInteractedWith = arrow;
  _arrowThatIsBeingInteractedWith.startInteraction();
  turnOnArrowHandleSystem();
  redrawCanvas();
}
function resetState() {
  switch(_state) {
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
  const container = document.createElement("div");
  container.classList.add("node-container");
  surface.appendChild(container);
  return container;
}
function clearSelection() {
  for(var i = 0; i < selectedNodes.length; i++) {
    selectedNodes[i].isSelected = false;
    selectedNodes[i].container.classList.remove("selected");
  }
  selectedNodes = [];
}
function addNodeToSelection(node: GraphNode) {
  if(!node.isSelected) selectedNodes.push(node);
  node.isSelected = true;
  node.container.classList.add("selected");
}
function deleteSelectedItems() {
  for(var i = 0; i < selectedNodes.length; i++) {
    const nodeToDelete = selectedNodes[i];
    // Remove connections
    while(nodeToDelete.outgoingArrows.length > 0) {
      let arrow = nodeToDelete.outgoingArrows[0];
      arrow.prepareForDeletion();
      removeFromArray(arrow, arrows);
    }
    while(nodeToDelete.incomingArrows.length > 0) {
      let arrow = nodeToDelete.incomingArrows[0];
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
function removeFromArray<T>(item: T, array: T[]) {
  const index = array.indexOf(item);
  if(index > -1) {
    array.splice(index, 1);
  }
}
function redrawCanvas() {
  // Draw background
  ctx.fillStyle = "white";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  // Draw arrow handle
  if(_nodeThatHasArrowHandle != null) {
    ctx.beginPath();
    const center = _nodeThatHasArrowHandle.center();
    const radius = _nodeThatHasArrowHandle.radius() + ARROW_HANDLE_MARGIN;
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
  for(var i = 0; i < arrows.length; i++) {
    arrows[i].draw(ctx);
  }
}
function vDistance(a: Vector, b: Vector) {
  const diffX = a.x - b.x;
  const diffY = a.y - b.y;
  return Math.sqrt(diffX * diffX + diffY * diffY);
}
interface Vector {
  x: number,
  y: number,
};
function vAdd(a: Vector, b: Vector) {
  return {x: a.x + b.x, y: a.y + b.y};
}
function vSubtract(a: Vector, b: Vector) {
  return {x: a.x - b.x, y: a.y - b.y};
}
function vLength(v: Vector) {
  return Math.sqrt(v.x * v.x + v.y * v.y);
}
function vScale(v: Vector, s: number) {
  return {x: v.x * s, y: v.y * s};
}
function turnOnArrowHandleSystem() {
  _arrowHandleSystemIsOn = true;
  if(_nodeThatIsBeingHovered != null) {
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
  let centerOfMovedNode = _nodeThatIsDirectTargetOfDrag.center();
  if(centerOfMovedNode.x < VIEW_MARGIN) {
  }
  if(centerOfMovedNode.y < VIEW_MARGIN) {
  }
  if(centerOfMovedNode.x > canvas.width - VIEW_MARGIN) {
  }
  if(centerOfMovedNode.y > canvas.height - VIEW_MARGIN) {
  }
}
function getArrowAt(point: Vector): {arrow: Arrow | null, t: number | null} {
  for(let i = 0; i < arrows.length; i++) {
    let {collision, t} = arrows[i].checkForCollisionWithPoint(point);
    if(collision) return {arrow: arrows[i], t: t};
  }
  return null;
}
