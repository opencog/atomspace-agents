
Design Notes
------------
To make agents work, the cogserver needs to intercept sexpr commands,
and do non-default things with them.  There are several things that
needs to happen.

* Change the sexpr `Commands.cc` to provide virtual methods, and
  overload those methods...

* Provide a client with some way of stating what policy should be used.

How can the second thing be done?  There are two choices:

* Create a `WriteThruStorageNode` class, which is identical to the
  `CogStorageClass`, except it causes this policy agent to run.
  Blech.

* Alter `CogStorageNode` to detect URL's of the form
  `wthru://example.com/xxx` and use that to open a Cogserver connection
  to `wthru` instead of `sexpr`.  Alternately, the URL could be
  `cog://example.com?policy=wthru&foo=bar` which is more flexible for
  future expansion.

The second alternative seems better.

### Boilerplate code
The code in these files:
```
WriteThruShell.cc
WriteThruShell.h
WriteThruShellModule.cc
WriteThruShellModule.h
```
is an almost pure cut-n-paste to the sexpr, scheme, python, json
boilerplate, with only a few minor changes.  We need to declare a
generic boilerplate template and get rid of this crapola.

The code in `WriteThruEval.cc` (to be written) is just a cut-n-paste
of generic evaluator code. The only thing that matters is the
overloading of the Commands in Commands.h