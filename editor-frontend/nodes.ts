
abstract class GraphNode {
  container: HTMLElement;
  wasMoved = false;
  outgoingArrows: Arrow[] = [];
  incomingArrows: Arrow[] = [];
  isBeingInteractedWith = false;
  isSelected = false;
  position: Vector;

  constructor(container: HTMLElement, eventHandler: (node: GraphNode, e: MouseEvent | TouchEvent) => void, position: Vector) {
    this.container = container;
    const self = this;
    this.position = vCopy(position);
    this.makeSureGraphicPositionMatchesLogicalPosition();
    // Hovering
    container.addEventListener("mouseover", function(e: MouseEvent) { // TODO: move these to main.ts
      eventHandler(self, e);
    });
    container.addEventListener("mouseout", function(e: MouseEvent) {
      eventHandler(self, e);
    });
    // Mouse dragging
    container.addEventListener("mousedown", function(e: MouseEvent) {
      eventHandler(self, e);
    });
    // Touch dragging
    container.addEventListener("touchstart", function(e: TouchEvent) {
      eventHandler(self, e);
    });
    // Clicks
    container.addEventListener("click", function(e: MouseEvent) {
      eventHandler(self, e);
    });
    container.addEventListener("contextmenu", function(e: MouseEvent) {
      eventHandler(self, e);
    });
    // https://www.w3schools.com/jsref/obj_mouseevent.asp
  }
  abstract startInteraction(): void;
  abstract stopInteraction(): void;
  abstract collidesWithPoint(point: Vector, margin: number): boolean;
  dragByAmount(change: Vector) {
    // Update connected arrows first so they can know where this node started its movement from
    for(var i = 0; i < this.incomingArrows.length; i++) {
      this.incomingArrows[i].targetHasMovedByAmount(change);
    }
    for(var i = 0; i < this.outgoingArrows.length; i++) {
      this.outgoingArrows[i].sourceHasMovedByAmount(change);
    }
    // Update own position last
    this.position.x += change.x;
    this.position.y += change.y;
    this.makeSureGraphicPositionMatchesLogicalPosition();
  }
  makeSureGraphicPositionMatchesLogicalPosition() {
    this.container.style.transform = "translate3d(" + this.position.x + "px, " + this.position.y + "px, 0)";
  }
  addOutgoingArrow(arrow: Arrow) {
    this.outgoingArrows.push(arrow);
  }
  addIncomingArrow(arrow: Arrow) {
    this.incomingArrows.push(arrow);
  }
  removeOutgoingArrow(arrow: Arrow) {
    removeFromArray(arrow, this.outgoingArrows);
  }
  removeIncomingArrow(arrow: Arrow) {
    removeFromArray(arrow, this.incomingArrows);
  }
  radius(): number {
    return Math.sqrt(
      Math.pow(this.container.offsetWidth, 2) +
      Math.pow(this.container.offsetHeight, 2)
    ) / 2;
  }
  center(): Vector {
    return {
      x: this.position.x + this.container.offsetWidth / 2,
      y: this.position.y + this.container.offsetHeight / 2,
    };
  }
}

function isTextNode(node: GraphNode): node is TextNode {
  return (node as TextNode).isTextNodeInstance != null;
}
function isLabelNode(node: GraphNode): node is LabelNode {
  return (node as LabelNode).isLabelNodeInstance != null;
}

class TextNode extends GraphNode {
  private ASSUMED_AREA_OF_CHARACTER = 100;
  private MIN_WIDTH = 80;
  isTextNodeInstance = true;
  constructor(container: HTMLElement, eventHandler: (node: GraphNode, e: MouseEvent) => void, position: Vector) {
    super(container, eventHandler, position);
    container.style.padding = "5px";
    container.style.border = "3px solid black";
    container.style.borderRadius = "6px";
    container.style.width = "100px"; // Start out wider
    container.style.textAlign = "center";
    container.style.overflowWrap = "break-word";
    container.style.minHeight = "20px";
    container.style.backgroundColor = "white";
  }
  startInteraction() {
    // Turn on active flag
    this.isBeingInteractedWith = true;
    // Turn on editing
    this.container.contentEditable = "true";
    // Keep the width constant until interaction is over
    this.container.style.width = this.container.clientWidth - 9.8 + "px";
    this.container.style.padding = "4.9px 5px";
    this.container.focus();
    // TODO: automatically select contents
  }
  stopInteraction() {
    // Turn off active flag
    this.isBeingInteractedWith = false;
    // Turn off editing
    this.container.contentEditable = "false";
    // Reset the width and padding
    const assumedAreaOfText = this.container.innerHTML.length * this.ASSUMED_AREA_OF_CHARACTER;
    const desiredWidth = Math.pow(assumedAreaOfText, .5);
    this.container.style.width = (desiredWidth > this.MIN_WIDTH ? desiredWidth : this.MIN_WIDTH) + "px";
    this.container.style.padding = "5px";
  }
  collidesWithPoint(point: Vector, margin: number): boolean {
    if(point.x < this.position.x - margin) return false;
    if(point.x > this.position.x + this.container.offsetWidth + margin) return false;
    if(point.y < this.position.y - margin) return false;
    if(point.y > this.position.y + this.container.offsetHeight + margin) return false;
    return true;
  }
  clearContents(): void {
    this.container.innerHTML = "";
  }
}

class LabelNode extends GraphNode {
  private ASSUMED_AREA_OF_CHARACTER = 100;
  isLabelNodeInstance = true;
  constructor(container: HTMLElement, eventHandler: (node: GraphNode, e: MouseEvent) => void, position: Vector) {
    super(container, eventHandler, position);
    container.style.textAlign = "center";
    container.style.font = "30px Sans-serif";
    container.style.backgroundColor = "white";
  }
  startInteraction() {
    // Turn on active flag
    this.isBeingInteractedWith = true;
    // Turn on editing
    this.container.contentEditable = "true";
    this.container.focus();
  }
  stopInteraction() {
    // Turn off active flag
    this.isBeingInteractedWith = false;
    // Turn off editing
    this.container.contentEditable = "false";
  }
  collidesWithPoint(point: Vector, margin: number): boolean {
    if(point.x < this.position.x - margin) return false;
    if(point.x > this.position.x + this.container.offsetWidth + margin) return false;
    if(point.y < this.position.y - margin) return false;
    if(point.y > this.position.y + this.container.offsetHeight + margin) return false;
    return true;
  }
  clearContents(): void {
    this.container.innerHTML = "";
  }
  isEmpty(): boolean {
    return this.container.innerHTML == "";
  }
}
