def deploy(name, binary, **kargs):
  native.genrule(
      name = name + "_rule",
      tools = [binary],
      outs = [name],
      cmd = "cp $(location " + binary + ") $(OUTS)",
      output_to_bindir = 1,
      executable = 1,
      **kargs)
