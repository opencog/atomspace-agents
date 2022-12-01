Obsolete
--------
A version of proxy agents has been implemented, and can be found
as a part of the [AtomSpace](https://github.com/opencog/atomspace),
in the [opencog/persist/proxy diectory.](https://github.com/opencog/atomspace/tree/master/opencog/persist/proxy)
A working example can be found in 
[examples/atomspace/persist-proxy.scm](https://github.com/opencog/atomspace/tree/master/examples/atomspace)
What was actually implemented resembles the ideas described below.

There is no code in this repo, there never was any code; it was all
a sketch for a general design. Since it is now possible to write basic
agents in pure Atomese, and fancier Agents by following the Proxy
examples, there does not appear to be any reason to maintain the repo
any longer. Thus its marked obsolete.

AtomSpace Proxy Agents
----------------------
AtomSpace Proxy Agents implement different policies for moving Atoms
between disk and RAM, and between different servers on the network.
They are the vital decision-makers that are used to stitch together
modular disk/RAM/network building blocks to build distributed and/or
decentralized AtomSpaces.

### XXX Attention
The text below describes an idea that is already implemented;
its in the CogServer repo. However, it is clear that this core idea
can be leveraged to build a much more powerful and sophisticated
processing mesh, and that mesh needs to be described. This README does
not describe it (yet). So TODO. Do this.

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
    |  CogServer  | <<==== Internet ====>>  | CogStorageNode |
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

The above desribes the simplest non-trivial agent: the write-through
proxy agent. It is currently implemented in the
[CogServer git repo, proxy subdir](https://github.com/opencog/cogserver/tree/master/opencog/cogserver/proxy).
It works, more or less.

The remainder, below, consists of more general ruminations about how
things can and should work.

### Terminology and building blocks
* The [AtomSpace](https://github.com/opencog/atomspace) is an in-RAM
  (hyper-)graph database.
* The [CogServer](https://github.com/opencog/cogserver) is a network
  server for AtomSpaces. It provides a networked command line for
  scheme, python and json, as well as a fast network connection for
  talking to `CogStorageNode`s.
* The [CogStorageNode](https://github.com/opencog/atomspace-cog) is
  an AtomSpace Node that can share Atoms across the network with
  the CogServer.
* The [RocksStorageNode](https://github.com/opencog/atomspace-rocks),
  an AtomSpace Node that can save/restore Atoms to RocksDB.
  ([RocksDB](https://rocksdb.org/) is an  embeddable persistent
  store for fast storage to disks/SSD drives).
* The [PostgresStorageNode](https://github.com/opencog/atomspace/tree/master/opencog/persist/sql),
  an AtomSpace plugin that can save/restore Atoms to PostgreSQL.
  Its old, slow and needs a major overhaul/redesign. But it still works.

A few more building blocks, not yet ready for general use:
* The [Websosckets server](https://github.com/opencog/atomspace-websockets),
  an alternative network server (alpha, not yet usable).
* The [RPC server](https://github.com/habush/atomspace-rpc),
  an alternative network server (alpha, not yet usable).

Both of the above suffer from not using the "standard"
[StorageNode API](https://wiki.opencog.org/w/StorageNode), which means
that custom code needs to be developed before these become usable.

What are Agents?
----------------
... and why are they needed? Explained by example. (The repeats the
example given above).

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

The Write-through Agent
-----------------------
The simplest non-trivial agent is the write-through agent. For every
write request that it receives on the network, it writes exactly the
same data to disk.

This should be easy to implement, and is critically needed, for the
LinkGrammar example, above.

The Read-through Agent
-----------------------
Much like the write-through agent, it passes on reads. This is harder,
though. The naive implementation would be inefficient: if a read request
is made, and the data is already in the AtomSpace, then why go all the
way down to the disk?  If a read request is made, and we already know
that its not on the disk, then why look for it again? To have this work
"elegantly", some kind of caching and time-stamping and expiration
infrastructure is needed.

The Remembering Agent
---------------------
The Remembering Agent combines the two above, and adds some
sophistication to the write cycle: instead of writing every time a
request comes through, it allows changes to accumulate in the AtomSpace,
and only later does a bulk write of all of the accumulated changes.
This is surprisingly difficult.

Here are some thoughts about such a design. One could stick a timestamp
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

The above sketches three different problems that plague any designs
beyond the very simplest ones.  The obvious solutions are not very good,
the good solutions are not obvious.

This Repo
---------
This repo is meant to be a dumping ground for experimental agents that
implement different kinds of policies, as sketched above.

XXX Above text needs to be re-written for the point of view of a
distribued processing netowrk. The basic proxy-agent design has been
implemented, and it works. The next layer of sophistication remains
undescribed.

Design
------
The code that used to be here has moved to the
[CogServer git repo](https://github.com/opencog/cogserver/tree/master/opencog/cogserver/proxy).
Look there for more.

Building
--------
Install the preq's. These are the AtomSpace, and the cogserver.
There's no working code yet. But when there is, you'll do this:
```
mkdir build; cd build
cmake ..
make -j
sudo make install.
```

Status
------
***Version 0.0.1*** A rough sketch of how things might work.
See, however the
[CogServer git repo](https://github.com/opencog/cogserver/tree/master/opencog/cogserver/proxy),
which contains a working Write-Thru agent (and, by the time you read
this, maybe there will be others. They're pretty easy to create.).

------
