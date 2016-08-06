def archive(binary, **kargs):
  native.genrule(
      name = binary + "_rule",
      tools = [
        ":" + binary,
        "//bin:imos-package",
        "//bin:imos-variables",
        "//rules:package",
      ],
      outs = [binary + ".ar"],
      cmd = "$(location //rules:package) $(location //bin:imos-package) " +
            "$(location :" + binary + ") $(OUTS)",
      output_to_bindir = 1,
      executable = 1,
      **kargs)
