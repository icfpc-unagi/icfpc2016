# Team Unagi's Repository for ICFPC 2016

## Remarkable Solvers

### wata/ConflictSolver.java

NOTE: Input must be preprocessed by iwiwi/prefilter, and output must be postprocessed by iwiwi/postfilter1 and iwiwi/postfilter2.

### chokduai/CodeFile1.cs

It finds a convex hull.

## Configuration for CI Services

### Codeship

Set up commands:

```sh
jdk_switcher home oraclejdk8
jdk_switcher use oraclejdk8
if [ ! -x ~/bazel-0.3.0/output/bazel ]; then rm -rf bazel-0.3.0 || true; fi && if [ ! -d ~/bazel-0.3.0 ]; then pushd ~ && git clone --depth=1 -b 0.3.0 https://github.com/bazelbuild/bazel.git bazel-0.3.0 && cd bazel-0.3.0 && EXTRA_BAZEL_ARGS='' ./compile.sh && popd; fi
echo 'startup --batch' > ~/.bazelrc && echo 'build --spawn_strategy=standalone --genrule_strategy=standalone --package_path %workspace%:/home/ubuntu/bazel-0.3.0/base_workspace' >> ~/.bazelrc
pushd ~ && mkdir -p ~/bin && ln -s ~/bazel-0.3.0/output/bazel ~/bin/bazel && popd
timeout 600 make prebuild
```

Test commands:

```sh
timeout 120 make test
```
