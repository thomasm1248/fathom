
abstract class GraphNode {
  container: HTMLElement;
  wasMoved = false;
  outgoingArrows: Arrow[] = [];
  incomingArrows: Arrow[] = [];
  isBeingInteractedWith = false;
  isSelected = false;
  position: Vector;

  constructor(container: HTMLElement, eventHandler: (node: GraphNode, e: MouseEvent | TouchEvent) => void) {
    this.container = container;
    const self = this;
    this.position = {x: 0, y: 0};
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
  abstract collidesWithPoint(point: Vector): boolean;
  dragByAmount(change: Vector) {
    this.position.x += change.x;
    this.position.y += change.y;
    this.makeSureGraphicPositionMatchesLogicalPosition();
    // Update connected arrows as well
    for(var i = 0; i < this.incomingArrows.length; i++) {
      this.incomingArrows[i].endPointHasMovedByAmount(change);
    }
    for(var i = 0; i < this.outgoingArrows.length; i++) {
      this.outgoingArrows[i].endPointHasMovedByAmount(change);
    }
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

class TextNode extends GraphNode { // TODO: when resizing, adjust positions of arrows as well
  private ASSUMED_WIDTH_OF_CHARACTER = 20;
  private ASSUMED_HEIGHT_OF_LINE = 20;
  private MIN_WIDTH = 100;
  constructor(container: HTMLElement, eventHandler: (node: GraphNode, e: MouseEvent) => void) {
    super(container, eventHandler);
    container.style.padding = "10px";
    container.style.border = "3px solid black";
    container.style.borderRadius = "5px";
    container.style.width = this.MIN_WIDTH + "px";
    container.style.textAlign = "center";
    container.style.overflowWrap = "break-word";
    container.style.minHeight = "30px";
    container.style.backgroundColor = "white";
  }
  startInteraction() {
    // Turn on active flag
    this.isBeingInteractedWith = true;
    // Turn on editing
    this.container.contentEditable = "true";
    // Keep the width constant until interaction is over
    this.container.style.width = this.container.clientWidth - 19.8 + "px";
    this.container.style.padding = "9.9px 10px";
    this.container.focus();
    // TODO: automatically select contents
  }
  stopInteraction() {
    // Turn off active flag
    this.isBeingInteractedWith = false;
    // Turn off editing
    this.container.contentEditable = "false";
    // Reset the width and padding
    const assumedWidthOfText = this.container.innerHTML.length * this.ASSUMED_WIDTH_OF_CHARACTER;
    const assumedAreaOfText = assumedWidthOfText * this.ASSUMED_HEIGHT_OF_LINE;
    const desiredWidth = Math.pow(assumedAreaOfText, .5);
    this.container.style.width = (desiredWidth > this.MIN_WIDTH ? desiredWidth : this.MIN_WIDTH) + "px";
    this.container.style.padding = "10px";
  }
  collidesWithPoint(point: Vector): boolean {
    if(point.x < this.position.x) return false;
    if(point.x > this.position.x + this.container.offsetWidth) return false;
    if(point.y < this.position.y) return false;
    if(point.y > this.position.y + this.container.offsetHeight) return false;
    return true;
  }
}
