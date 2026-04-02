[TOC]

# Using and developing with or for the Kariba library

There are three kind of uses of the Kariba library:

* you are creating your own model, using the functionality of the
  library; you don't need extra functionality.

* you are creating your own model, but you need to extend some
  functionality of the library.

* you are improving, extending or fixing bugs in the library


These three scenarios are detailed below

## Creating your own model using the library

In this case, build and install the library first, as described in the
installation section. Note in which directory you installed the
library, or perhaps you left it where it was build. Both options are
fine.

For your model, create directory outside of the Kariba
repository. This should probably become a Git repository of its own,
to manage changes to your own model (including, for example,
experimental versions on Git branches).

So you'd have two or optionally three directories: one with the Kariba
repository, one with your model repository, and an optional one where
you may have the Kariba library installed. For me, that would be
something like

```
$HOME/code/kariba
$HOME/code/my_model
$HOME/sw
```

Once you've written the start of your model, add the necessary Kariba
include files to your sources, e.g.

```
#include <kariba/Powerlaw.hpp>
#include <kariba/Thermal.hpp>
```

Note the use of angular brackets and the kariba/ directory prefix.

When you compile your code, you will need the `-I` option to
point the compiler to the Kariba header files.

If you didn't install Kariba in a separate directory, these should
point to the directory where the Kariba library was compiled and which
has the header files in the 'kariba' subdirectory. Also, find path of
the 'libkariba.a' static library, which is probably in the same
directory. Then, set `-I` to have this directory included, and include
the full path to the 'libkariba.a' library with your compilation. In
the above example, the relevant directory is `$HOME/code/kariba/src`,
so something like the following compilation command works
(`mymodel.cpp` and `utils.cpp` being local project files):

```
g++ -std=c++17 -O3 -Wall -Wextra -I$HOME/code/kariba/src $HOME/code/kariba/src/libkariba.a -lgsl -lm -o mymodel mymodel.cpp utils.cpp
```

Note there is no `-L` flag before the 'libkariba.a' path.

Add an `-fopenmp` flag if needed, and replace `-O3` with e.g. `-Ofast` if wanted (and deemed safe).


If you installed the library (and its header files) in a separate directory, it's very similar. With the above example, it now becomes

```
g++ -std=c++17 -O3 -Wall -Wextra -I$HOME/sw/include $HOME/code/kariba/sw/lib/libkariba.a -lgsl -lm -o mymodel mymodel.cpp utils.cpp
```

The above two variants compile your code with a static variant of the Kariba library. If you'd like, you can use the dynamic library instead. The compilation variants then look as follows (using the `-L` option this time):

```
g++ -std=c++17 -O3 -Wall -Wextra -I$HOME/code/kariba/src -L$HOME/code/kariba/src -lkariba -lgsl -lm -o mymodel mymodel.cpp utils.cpp
```

(not installed; directly from the source repository)

and

```
g++ -std=c++17 -O3 -Wall -Wextra -I$HOME/sw/include -L$HOME/code/kariba/sw/lib/ -lkariba -lgsl -lm -o mymodel mymodel.cpp utils.cpp
```

With this variant, you'll need to set your `LD_LIBRARY_PATH` (on
Linux) or `DYLD_LIBRARY_PATH` (on macOS) environment variables to
point to the Kariba shared library. That would be (bash, zsh):

```
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$HOME/sw/lib
```

for a Linux installed variant, and similar for not-installed versions
and macOS, replacing the path and/or environment variable accordingly.

The static library is easier to use. The advantage of a shared library
can be that, if that library changes (e.g., because there was a bug
fix of a specific function) and the library is updated (you
re-installed the Kariba library), then your model follows that
fix. With a static library, you would have to recompile your model as
well. But it is likely that that is very quick, so you can probably
still use the static library variant without hassle, even in the rare
cases of updates to the Kariba library.



## Creating your own model using the library, extending some parts of the library

Essentially, you work the same way as above: create a repository for
your model separately and independently from the Kariba library, and
compile as suggested above.

For parts where you want to extend functionality, there may be a few ways to go about:

### Extend or change class functionality (e.g., a member function)

