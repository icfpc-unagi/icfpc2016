test:
	bazel test -c opt --test_output=errors //...
.PHONY: test

build:
	bazel build -c opt //...
.PHONY: build

upload:
	bazel run -c opt //package:upload
.PHONY: upload
