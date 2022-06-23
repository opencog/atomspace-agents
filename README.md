
AtomSpace Agents
----------------
AtomSpace Agents implement different policies for moving Atoms between
disk and RAM, and between different servers on the network. They are
the vital decision-makers that are used to stitch together modular
disk/RAM/network building blocks to build distributed and/or
decentralized AtomSpaces.

### Example
Consider the following usage scenario. It is taken from the
[LinkGrammar AtomSpace dictionary backend](https://github.com/opencog/link-grammar/tree/master/link-grammar/dict-atomese).
Stacked boxes represent shared-libraries, with shared-library calls going
downwards. It illustrates a LinkGrammar parser, using a dictionary located
in the AtomSpace. But, as AtomSpaces start out empty, the data has to come
"from somewhere". In this case, the data comes from another AtomSpace, running
remotely (in the demo, its in a Docker container).  That AtomSpace in turn
loads its data from a
[RocksStorageNode](https://github.com/opencog/opencog-rocks), which uses
[RocksDB](https://rocksdb.org) to work with the local disk drive.
The network connection is provided by a
[CogServer](https://github.com/opencog/cogserver) to
[CogStorageNode](https://github.com/opencog/opencog-cog) pairing.
Note: all this code already works, and is stable (stays up for weeks/months
without crashing or data corruption.)
```
                                            +----------------+
                                            |  Link Grammar  |
                                            |    parser      |
                                            +----------------+
                                            |   AtomSpace    |
    +-------------+                         +----------------+
    |             |                         |                |
    |  Cogserver  | <<==== Internet ====>>  | CogStorageNode |
    |             |                         |                |
    +-------------+                         +----------------+
    |  AtomSpace  |
    +-------------+
    |    Rocks    |
    | StorageNode |
    +-------------+
    |   RocksDB   |
    +-------------+
    | disk drive  |
    +-------------+
```
The above works great, as long as the dataset that's on disk is fully
loaded into the server AtomSpace, before any clients try to use it.
What doesn't work (without an agent) is the case where an update in the
client AtomSpace needs to be pushed to the server, and then pushed to
the disk drive.  Pushing to the CogServer is easy (its a simple existing
function). But how to get it to the disk drive?

There are two ways to implement this idea:
* Top-down command. The client app (link-grammar) tells the CogServer
  what to do, and the CogServer obeys orders, and just does it. There
  are two problems with this design:
  - How to send orders telling the CogServer what to do? There is no
    infrastructure for this.
  - What happens if two different apps are sending conflicting orders?
    How can one avoid them trampling on one-another?

* Policy agents. The client app (link-grammar) attaches to a server that
  implements the desired policy (which, in this case, is a write-through
  from network to disk storage).  This solves both problems above, as
  the policy agent knows what to do, and it knows how to resolve conflicts
  that individual apps might not even be aware of.

This repo implements such policy ageints. The above is the simplest
non-trivial agent: the write-through agent.

### Existing building blocks
* The [AtomSpace](https://github.com/opencog/atomspace), the in-RAM
  database.
* The [CogServer](https://github.com/opencog/cogserver), a network
  server for AtomSpaces.
* The [Cog backend client](https://github.com/opencog/atomspace-cog),
  an AtomSpace plugin that can share Atoms across the network with
  the CogServer.
* The [Rocks backend](https://github.com/opencog/atomspace-rocks),
  an AtomSpace plugin that can save/restore Atoms to RocksDB.
  ([RocksDB](https://rocksdb.org/) is an  embeddable persistent
  store for fast storage to disks/SSD drives).
* The [SQL backend](https://github.com/opencog/atomspace/tree/master/opencog/persist/sql),
  an AtomSpace plugin that can save/restore Atoms to PostgreSQL.

A few more building blocks, not yet ready for general use:
* The [Websosckets server](https://github.com/opencog/atomspace-websockets),
  an alternative network server (alpha, not yet usable).
* The [RPC server](https://github.com/habush/atomspace-rpc),
  an alternative network server (alpha, not yet usable).

What are Agents?
----------------
... and why are they needed? Explained by example.

Let's start small. So, right now, you can take the cogserver, start it
on a large-RAM machine, and have half-a-dozen other AtomSpaces connect
to it. They can request Atoms, do some work, push those Atoms back to
the cogserver. This gives you a distributed AtomSpace, as long as
everything fits in the RAM of the cogserver, and you don't turn the
power off.

The next obvious step is to enable storage-to-disk for the cogserver.
This is where the first design difficulties show up.  If a cogserver
is running out of RAM, it should save some Atoms to disk, and clear
them out of RAM. Which ones?  Answer: write a "remembering agent" that
implements some policy for doing this.  There is no need to modify
the cogserver itself, or any of the clients, to create this agent:
so it's a nice, modular design. That's why the existing pieces are
"building blocks": with this third piece, this "remembering agent",
one gets a truly functional small distributed AtomSpace.

If things don't fit on one disk, or if there are hundreds of clients
instead of dozens, then one needs a "sharing agent" to implement some
policy for sharing portions of an AtomSpace across multiple cogservers.
Again, this is an orthogonal block of code, and one can imagine having
different kinds of agents for implementing different kinds of policies
for doing this.

An Example Agent
----------------
Consider the simplest agent - the "remembering agent" that sometimes
moves atoms from RAM to disk, and then frees up RAM.  How should that
work? Well, it's surprisingly ... tricky.  One could stick a timestamp
on each Atom (a timestamp Value) and store the oldest ones. But this
eats up RAM, to store the timestamp, and then eats more RAM to keep
a sorted list of the oldest ones. If we don't keep a sorted list,
then we have to search the atomspace for old ones, and that eats CPU.
Yuck and yuck.

Every time a client asks for an Atom, we have to update the timestamp
(like the access timestamp on a Unix file.)  So, Unix files have three
timestamps to aid in decision-making - "created", "modified" and "accessed".
This works because most Unix files are huge compared to the size of the
timestamps. For the AtomSpace, the Atoms are the same size as the
timestamps, so we have to be careful not to mandate that every Atom
must have some meta-data.

There's another problem. Suppose some client asks for the incoming set
of some Atom. Well, is that in RAM already, or is it on disk, and needs
to be fetched? The worst-case scenario is to assume it's not in RAM, and
always re-fetch from disk. But this hurts performance. (what's the point
of RAM, if we're always going to disk???) Can one be more clever? How?

There's a third problem: "vital" data vs "re-creatable" data. For example,
a genomics dataset itself is "vital" in that if you erase anything, it's a
permanent "dataloss".  The [MOZI genomics code-base](https://github/mozi-ai),
as it is being used, performs searches, and places the search results into
the AtomSpace, as a cache, to avoid re-searching next time. These search
results are "re-creatable".   Should re-creatable data be saved to disk?
Sometimes? Always? Never? If one has a dozen Values attached to some Atom,
how can you tell which of these Values are "vital", and which are
"re-creatable"?

The above sketches three different problems that even the very simplest
agent must solve to make even the simplest distributed system.   The
obvious solution is not very good, the good solution is not obvious.

This Repo
---------
This repo is meant to be a dumping ground for experimental agents that
implement different kinds of policies, as sketched above.

Status
------
***Version 0.0.0*** There is nothing here yet.
