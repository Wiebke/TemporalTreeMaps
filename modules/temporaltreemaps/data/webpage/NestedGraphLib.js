'use strict'; /* globals $ d3 Viz*/

// =============================================================================
// Version 1.2
// =============================================================================
// Usage:
//      1. Represent Graph G = (N, ET, EN) as JSON Object
//         (for details about the definition see the paper)
//      2. call method 'drawNestedGraph' with the graph G and a containerID
//         where to draw the graph

// Change Log
//      - Better initial positioning of the graph
//      - Adaptive to Resize

var NestedGraphLib = {

    // Some color maps from ColorBrewer (http://colorbrewer2.org/)
    colorMaps: {
        '3-class blue':['#3182bd', '#6baed6', '#9ecae1'],
        '6-class YlOrRd':['#bd0026','#f03b20','#fd8d3c','#feb24c','#fed976','#ffffb2'],
        '6-class YlGnBu':['#253494','#2c7fb8','#41b6c4','#7fcdbb','#c7e9b4','#ffffcc']
    },

    currentColorMap: '3-class blue',
    wScale: 0.25, // component width scale
    xScale: 2,    // x-axis scale
    yScale: 1,    // y-axis scale

    // Computes for G the size of nodes based on the number of its children
    computeNodeSize: function(G){
        // Get sorted list of all levels
        var levels = [];
        var level;
        for(level in G.ET)
            levels.push( parseFloat(level) );
        levels = levels.sort();
        var maxLevel = levels.pop();

        // Set width of Nodes with max Level to 1
        var nodes = G.N;
        var node;
        var nodeLabel;
        for(nodeLabel in nodes){
            node = nodes[nodeLabel];
            if(node.l === maxLevel)
                node.w = 1;
        }

        // Compute for remaining Nodes Size based on children
        // Iterate over nodes in decreasing level order
        var edges;
        var i,j;
        for(i=levels.length-1; i>=0; i--){
            level = levels[i];
            for(nodeLabel in nodes){
                node = nodes[nodeLabel];
                if(node.l === level){
                    edges = G.EN[ node.t ][ nodeLabel ] || [];
                    var width = 1;
                    for(j=0; j<edges.length; j++)
                        width+= nodes[edges[j]].w; // We use the fact that we already computed the width of the children
                    node.w = width;
                }
            }
        }
    },

    // Convert G to the dot-format
    convertToDot: function(G){
        var i,j, nodeLabel, time, level;
        var nodes = G.N;
        var trackingGraphs = G.ET;
        var nestingTrees = G.EN;

        /* DotFormat:
            diGraph g {rankdir=LR;
                + nodeString
                + rankString
                + edgeString
            }
        */
        var rankString = '';
        var nodeString = '';
        var edgeString = '';

        // Create a Rank for each Timestep
        var ranks = {};
        for(time in nestingTrees){
            ranks[time] = '{rank=same';
        }

        var minW = Infinity;
        var maxW = -Infinity;
        for(nodeLabel in nodes){
            var node = nodes[nodeLabel];
            nodeString += nodeLabel + '[shape=box,fixedsize=true,height='+node.w+'];\n';
            minW = Math.min(minW, node.w);
            maxW = Math.max(maxW, node.w);
            ranks[ node.t ] += ' '+ nodeLabel;
        }
        var minMaxRange = maxW-minW;

        for(time in nestingTrees){
            rankString += ranks[time] + '}\n';
        }

        for(level in trackingGraphs){
            var trackingGraph = trackingGraphs[level];

            // Add Edges of the Tracking Graph
            for(var startNode in trackingGraph){
                var endNodes = trackingGraph[startNode];
                for(i=0,j=endNodes.length; i<j; i++){
                    // The weight enforces that nodes of roughly the same size are on straight lines
                    var weight = nodes[startNode].w/nodes[endNodes[i]].w;
                    weight = weight<0.5 || weight>1.5 ? 0 : 1;
                    edgeString+=startNode+'->'+endNodes[i]+' [weight='+weight+'];\n';
                }
            }
        }

        return 'diGraph g {rankdir=LR;\n' + nodeString + '\n' + rankString + '\n' + edgeString + '\n}';
    },

    // Computes the initial layout for each edge set ET in G with GraphViz
    computeInitialLayout: function(G){
        var dotFormat = this.convertToDot(G);
        var nodes = G.N;
        var dotLayout = Viz(dotFormat, { format: 'plain' }).split('\n');
        for(var i in dotLayout){
            var line = dotLayout[i].split(' ');
            if(line[0] === 'node')
                nodes[line[1]].layout = {
                    x: nodes[line[1]].t,
                    y: parseFloat(line[3]),
                    w: parseFloat(line[5])*this.wScale
                };
        }
    },

    // Computes for each node the slot positions for its children
    computeSlots: function(G){
        // Get sorted list of all levels
        var levels = [];
        var level;
        for(level in G.ET)
            levels.push( parseFloat(level) );
        levels = levels.sort();

        var nodes = G.N;

        // Function to sort nodes according to their initial layout
        var sortFunction = function(a,b){
            return nodes[a].layout.y - nodes[b].layout.y;
        };

        // Skip last Level
        for(var i=0; i<levels.length-1; i++){
            level = levels[i];
            // For each node...
            for(var nodeLabel in nodes){
                var node = nodes[nodeLabel];
                // with the correct level...
                if(node.l===level){
                    var children = G.EN[node.t][nodeLabel] || [];
                    var nChildren = children.length;
                    var j;
                    // check the number of children:
                    if(nChildren<1) // if it has no children skip
                        continue;
                    else if(nChildren===1) // if it has one child set to same position
                        nodes[children[0]].layout.y = node.layout.y;
                    else { // otherwise sort children according to their layout...
                        children = children.sort(sortFunction);

                        // then compute their total width
                        var wChildren = 0;
                        for(j in children)
                            wChildren += nodes[children[j]].layout.w;

                        // determine the available space of the parent
                        var sizeDiff = node.layout.w - wChildren;
                        var gapWidth = sizeDiff/(nChildren+1);

                        // determine the position of the children from top to bottom
                        var y = gapWidth + node.layout.y - node.layout.w;
                        for(j in children){
                            y += gapWidth + nodes[children[j]].layout.w;
                            nodes[children[j]].layout.y = y;
                            y += gapWidth + nodes[children[j]].layout.w;
                        }
                    }
                }
            }
        }
    },

    // Draws G via D3 by first computing the edge width (if necessary), then compute the initial layout, compute the slots, and finally draw each layer from bottom to top.
    drawNestedGraph: function(G, containerID){
        var i;
        // If first node does not have width property compute topological size
        for(i in G.N){
            if(!G.N[i].hasOwnProperty('w'))
                this.computeNodeSize(G);
            break;
        }

        // Compute initial layout
        this.computeInitialLayout(G);

        var container = document.getElementById(containerID);

        // Init SVG
        container.innerHTML = '';
        var svg = d3.select('#'+containerID).append('svg')
            .attr('class', 'ntg');

        const resize = e => {
            var containerSize = container.getBoundingClientRect();
            svg
                .attr('width', containerSize.width)
                .attr('height', containerSize.height);
        };
        resize();

        window.addEventListener('resize', resize);

        var root = svg.append('g');

        var zoom = d3.zoom()
            .scaleExtent([0.02, 10])
            .on('zoom', function () {
                root.attr('transform', d3.event.transform);
            });
        svg.call(zoom);

        // Compute Slots
        this.computeSlots(G);

        // Get sorted list of all levels
        var levels = [];
        for(var level in G.ET)
            levels.push( parseFloat(level) );
        levels = levels.sort();

        // Draw each layer bottom to top
        var colors = this.colorMaps[this.currentColorMap];
        var nColors = colors.length;
        for(i=0; i<levels.length; i++)
            this.drawLayer(G, G.ET[levels[i]], colors[i%nColors], root);

        // Set Initial Zoom
        var bb = root.node().getBBox();

        var containerSize = container.getBoundingClientRect();

        svg.call(zoom.transform, d3.zoomIdentity.translate(
            -0.5*(bb.x+bb.width/2-containerSize.width),
            -0.5*(bb.y+bb.height/4-containerSize.height)
        ).scale(0.5));
    },

    // Draws an edge from n1 to n2
    drawEdge: function(n1, n2, color, parent){
        // First Node
        var l1 = n1.layout;
        var p1w = l1.w*100*this.yScale;
        var p1x = l1.x*100*this.xScale;
        var p1y = l1.y*100*this.yScale;

        // Second Node
        var l2 = n2.layout;
        var p2w = l2.w*100*this.yScale;
        var p2x = l2.x*100*this.xScale;
        var p2y = l2.y*100*this.yScale;

        // Bezier control point location between n1 and n2
        var delta = 0.6;

        parent.append('path')
            .attr('fill', color)
            .attr('d',
                'M'+p1x+','+(p1y+p1w) // Move to n1 where width is added to y
                +'C'+(p1x+delta*(p2x-p1x))+','+(p1y+p1w)+','+(p1x+delta*(p2x-p1x))+','+(p2y+p2w)+','+p2x+','+(p2y+p2w) // Draw Bezier curve to n2
                +'L'+p2x+','+(p2y-p2w) // Draw line to n2 where width is substracted from n2
                +'C'+(p1x+delta*(p2x-p1x))+','+(p2y-p2w)+','+(p1x+delta*(p2x-p1x))+','+(p1y-p1w)+','+p1x+','+(p1y-p1w) // Draw Bezier curve back to n1
            );
    },

    // Draws one layer of G
    drawLayer: function(G, trackingGraph, color, parent){
        var nodes = G.N;

        for(var startNode in trackingGraph){
            var endNodes = trackingGraph[startNode];
            for(var i=0; i<endNodes.length; i++)
                this.drawEdge(nodes[startNode], nodes[endNodes[i]], color, parent);
        }
    }
};