
# Fathom

## Next Up

[x] Figure out SDL2
[ ] Start SDL2 project
[ ] Make editable text-box class
[ ] Start making framework
[ ] Continue researching [word-wrap](https://www.studyplan.dev/sdl2-minesweeper/sdl2-text)

## Ideas

- objects being represented by nodes are given access to each other, and can view their type information, the source node gets to see the other first before deciding what interface to expose
- connection labels are not visible to nodes, they're only for the users (like position)
- if nodes reject a connection, they provide a short reason why, as well as a more verbose message
- a label node clusters nodes together and draws a bubble around them
- hovering over a node should also show info button with message and verbose message
- right clicking connection opens something specific or it just opens editor menu
- right clicking a node opens transformation menu
- left clicking on chart area does nothing 
- right clicking on chart area opens new node ui
- typing on chart inserts new text node at mouse
- pasting onto chart queries viewer for suitable node, places at center
- dropping on chart queries viewer for suitable node placed on drop location
- use one universal new node / transformation menu
- turn on and off physics for the graph
- sandbox js https://codereview.stackexchange.com/questions/49253/sandbox-or-safely-execute-eval
- use code from test.html in drive
- find a way to use graph specific settings to implement original vision of having multiple viewers for specific purposes
- run visual tests on curve collision
- access other files https://github.com/tauri-apps/tauri/issues/5190
- consider the security of including others' nodes
- dragging mouse to edge should pan
- copy and paste selection
- capture some node events with true, and find a way to securely pass off events to nodes without ruining their ability to have control components such as checkboxes
- make nodes such as checkboxes and buttons, or make a form node with text input and object output
- access settings from hamburger menu
- install new transformations by installing a node that does the transformation, then replaces itself
- installing new node automatically connects it to other nodes using all possible transformations
- consider making some standard style parameters that all nodes can go off of
- nodes can trigger a redraw
- text node listens to changes in its own size, then triggers redraw
- area of character instead of height and width
- when clicking on a node, check the mouse button
- press tab while interacting with a node or arrow to change its style
- let nodes draw on canvas
- use div for selection box
- allow pan farther
- put an x button in top left corner for closing current graph, and maybe a save button too
- find a way to quickly start editing a new graph almost like having another tab, or just implement tabs
- use an off-white theme
- splitting a quadratic in two https://stackoverflow.com/questions/37082744/split-one-quadratic-bezier-curve-into-two
- shrink text node width to fit content
- use two halves of grid to track node distance matrix
- clusters can be connected via arrows as if they were nodes themselves
- change cursor as it hovers over things
- use tabs in text node to make tables
- clustering could take size of nodes into account, but use radius
- cluster labels should stay within view as long as one of the cluster members are within view
- text node types: text, doc, script
- script node has collapse button, status indicator, and play/stop button at the top
- tool node: contains manifest.yml, internal script, config.txt, and executable
- tool nodes can return an image
- file node types: fathom view, image, tool, audio
- make a tool that strips extra information from a fathom view