If you want to add or change the behaviour of a function of a
particular Kariba class, you can do that fairly easily: use the C++
inheritance abilities to create a new class, inheriting from an
existing Kariba class you'd like to extend, then add the new function,
or override an existing function.

As an example, let's say we want to change the `set_p()` member
function of the `kariba::Thermal` class. The minimum and maximum
energy are not what we'd like (first lines of the original function):

```
void Thermal::set_p() {    //
    double emin =
        (1. / 100.) * Temp;      // minimum energy in kev, 1/100 lower than peak
    double emax = 20. * Temp;    // maximum energy in kev, 20 higher than peak
    double gmin, gmax, pmin, pmax, pinc;
...
```

So create a new header file (or add to an existing one), and include the following:

```
#include <kariba/Thermal>

class Thermal2 : public Thermal {
  public:
    // inherit constructors; necessary if there
	// are member variables to initialize
	// in the constructor
    using Thermal::Thermal;

    void set_p() override;
};
```

and in a source file, define the class and implement the overriddend function:

```
#include "thermal2.hpp"

Thermal2::set_p() {    //
    double emin =
        (1. / 10.) * Temp;      // minimum energy in kev, 1/100 lower than peak
    double emax = 2. * Temp;    // maximum energy in kev, 20 higher than peak
    double gmin, gmax, pmin, pmax, pinc;
...
```

