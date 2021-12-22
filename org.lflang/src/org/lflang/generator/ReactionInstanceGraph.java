/** A graph that represents causality cycles formed by reaction instances. */

/*************
Copyright (c) 2021, The University of California at Berkeley.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************/

package org.lflang.generator;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.lflang.generator.ReactionInstance.Runtime;
import org.lflang.graph.DirectedGraph;
import org.lflang.lf.Variable;

/**
 * This class analyzes the dependencies between reaction runtime instances.
 * For each ReactionInstance, there may be more than one runtime instance because
 * the ReactionInstance may be nested within one or more banks.
 * In the worst case, of these runtime instances may have distinct dependencies,
 * and hence distinct levels in the graph. Moreover, some of these instances
 * may be involved in cycles while others are not.
 * 
 * Upon construction of this class, the runtime instances are created if necessary,
 * stored each ReactionInstance, and assigned levels (maximum number of
 * upstream reaction instances), deadlines, and single dominating reactions.
 * 
 * After creation, the resulting graph will be empty unless there are causality
 * cycles, in which case, the resulting graph is a graph of runtime reaction
 * instances that form cycles.
 * 
 * @author{Marten Lohstroh <marten@berkeley.edu>}
 * @author{Edward A. Lee <eal@berkeley.edu>}
 */
class ReactionInstanceGraph extends DirectedGraph<ReactionInstance.Runtime> {
    
    /**
     * Create a new graph by traversing the maps in the named instances 
     * embedded in the hierarchy of the program. 
     */
    public ReactionInstanceGraph(ReactorInstance main) {
        this.main = main;
        rebuild();
    }

    ///////////////////////////////////////////////////////////
    //// Public fields

    /**
     * The main reactor instance that this graph is associated with.
     */
    public final ReactorInstance main;
    
    ///////////////////////////////////////////////////////////
    //// Public methods
    
    /**
     * Rebuild this graph by clearing and repeating the traversal that 
     * adds all the nodes and edges.
     */
    public void rebuild() {
        this.clear();
        addNodesAndEdges(main);
        // Assign a level to each reaction. 
        // If there are cycles present in the graph, it will be detected here.
        assignLevels();
        if (nodeCount() != 0) {
            // The graph has cycles.
            main.reporter.reportError("Reactions form a cycle! " + toString());
            throw new InvalidSourceException("Reactions form a cycle!");
        }
    }
    
    ///////////////////////////////////////////////////////////
    //// Protected methods
        
    /**
     * Add to the graph edges between the given reaction and all the reactions
     * that depend on the specified port.
     * @param port The port that the given reaction has as an effect.
     * @param reaction The reaction to relate downstream reactions to.
     */
    protected void addDownstreamReactions(PortInstance port, ReactionInstance reaction) {
        // This is a scary piece of code because of the richness of possible
        // interconnections using banks and multiports. For any program without
        // banks and multiports, each of these for loops iterates exactly once.
        List<Runtime> srcRuntimes = reaction.getRuntimeInstances();
        for (SendRange sendRange : port.eventualDestinations()) {
            int depth = (port.isInput())? 2 : 1;
            for (Range<PortInstance> dstRange : sendRange.destinations) {
                Iterator<Integer> sendParentIDs = sendRange.parentInstances(depth).iterator();
                for (int dstIndex : dstRange.parentInstances(1)) {
                    int srcIndex = sendParentIDs.next();
                    // Destination may be wider than the source (multicast).
                    if (!sendParentIDs.hasNext()) sendParentIDs = sendRange.parentInstances(depth).iterator();
                    for (ReactionInstance dstReaction : dstRange.instance.dependentReactions) {
                        List<Runtime> dstRuntimes = dstReaction.getRuntimeInstances();
                        Runtime srcRuntime = srcRuntimes.get(srcIndex);
                        Runtime dstRuntime = dstRuntimes.get(dstIndex);
                        addEdge(dstRuntime, srcRuntime);
                        
                        // Propagate the deadlines, if any.
                        if (srcRuntime.deadline.compareTo(dstRuntime.deadline) > 0) {
                            srcRuntime.deadline = dstRuntime.deadline;
                        }
                        
                        // If this seems to be a single dominating reaction, set it.
                        // If another upstream reaction shows up, then this will be
                        // reset to null.
                        if (this.getUpstreamAdjacentNodes(dstRuntime).size() == 1) {
                            dstRuntime.dominatingReaction = srcRuntime;
                        } else {
                            dstRuntime.dominatingReaction = null;
                        }
                    }
                }
            } 
        }
    }

