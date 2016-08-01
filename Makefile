test:
	bazel test -c opt --test_output=errors //...
.PHONY: test

prebuild:
	bazel build -c opt \
	    //external:base \
	    //external:protoc \
	    //external:protolib \
	    //external:testing \
	    //proto/...

build:
	bazel build -c opt //...
.PHONY: build

upload:
	bazel run -c opt //package:upload
.PHONY: upload
