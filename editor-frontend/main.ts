
// ### #   # ### #####
//  #  ##  #  #    #
//  #  # # #  #    #
//  #  #  ##  #    #
// ### #   # ###   #

// Initialize environment
let divStack = document.getElementById("stack");
let surface = document.createElement("div"); // TODO: alter canvas drawing order
surface.classList.add("surface");
divStack.appendChild(surface);
let canvas = document.createElement("canvas");
canvas.width = surface.offsetWidth;
canvas.height = surface.offsetHeight;
surface.appendChild(canvas);
let ctx = canvas.getContext("2d");
let selectionBox = document.createElement("div");
selectionBox.id = "selection-box";
surface.appendChild(selectionBox);

// Settings
const ARROW_HANDLE_MARGIN = 20;
const VIEW_MARGIN = 400;
const SCROLL_SENSITIVITY = 0.3;;

// Globals
let _state: "waiting" |
            "panning" |
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
let _nodeThatWasMostRecentlySelected: GraphNode;

let nodes: GraphNode[] = [];
let selectedNodes: GraphNode[] = [];
let arrows: Arrow[] = [];

let mouse = {x: 0, y: 0};
let prevMouse = {x: 0, y: 0};

// Allow the user to pan the view, but don't let them pan too far
let _topLeftBounderyMarker = {x: canvas.width / 2, y: canvas.height / 2};
let _bottomRightBounderyMarker = {x: canvas.width / 2, y: canvas.height / 2};

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
}, false);

