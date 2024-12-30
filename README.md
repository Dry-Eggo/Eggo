Eggo is envisioned to be a similar language to C/C++, if i have the capabiities too carry that out.

The bin/ directory might be a bit messy tho.

Any suggestions will be appreciated(except for performance-related ones).
Inspiration from  	www.youtube.com/@pixeled-yt

Eggo's Intermediate Repreentation is x86-64 linux assembly.

------Features------

Eggo utilizes a mnemonic-style syntax to make development smoother and feel alot like Vim or Emac
Version 0.1
status -- [Unfinished]

1] supports variable definition with the "mk" keyword, short for "make". e.g mk foo... followed by a type.
   Eggo supports two main type ["int", "str"] and a "still in development" one which are booleans. Types of variables 
   must be specified and is not inferred by the compiler.

   mk age : int = 30;
   
2] supports small functions as it is lacking in many features now. Functions are defined with the "mkf" keyword, short for
   "make function". Functions have return types which musn't be specified. default is null. argument types must be specified.

    mkf foo(msg : str) : str
    {
        mk ret_value : str = "Hello From Eggo";
        #! this is a comment
        ret ret_value;
    };

    type Checking isnt fully implemented yet...

 3] Eggo requires an entry point named "main".
 
    mkf main() : int
    {
        ret 0;
    };

    