where you will need to write the rest of the function in the place of
the ellipses, of course (so copy-paste the old function, change
`Thermal` to `Thermal2`, make the actual changes for `emin` and `max`,
and you're good).

Note that we don't need to override the constructor, or any of the
other functions. As long as these stay the same, and there are no new
member variables that need to be initialized in the constructor, there
is no need for this.

When inheriting, be aware of the access priviliges:

* all member variables in the base classes are protected. That way,
  they can be accessed directly from a child class.

* inherit with `public`. That way, member variables are still
  protected in derived (child) classes, so that every grandchildren
  can access their grandparents' member variables. While it can be
  argued this is not (always) good, in cases like the above, this is a
  major and important convenience (otherwise, you'd need to add a lot
  of setters and getters to the base class).

* member functions, overridden or newly added, should practically
  always be `public`.

* the `using Thermal::Thermal` tells the compiler that `Thermal2`
  should use the matching constructor from `Thermal` to initialize the
  class and its member variables (matching here means, by arguments;
  if there's only one constructor, which is the case in all Kariba
  classes, this doesn't matter).

Important to note is the use of the `override` keyword at the end of
the definition of the overridden member function. This indicates to
the compiler this function is overridden from the base class. This can
prevent errors: if you accidentally had changed the function signature
(e.g., `void set_p(int)`), you wouldn't override the base class member
function, but you would have overloaded it, and there would suddenly
be two, different, `set_p` functions. With the `override` keyword
after the function declaration, the compiler catches this, as there'll
need to be a base class function with the exact same signature. So
`override` serves as a safety net for the programmer. It is actually
not mandatory to use `override`, and the compiler will happily compile
the code without it, whether the function is overridden or overloaded,
but it won't distinguish between the two variants, and will not warn
the programmer about it.

### Overloading member functions

You can also *overload* functions from a base class, by using the same
function name with a different signature (e.g., different
arguments). This can be convenient if you have a function that
requires some extra arguments.

As a bonus, if you want, you can then call the original (base class)
function from your new function, and wrap some code around it.

In our above example, this would like the following for our derived
class:

```
#include <kariba/Thermal>

class Thermal2 : public Thermal {
  public:
    // inherit constructors; necessary if there
	// are member variables to initialize
	// in the constructor
    using Thermal::Thermal;

    using Thermal::set_p;  // important!
    void set_p(int n);
};
```

and then in the definition of `Thermal2::set_p`:

```
void Thermal2::set_p(int n) {
	// irrelevant loop to demonstrate wrapping the base set_p
	
	for (int i = 0; i < n; ++i) {
	    set_p();  // calls the base function, inferred from the lack of argument
    }
}
```

Note how `Thermal2::set_p` calls `Thermal::set_p`, by virtue of the
two functions having different arguments.

The `using Thermal::set_p` in the declaration of `Thermal2` is very
important; without it, any base member functions called `set_p` are
not recognised by the overriden `set_p`, and `Thermal2` would only
know about its own `set_p`, not about the base `set_p`
function(s). This is not too dissimilar from the `using
Thermal::Thermal` seen earlier for the constructor.


And yes, you can of course overload *and* override functions from the
base class. So if you have `set_p(int)` and `set_p() override` in
`Thermal2`, with the same definition for `set_p(int)` as above, it
would now call the local `Thermal2::set_p(int)` instead.

-----

Logically, perhaps, the lower and upper boundary factors should be
arguments in `set_p()`, perhaps with default settings of 1/100 and 20. 
Which leads to the third variant of working with Kariba: extending
the library, adding improvements or simply fixing bugs



## Improve, extend or fix a bug in the library

In this case, you'll work directly in the Kariba repository. However, do this on a new branch.

These changes should be for long-term changes; if you have something
that is for a specific or temporary case, use the workflow in the
section above.

### Git setup

First, check that you actually have and use a (GitHub) fork of the
library. On GitHub, fork the repository. Then, copy the URL for the
SSH-variant of the cod of your fork.

Now, back in the terminal on your local machine, check the remotes of the Kariba library:

```
git remote -v
```

If you see an 'upstream' and 'origin' remote, check that 'upstream'
URL points to the original repository, not your fork. The 'origin' URL
should point to your fork, with the URL starting with
`git@github.com:` (this indicates SSH is used).

If you only see an 'origin' remote, it likely points to the original
repository, not your fork. Change that:

```
git remote set-url origin git@github.com:<username>/kariba.git
```

(the URL is the one you copied from GitHub; obviously `<username>` is your user name.)

Now add the URL to the original Kariba repository as an upstream:

```
git remote set-url origin https//github.com/kariba/kariba.git
```

Check `git remote -v` again to see that the URLs and names now match as mentioned above.


### Create a branch and add your changes

Create a separate branch, from the main branch, and add your changes there.

When finished, test that the library still builds, and that the unit
tests and examples still build and run. If there was a fix that
changes the outcome of a function, update the unit test accordingly
(just be sure that your change is correct). If you added a function,
add one or a few unit tests that specifically test this function.

### Push the branch and create a pull request

Once everything is commit (you can easily make multipe small commits
if it makes sense), first check that you are still up to date with the
upstream repository. Pull in any changes and rebase upstream/main:

```
git fetch upstream
git rebase upstream/main
```

Then push the branch to your fork:

```
git push origin <mybranch>
```

Now, on GitHub, create a pull request from that branch to the original
repository. There is likely a big green button on the GitHub page:
click it to prepare the pull request.

Fill out the necessary details, check again if everything looks good,
then create the pull request (PR).

You are automatically already on the origina repository site, on the
details for your PR. If everything went correctly, the continuous
integration (CI) tests will be run. Wait to see if these all pass.

Now you'll hae to wait until a maintainer merges your PR. Or they may
have some further questions or suggestions to improve your PR.

If you yourself are the maintainer, I'd suggest to wait a few days
before merging. Sometimes, you later realise you have forgotten
something, and you can then still easily add it to the PR.

Note: please avoid pushing directly to the original repository in you
are a maintainer (or even directly to the main branch): using a
personal fork makes following the flow of updates much easier. The PR,
and changes that came with, is nicely laid out on GitHub (also when
closed), and because merging a PR creates a merge commit, there are
clear points where a consistent set of changes was added to the
library.

### Updating the pull request

Sometimes you want to update a pending pull request: you've forgotten
to commit a file, accidentally commited a file that shouldn't be
there[*], or made a typo while changing things.

For that, go to the correct branch in your local repository, make the
changes as needed, then force-push the branch to your fork:

```
git push origin <mybranch> --force
```

You may not need the `--force` flag, if you only added commits. If you
changed commits (with `--amend`, or an interactive rebase, to keep the
PRs history cleaner), then `--force` is needed.

Once updated, the CI for the PR should run again, so check that your
updates didn't make the tests fail.


-----

[*] If you accidentally pushed a private key, a password, or some
other secret onto GitHub, then changing things and force-pushing the
update won't help. Even if you overwrite the Git history (an
interactive rebase can do that), bots scrape GitHub all the time, and
the secret may already have been comprised. Do get rid of the secret
in the code, but also change the relevant secret as soon as possible.
