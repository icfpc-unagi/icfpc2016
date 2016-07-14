test:
	bazel test -c opt --test_output=all //...
.PHONY: test

build:
	bazel build -c opt //...
.PHONY: build

upload:
	bash package/upload.sh
.PHONY: upload
