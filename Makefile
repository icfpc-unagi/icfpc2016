test:
	bazel test -c opt --test_output=all //...
.PHONY: test

build: refresh
	bazel build -c opt //...
.PHONY: build

upload:
	bash package/upload.sh
.PHONY: upload

refresh:
	for path in bazel-bin/*; do \
		if [ "$${path}" != 'bazel-bin/external' ]; then \
			rm -rf "$${path}"; \
		fi; \
	done
.PHONY: refresh
