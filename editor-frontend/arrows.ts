
const SIZE_OF_ARROWHEAD = 5;
const COLLISION_MARGIN = 20;
const COLLISION_INTERVALS = 10;
const COLLISION_PRECISION = 4;
const LABEL_LINE_HEIGHT = 18;
const LABEL_BOX_PADDING = 5;
const NODE_INTERSECTION_MARGIN = 5;
const NODE_INTERSECTION_PRECISION = 0.2;
class Arrow {
  sourceNode: GraphNode;
  targetNode: GraphNode; // TODO: move util functions and types to their own file
  control: Vector;
  mouse: Vector;
  isBeingInteractedWith = false;
  label: string[] | null = null;
  labelIsBeingEdited = false;
  constructor(sourceNode: GraphNode, mouse: Vector) { // TODO: avoid glitchy behavior when nodes overlap
    this.sourceNode = sourceNode;
    this.sourceNode.addOutgoingArrow(this);
    this.mouse = {x: 0, y: 0};
    this.updateEndPointFromMouse(mouse);
  }
  draw(ctx: CanvasRenderingContext2D) {
    // Some calculations
    const start = this.sourceNode.center();
    const end = this.targetNode?.center() ?? this.mouse;
    let intersectionWithTargetNode = end;
    let intersectionT = 1;
    if(this.targetNode !== undefined) {
      let {point, t} = findWhereCurveIntersectsNode(start, this.control, end, this.targetNode);
      intersectionWithTargetNode = point;
      intersectionT = t;
    }
    // Start drawing
    ctx.save();
    // Plot curve
    ctx.save();
    ctx.beginPath();
    ctx.moveTo(start.x, start.y);
    ctx.quadraticCurveTo(this.control.x, this.control.y, end.x, end.y);
    // Use a highlight when being interacted with
    if(this.isBeingInteractedWith) {
      ctx.shadowColor = "blue";
      ctx.shadowBlur = 5;
    }
    // Finish drawing curve
    ctx.strokeStyle = "black";
    ctx.lineWidth = 3;
    ctx.stroke();
    ctx.restore();
    // Prepare to draw arrowhead
    ctx.save();
    let d = vSubtract(
              quadraticInterpolation(start, this.control, end, intersectionT),
              quadraticInterpolation(start, this.control, end, intersectionT - 0.01)
            );
    let direction = Math.atan2(d.y, d.x);
    ctx.translate(intersectionWithTargetNode.x, intersectionWithTargetNode.y);
    ctx.rotate(direction);
    // Draw a circle to cover up part of the curve
    if(this.targetNode != null) {
      ctx.beginPath();
      ctx.arc(0, 0, 3, 0, Math.PI*2);
      ctx.fillStyle = "white";
      ctx.fill();
    }
    // Draw arrowhead
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(-SIZE_OF_ARROWHEAD*2, SIZE_OF_ARROWHEAD);
    ctx.lineTo(-SIZE_OF_ARROWHEAD*2, -SIZE_OF_ARROWHEAD);
    ctx.fillStyle = "black";
    ctx.fill();
    ctx.restore();
    // Draw label
    if(this.label != null) {
      ctx.font = "16px Sans-serif";
      ctx.textAlign = "center";
      ctx.textBaseline = "top";
      // Do some measuring
      let labelWidth: number | null = null;
      for(let i = 0; i < this.label.length; i++) {
        let nextLine = ctx.measureText(this.label[i]);
        if(labelWidth === null || nextLine.width > labelWidth) labelWidth = nextLine.width;
      }
      // Find midpoint of quadratic curve
      let labelPosition = quadraticInterpolation(this.sourceNode.center(), this.control, this.targetNode.center(), 0.5);
      // Move context to label position
      ctx.translate(labelPosition.x, labelPosition.y - LABEL_LINE_HEIGHT * this.label.length / 2);
      // Draw background box
      let boxWidth = labelWidth + LABEL_BOX_PADDING * 2;
      let boxHeight = this.label.length * LABEL_LINE_HEIGHT + LABEL_BOX_PADDING * 2;
      ctx.fillStyle = "white";
      ctx.fillRect(-boxWidth/2, -LABEL_BOX_PADDING, boxWidth, boxHeight);
      // Draw each line
      ctx.fillStyle = "black";
      for(let i = 0; i < this.label.length; i++) {
        ctx.fillText(this.label[i], 0, i * LABEL_LINE_HEIGHT);
      }
    }
    ctx.restore();
  }
  setTargetNode(node: GraphNode) {
    if(this.targetNode != null) {
      // Disconnect TODO
    }
    this.targetNode = node;
    this.targetNode.addIncomingArrow(this);
    // TODO: update control point too to keep line straight
  }
  // Only use during creation
  updateEndPointFromMouse(mouse: Vector) {
    this.mouse.x = mouse.x;
    this.mouse.y = mouse.y;
    let sourceCenter = this.sourceNode.center();
    this.control = {
      x: (sourceCenter.x + mouse.x)/2,
      y: (sourceCenter.y + mouse.y)/2,
    };
  }
  prepareForDeletion() {
    // Disconnect from source node
    if(this.sourceNode != null) {
      this.sourceNode.removeOutgoingArrow(this);
    }
    // Disconnect from target node
    if(this.targetNode != null) {
      this.targetNode.removeIncomingArrow(this);
    }
  }
  endPointHasMovedByAmount(change: Vector) {
    this.control.x += change.x / 2;
    this.control.y += change.y / 2;
  }
  startInteraction() {
    this.isBeingInteractedWith = true;
  }
  stopInteraction() {
    this.isBeingInteractedWith = false;
    this.labelIsBeingEdited = false;
    if(this.label != null && this.label.length === 1 && this.label[0].length === 0) this.label = null;
  }
  checkForCollisionWithPoint(point: Vector): {collision: boolean, t: number | null} {
    let start = this.sourceNode.center();
    let end = this.targetNode.center();
    let control = this.control;
    // Do bounding box first
    let left = Math.min(start.x, control.x, end.x) - COLLISION_MARGIN;
    let right = Math.max(start.x, control.x, end.x) + COLLISION_MARGIN;
    let top = Math.min(start.y, control.y, end.y) - COLLISION_MARGIN;
    let bottom = Math.max(start.y, control.y, end.y) + COLLISION_MARGIN;
    if(point.x < left || point.x > right || point.y < top || point.y > bottom) return {collision: false, t: null};
    // Generate points on the line and find the closest
    let best: number | null = null;
    let bestT: number | null;
    const INCREMENT = 1 / COLLISION_INTERVALS;
    for(let i = INCREMENT; i < 1; i += INCREMENT) {
      let testPoint = quadraticInterpolation(start, control, end, i);
      let distance = vDistance(testPoint, point);
      if(best == null || distance < best) {
        best = distance;
        bestT = i;
      }
    }
    // Do a search around
    let currentT = bestT;
    let interval = INCREMENT / 2;
    for(let i = 0; i < COLLISION_PRECISION; i++) {
      let left = quadraticInterpolation(start, control, end, currentT - interval);
      let right = quadraticInterpolation(start, control, end, currentT + interval);
      if(vDistance(left, point) < vDistance(right, point)) {
        currentT -= interval;
      } else {
        currentT += interval;
      }
      interval /= 2;
    }
    // Compare to end points
    let currentBestT = currentT;
    if(vDistance(start, point) < vDistance(quadraticInterpolation(start, control, end, currentBestT), point)) {
      currentBestT = 0;
    }
    if(vDistance(end, point) < vDistance(quadraticInterpolation(start, control, end, currentBestT), point)) {
      currentBestT = 1;
    }
    // Make sure final point is within margin
    return {collision: vDistance(quadraticInterpolation(start, control, end, currentBestT), point) <= COLLISION_MARGIN, t: currentBestT};
  }
  onKeyPressed(e: KeyboardEvent): boolean /* is a redraw requested */ {
    // Check if the user wants to start editing the label
    if(!this.labelIsBeingEdited) {
      if(e.key.length === 1) {
        this.labelIsBeingEdited = true;
        this.label = [""];
      } else {
        return false;
      }
    }
    // Leave if the user is not editing the label
    if(!this.labelIsBeingEdited) return;
    // If user enters text, add it to the label
    if(e.key.length === 1) {
      this.label[this.label.length-1] += e.key;
      return true;
    }
    // If user presses enter, add a new line
    if(e.key === "Enter") {
      this.label.push("");
      return true;
    }
    // If user presses backspace, undo the previous action
    if(e.key === "Backspace") {
      if(this.label[this.label.length-1].length > 0) {
        // Remove most recently typed character
        let lastLine = this.label.pop();
        this.label.push(lastLine.substring(0, lastLine.length-1));
        return true;
      } else {
        // Remove last line
        this.label.pop();
        if(this.label.length === 0) this.label.push("");
        return true;
      }
    }
  }
  bendArrow(change: Vector, t: number) {
    let weight = 1 / (2 * t * (1 - t));
    change = vScale(change, weight);
    this.control = vAdd(this.control, change);
  }
}

