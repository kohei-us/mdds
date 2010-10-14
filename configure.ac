AC_INIT(mdds, 0.3.1, kohei.yoshida@gmail.com)

AC_ARG_WITH(hash-container,
[
  --with-hash-container   specify the type of hash containers to use.  
                          Supported types are: boost, c++0x, or stlport.  
                          c++0x is used if unspecified.

                          With c++0x, we assume that unordered_map and 
                          unordered_set are available under the std namespace,
                          whereas with boost they are assumed to be defined in
                          the boost namespace.  When stlport is specified, we
                          assume that hash_map and hash_set are available
                          under the std namespace.

                          Example:  --with-hash-container=boost],
,with_hash_container=c++0x)

AC_OUTPUT(Makefile)
