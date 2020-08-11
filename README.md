
AtomSpace Agents
----------------
AtomSpace Agents implement different policies for moving Atoms between
disk and RAM, and between different servers on the network. They are
the vital decision-makers that are used to stitch together modular
building blocks to build distributed and/or decentralized AtomSpaces.

The Agents are distinct from the disk, RAM and network modules, which
are "building blocks" that the agents can "glue together".

Existing building blocks:
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

Let me start small. So, right now, you can take the cogserver, start it
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
one gets a truly functional small distributed system.

If things don't fit on one disk, or if there are hundreds of clients
instead of dozens, then one needs a "sharing agent" to implement some
policy for sharing portions of an atomspace across multiple cogservers.
Again, this is an orthogonal block of code, and one can imagine having
different kinds of agents for implementing different kinds of policies
for doing this.

An Example Agent
----------------
Consider the simplest agent - the "remembering agent" that sometimes
moves atoms from RAM to disk, and then frees up RAM.  How should that
work? Well, it's surprisingly ... tricky.  One could stick a timestamp
on each Atom (a timestamp Value) and store the oldest ones. But this
eats urp RAM, to store the timestamp, and then eats more RAM to keep
a sorted list of the oldest ones. If we don't keep a sorted list,
then we have to search the atomspace for old ones, and that eats CPU.
Yuck and yuck. 

Every time a client asks for an Atom, we have to update the timestamp
(like the access timestamp on a unix file.)  So, unix files have three
timestamps to aid in decision-making - created, modified and accessed.
It works because most files are huge compared to the size of the
timestamps. For the AtomSpace, the Atoms are the same size as the
timestamps, so we have to be careful not to mandate that every Atom
must have some meta-data.

There's another problem. Suppose some client asks for the incoming set
of some Atom. Well, is that in RAM already, or is it on disk, and needs
to be fetched? The worst-case scenario is to assume it's not, and always
re-fetch from disk. But this hurts performance. (what's the point of RAM,
if we're always going to disk???) Can one be more clever? How?

There's a third problem: "vital" data vs "re-creatable" data. For example,
a genomics dataset itself is "vital" in that if you erase anything, it's a
permanent "dataloss".  As it is being used, the
[MOZI genomics codebase](https://github/mozi-ai) performs searches, and
places the search results into the atomspace, as a cache, to avoid
re-searching next time. These search results are "re-creatable".   Should
re-creatable data be saved to disk? Sometimes? Always? Never? If one has
a dozen Values attached to some Atom, how can you tell which of these
Values are "vital", and which are "recreatable"?

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