function quadraticInterpolation(start: Vector, control: Vector, end: Vector, t: number) {
  let a = vSubtract(control, start);
  let b = vSubtract(end, control);
  a = vAdd(start, vScale(a, t));
  b = vAdd(control, vScale(b, t));
  let c = vSubtract(b, a);
  c = vAdd(a, vScale(c, t));
  return c;
}
function findWhereCurveIntersectsNode(start: Vector, control: Vector, end: Vector, node: GraphNode): {point: Vector, t: number} {
  const toleranceSquared = NODE_INTERSECTION_PRECISION * NODE_INTERSECTION_PRECISION;
  let currentT = 0.5; // half
  let searchIncrement = 0.25; // quarter
  let testPoint: Vector = null;
  let prevTestPoint: Vector = null;
  let sizeOfStepTaken: number = null;
  while((sizeOfStepTaken ?? 100/*big number*/) > toleranceSquared) {
    testPoint = quadraticInterpolation(start, control, end, currentT);
    if(node.collidesWithPoint(testPoint)) {
      currentT -= searchIncrement;
    } else {
      currentT += searchIncrement;
    }
    searchIncrement /= 2;
    if(prevTestPoint != null) {
      sizeOfStepTaken = vDistanceSquared(testPoint, prevTestPoint);
    }
    prevTestPoint = testPoint;
  }
  return {point: testPoint, t: currentT};
}
