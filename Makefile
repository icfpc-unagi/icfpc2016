test:
	bazel test -c opt --test_output=all //...
.PHONY: test

build:
	bazel build -c opt //...
.PHONY: build

upload:
	bazel run -c opt //package:upload
.PHONY: upload
