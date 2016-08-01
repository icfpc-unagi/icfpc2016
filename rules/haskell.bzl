def haskell_binary(name, srcs, **kargs):
  native.genrule(
      name = name + "_rule",
      srcs = srcs,
      outs = [name],
      cmd = "ghc -O -o \"$@\" $(SRCS)",
      output_to_bindir = 1,
      executable = 1,
      **kargs)
