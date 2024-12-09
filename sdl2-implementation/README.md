
# Fathom - A Map For Your Thoughts

Unsatisfied by all the diagramming/mind-map tools I've been able to find, I decided to go ahead and make my own. Fathom is designed to have the following features:

- Files are stored locally rather than on the cloud
- Interface designed for quick and satisfying editing instead of for feature-rich graphic design
- Easily integrated with other tools
- Fast and lightweight
- Simple installation
- Free

## Local

While it would be nice to access your thinking spaces from any computer, such a convenience comes with its own drawbacks:

- Lack of data ownership
- Difficult to integrate with local tools

## Interface

It's common for diagram editors to present the user with a complex system of menues, sub-menues, and sub-sub-menues of various features and tools for tweaking a graph to look exactly the way the user wants it to look. For someone who's trying to make a professional chart to put on a slideshow or in a book, these features are desirable. The problem with such features is that not everyone needs them. If you're just trying to get thoughts out of your mind and onto a digital surface, you don't need 400 different box shapes to choose from. You don't need 57 fonts, 7 arrow types, 5 different ways of attaching an arrow to a box, and 8 export options! You need a minimalistic interface that has only one goal: help you turn thoughts into a visual graph of interconnected boxes in the fastest and most satisfying way possible.

## Integration With Other Tools

Config files are boring. Apps keep using them because when it comes down to it, it's much easier to read a file than create a whole GUI just for configuration. But what if you could use an interconnected graph as a config file? What if tools were built that accepted a graph or diagram as input? And what if those tools could provide the user feedback through the graph? Fathom enables these kinds of interactions between external tools and user-created graphs by using an easy-to-read JSON format for graph storage. It will be easy for amateur programmers to integrate Fathom graphs into their personal tools and workflows.

To make integration even more seamless, the Fathom file-format will allow external tools to markup the graph with their own notes and warnings for the user. Additional buttons can be added to the UI of a graph that can be bound to system commands.

By embracing simple integration, Fathom makes it easy for people to create and add their own "plugins". While many applications have a plugin system, the learning curve required is too high for many programmers to even bother with it. By making plugins incredibly simple to create and install, Fathom encourages experimentation and creativity.

## Fast and Lightweight

Despite the slowing down of Moore's law, companies continue to make poorly-designed software in hopes that by the time their products hit the market, computers will have sped up enough to make their products seem fast. One major design principle of Fathom is to avoid bloat. Not only is Fathom designed to use the memory and CPU power of your computer efficiently, it avoids dependencies and libraries like the plague. For example, rather than using a clunky GUI library to make Fathom, I spun up my own simple GUI framework that does exactly what Fathom needs and nothing more.

## Simple Installation

I want Fathom to be a long-term solution. Even if I discontinue development or take down the GitHub repo, I want to make sure that people who have made a lot of graphs with Fathom will be able to continue viewing and editing their graphs to their heart's content. To make this possible, Fathom will be packaged with only a few needed files and a single executable for each operating system. It will be easy to install, and will be simple enough that it can still be used without being installed or will at least be able to be installed manually if needed. This should prevent it from becoming unusable if I can't maintain an installer for your particular operating system for whatever reason.

## Free

In gratitude of all the free software I've used throughout my life, I consider this my contribution back to the free software community. Not only do I want to contribute, but this tool simply can't be what I've envisioned it to be without being totally free to use.