// Key Down
window.addEventListener("keydown", function(e: KeyboardEvent) {
  if(_state === "waiting") {
    e.stopPropagation();
    // Backspace deletes selected nodes
    if(e.keyCode == 8) {
      deleteSelectedItems(); // TODO: add undo button
      return;
    }
    // User started typing without interacting with a node
    if(e.key.length == 1) {
      // Has the user recently selected a node?
      if(_nodeThatWasMostRecentlySelected != null) {
        // Edit that node
        if(isTextNode(_nodeThatWasMostRecentlySelected)) _nodeThatWasMostRecentlySelected.clearContents();
        if(isLabelNode(_nodeThatWasMostRecentlySelected)) _nodeThatWasMostRecentlySelected.clearContents();
        switchToStateInteracting(_nodeThatWasMostRecentlySelected);
      } else {
        // Make a new node to type in instead
        const newNode = new TextNode(getNewNodeContainer(), mouseEventFromNode, mouse);
        nodes.push(newNode);
        recalculateCanvasBounderies();
        switchToStateInteracting(newNode);
      }
    }
  }
  else if(_state === "interacting with arrow") {
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
  dragCanvas(change);
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
    // Check if user clicked on an arrow
    let {arrow, t} = getArrowAt(mouse);
    if(arrow != null) {
      // Drag that arrow
      switchToStateDraggingArrow(arrow, t);
    } else {
      // The user clicked away from the node being interacted with
      switchToStateWaiting();
    }
  }
  else if(_state === "interacting with arrow") {
    // Check if user clicked on an arrow
    let {arrow, t} = getArrowAt(mouse);
    if(arrow != null) {
      // Drag that arrow
      switchToStateDraggingArrow(arrow, t);
    } else {
      // User has clicked away from the current arrow
      switchToStateWaiting();
    }
  }
  else if(_state === "waiting") {
    // Check if user clicked on an arrow
    let {arrow, t} = getArrowAt(mouse);
    if(arrow != null) {
      switchToStateDraggingArrow(arrow, t);
    }
    else if(e.type === "mousedown") {
      let em = e as MouseEvent;
      if(em.button === 2) {
        // Otherwise, start dragging the screen
        switchToStatePanning();
      }
      else {
        switchToStateSelecting();
      }
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
  // Find the amount the mouse moved
  let change = {x: 0, y: 0};
  change.x = mouse.x - prevMouse.x;
  change.y = mouse.y - prevMouse.y;

  if(_state === "dragging") {
    _userMightBeTryingToInteractWithNode = false;
    // Add the change to all the selected nodes
    for(var i = 0; i < selectedNodes.length; i++) {
      selectedNodes[i].dragByAmount(change);
    }
    // Redraw the canvas so that arrows move as well
    redrawCanvas();
    // Recalculate where the bounds of the canvas are
    recalculateCanvasBounderies();
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
  else if(_state === "panning") {
    _userMightBeTryingToMakeALabelNode = false;
    dragCanvas(change);
  }
  else if(_state === "selecting") {
    selectionBox.style.width = Math.abs(mouse.x - _mousePositionAtStartOfSelection.x) + "px";
    selectionBox.style.height = Math.abs(mouse.y - _mousePositionAtStartOfSelection.y) + "px";
    let left = Math.min(_mousePositionAtStartOfSelection.x, mouse.x);
    let top = Math.min(_mousePositionAtStartOfSelection.y, mouse.y);
    selectionBox.style.transform = "translate3d(" + left + "px, " + top + "px, 0)";
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
    if(_nodeThatArrowMightConnectTo != null && _nodeThatArrowMightConnectTo != _arrowThatIsBeingCreated.sourceNode) {
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
  else if(_state === "panning") {
    if(e.type === "mouseup") {
      let em = e as MouseEvent;
      if(em.button === 2) {
        if(_userMightBeTryingToMakeALabelNode) {
          // Make a label node
          const newNode = new LabelNode(getNewNodeContainer(), mouseEventFromNode, mouse);
          nodes.push(newNode);
          switchToStateInteracting(newNode);
        } else {
          // Go back to waiting state
          switchToStateWaiting();
        }
      }
    }
  }
  else if(_state === "selecting") {
    // Find the bounding box of the selection
    let left = Math.min(_mousePositionAtStartOfSelection.x, mouse.x);
    let top = Math.min(_mousePositionAtStartOfSelection.y, mouse.y);
    let right = Math.max(_mousePositionAtStartOfSelection.x, mouse.x);
    let bottom = Math.max(_mousePositionAtStartOfSelection.y, mouse.y);
    // Switch to waiting state
    switchToStateWaiting();
    // Add nodes within bounds to selection
    for(let i = 0; i < nodes.length; i++) {
      if(nodes[i].position.x < left) continue;
      if(nodes[i].position.y < top) continue;
      if(nodes[i].position.x + nodes[i].container.offsetWidth > right) continue;
      if(nodes[i].position.y + nodes[i].container.offsetHeight > bottom) continue;
      addNodeToSelection(nodes[i]);
    }
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
    if((e.type === "touchstart" || e.type === "mousedown") && node != _nodeThatIsBeingInteractedWith) {
      switchToStateDragging(node, e.shiftKey);
    }
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
let _userMightBeTryingToMakeALabelNode: boolean;
function switchToStatePanning() {
  resetState();
  _state = "panning";
  console.log("panning");
  _userMightBeTryingToMakeALabelNode = true;
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
let _mousePositionAtStartOfSelection: Vector;
function switchToStateSelecting() {
  resetState();
  _state = "selecting";
  console.log("selecting");
  selectionBox.style.transform = "translate3d(" + mouse.x + "px, " + mouse.y + "px, 0)";
  selectionBox.style.width = "0";
  selectionBox.style.height = "0";
  selectionBox.style.visibility = "visible";
  surface.appendChild(selectionBox);
  _mousePositionAtStartOfSelection = vCopy(mouse);
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
      _nodeThatIsBeingInteractedWith = null;
      break;
    case "panning":
      _userMightBeTryingToMakeALabelNode = null;
      break;
    case "dragging":
      _nodeThatIsDirectTargetOfDrag = null;
      _userMightBeTryingToInteractWithNode = false;
      break;
    case "selecting":
      selectionBox.style.visibility = "collapse";
      _mousePositionAtStartOfSelection = null;
      break;
    case "interacting":
      if(isLabelNode(_nodeThatIsBeingInteractedWith) && _nodeThatIsBeingInteractedWith.isEmpty()) {
        deleteNode(_nodeThatIsBeingInteractedWith);
      }
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
  _nodeThatWasMostRecentlySelected = null;
}
function addNodeToSelection(node: GraphNode) {
  if(!node.isSelected) selectedNodes.push(node);
  node.isSelected = true;
  _nodeThatWasMostRecentlySelected = node;
  node.container.classList.add("selected");
}
function deleteSelectedItems() {
  for(var i = 0; i < selectedNodes.length; i++) {
    deleteNode(selectedNodes[i]);
  }
  // Cleanup
  _nodeThatHasArrowHandle = null; // TODO: this shouldn't be managed here
  redrawCanvas();
}
function deleteNode(node: GraphNode) {
  // Remove from selection
  removeFromArray(node, selectedNodes);
  // Remove connections
  while(node.outgoingArrows.length > 0) {
    let arrow = node.outgoingArrows[0];
    arrow.prepareForDeletion();
    removeFromArray(arrow, arrows);
  }
  while(node.incomingArrows.length > 0) {
    let arrow = node.incomingArrows[0];
    arrow.prepareForDeletion();
    removeFromArray(arrow, arrows);
  }
  // Remove this node
  removeFromArray(node, nodes);
  surface.removeChild(node.container);
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
  /* Draw boundery area
  ctx.save();
  ctx.shadowColor = "lightgray";
  ctx.shadowBlur = 30;
  ctx.shadowOffsetX = 10;
  ctx.shadowOffsetY = 10;
  const dimensions = vSubtract(_bottomRightBounderyMarker, _topLeftBounderyMarker);
  ctx.fillRect(_topLeftBounderyMarker.x, _topLeftBounderyMarker.y, dimensions.x, dimensions.y);
  ctx.restore();
  //*/
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
function vDistanceSquared(a: Vector, b: Vector) {
  const diffX = a.x - b.x;
  const diffY = a.y - b.y;
  return diffX * diffX + diffY * diffY;
}
function vCopy(v: Vector) {
  return {
    x: v.x,
    y: v.y
  };
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
function vNormalize(v: Vector) {
  let length = vLength(v);
  return vScale(v, 1/length);
}
function vDot(a: Vector, b: Vector) {
  return a.x * b.x + a.y * b.y;
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
function getArrowAt(point: Vector): {arrow: Arrow | null, t: number | null} {
  for(let i = 0; i < arrows.length; i++) {
    let {collision, t} = arrows[i].checkForCollisionWithPoint(point);
    if(collision) return {arrow: arrows[i], t: t};
  }
  return {arrow: null, t: null};
}
function dragCanvas(change: Vector) {
  // Prevent user from scrolling too far away
  if(_topLeftBounderyMarker.x > canvas.width/2 && change.x > 0) change.x = 0;
  if(_bottomRightBounderyMarker.x < canvas.width/2 && change.x < 0) change.x = 0;
  if(_topLeftBounderyMarker.y > canvas.height/2 && change.y > 0) change.y = 0;
  if(_bottomRightBounderyMarker.y < canvas.height/2 && change.y < 0) change.y = 0;
  // Move boundery
  _topLeftBounderyMarker = vAdd(_topLeftBounderyMarker, change);
  _bottomRightBounderyMarker = vAdd(_bottomRightBounderyMarker, change);
  // Update positions of all objects
  for(var i = 0; i < nodes.length; i++) {
    // TODO: don't drag the node being dragged if _state === "dragging"
    nodes[i].dragByAmount(change);
  }
  // Update canvas
  redrawCanvas();
}
function recalculateCanvasBounderies() {
  if(nodes.length > 0) {
    // Set both to the center of a random node
    const center = nodes[0].center();
    _topLeftBounderyMarker = vCopy(center);
    _bottomRightBounderyMarker = vCopy(center);
  } else {
    // Reset to default and hope that it's not outside where it's supposed to be
    _topLeftBounderyMarker = {x: canvas.width / 2, y: canvas.height / 2};
    _bottomRightBounderyMarker = {x: canvas.width / 2, y: canvas.height / 2};
  }
  // Stretch bounderies to fit all nodes
  for(let i = 0; i < nodes.length; i++) {
    let center = nodes[i].center();
    let radius = nodes[i].radius();
    if(center.x - radius < _topLeftBounderyMarker.x) _topLeftBounderyMarker.x = center.x - radius;
    if(center.x + radius > _bottomRightBounderyMarker.x) _bottomRightBounderyMarker.x = center.x + radius;
    if(center.y - radius < _topLeftBounderyMarker.y) _topLeftBounderyMarker.y = center.y - radius;
    if(center.y + radius > _bottomRightBounderyMarker.y) _bottomRightBounderyMarker.y = center.y + radius;
  }
}
