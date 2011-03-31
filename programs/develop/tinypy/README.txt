64k tinypy
"batteries not (yet) included"
Copyright (c) 2008 Phil Hassey

Check it out:

$ python setup.py linux pygame
$ ./build/tinypy examples/julia.py
$ ./build/tinypy your-program-goes-here.py

Depends on:
- python (only for bootstrapping)
- sdl (for the pygame module)
- gcc

Credits:
    - math module - Rockins Chen <ybc2084@gmail.com>
    - VS support - Krzysztof Kowalczyk
    - bug fixin' - Dean Hall & Allefant

Thanks to allefant and the python community for all the tips and feedback!
Thanks to John M. for a python 2.5 compat. patch.
And to illume and the rest of #ludumdare for morale support.
Also thanks to python.org, lua.org, valgrind.org, nekovm.org, pypy.org
http://javascript.crockford.com/tdop/tdop.html
http://www.memorymanagement.org/articles/recycle.html
http://shed-skin.blogspot.com/

Other "tiny" python implementations:
http://pymite.python-hosting.com/
http://students.ceid.upatras.gr/~sxanth/pyvm/

F.A.Q.s:

Q. If I run boot.py it says you've got like 80k of code!  That's TOTALLY
    not 64k!  I want my money back.
A. Err... that's true.  But 64k sounds *SO* much better than 80k.
    If you *really* want it to be 64k, just run:
    $ python mk64k.py
    This does the following things:
        - changes 4 spaces into tabs and removes blank lines
        - removes comments
        - removes the "namespacing" i.e. "tp_print" becomes "print"

Q. The binary is bigger than 64k.  I hate big binaries.
A. I don't really care, but if you run "upx tinypy" it makes the tinypy
    binary smaller than 64k.

Q. No matter how you spin this, it's just plain NOT 64k.
A. Let the buyer beware?  I dunno, it's close enough.  Let's call it a rounding
    error, shall we?

Q. How come some oddball combinations of variable and named arguments don't work?
A. Ask me some other time.  Short answer: I do it like lua does it.  Only calls
    like this make sense:
        call_with_var_args(a,b,c,*d)
        call_with_named_args(a=b,**c)
    mixes of both just don't work, sorry!

Q. At the end of build.py tinypy doesn't work!
A. This is probably because of my use of -O3 in the final step.  Run the command
    again without -O3.  Some versions of GCC are buggy and don't do well with it.