    /**
     * Build the graph by adding nodes and edges based on the given reactor 
     * instance.
     * @param reactor The reactor on the basis of which to add nodes and edges.
     */
    protected void addNodesAndEdges(ReactorInstance reactor) {
        ReactionInstance previousReaction = null;
        for (ReactionInstance reaction : reactor.reactions) {
            List<Runtime> runtimes = reaction.getRuntimeInstances();
            
            // Add reactions of this reactor.
            for (Runtime runtime : runtimes) {
                this.addNode(runtime);                
            }
            
            // If this is not an unordered reaction, then create a dependency
            // on any previously defined reaction.
            if (!reaction.isUnordered) {
                // If there is an earlier reaction in this same reactor, then
                // create a link in the reaction graph for all runtime instances.
                if (previousReaction != null) {
                    List<Runtime> previousRuntimes = previousReaction.getRuntimeInstances();
                    int count = 0;
                    for (Runtime runtime : runtimes) {
                        this.addEdge(runtime, previousRuntimes.get(count));
                        count++;
                    }
                }
                previousReaction = reaction;
            }

            // Add downstream reactions. Note that this is sufficient.
            // We don't need to also add upstream reactions because this reaction
            // will be downstream of those upstream reactions.
            for (TriggerInstance<? extends Variable> effect : reaction.effects) {
                if (effect instanceof PortInstance) {
                    addDownstreamReactions((PortInstance)effect, reaction);
                }
            }
        }
        // Recursively add nodes and edges from contained reactors.
        for (ReactorInstance child : reactor.children) {
            addNodesAndEdges(child);
        }
    }
    
    ///////////////////////////////////////////////////////////
    //// Private methods

    /**
     * Analyze the dependencies between reactions and assign each reaction
     * instance a level. This method removes nodes from this graph as it
     * assigns levels. Any remaining nodes are part of causality cycles.
     * 
     * This procedure is based on Kahn's algorithm for topological sorting.
     * Rather than establishing a total order, we establish a partial order.
     * In this order, the level of each reaction is the least upper bound of
     * the levels of the reactions it depends on.
     */
    private void assignLevels() {
        List<ReactionInstance.Runtime> start = new ArrayList<ReactionInstance.Runtime>(rootNodes());
        
        // All root nodes start with level 0.
        for (Runtime origin : start) {
            origin.level = 0;
        }

        // No need to do any of this if there are no root nodes; 
        // the graph must be cyclic.
        while (!start.isEmpty()) {
            Runtime origin = start.remove(0);
            Set<Runtime> toRemove = new LinkedHashSet<Runtime>();
            // Visit effect nodes.
            for (Runtime effect : getDownstreamAdjacentNodes(origin)) {
                // Stage edge between origin and effect for removal.
                toRemove.add(effect);
                
                // Update level of downstream node.
                effect.level = Math.max(effect.level, origin.level+1);   
            }
            // Remove visited edges.
            for (Runtime effect : toRemove) {
                removeEdge(effect, origin);
                // If the effect node has no more incoming edges,
                // then move it in the start set.
                if (getUpstreamAdjacentNodes(effect).size() == 0) {
                    start.add(effect);
                }
            }
            
            // Remove visited origin.
            removeNode(origin);
        }
    }
}
