licenses(["notice"])

package(default_visibility = ["//visibility:private"])

load("//rules:deploy.bzl", "deploy")

deploy(
    name = "score",
    binary = "//iwiwi:score",
)

deploy(
    name = "chokudai",
    binary = "//ai/chokudai:chokudai.ar",
)

deploy(
    name = "wata",
    binary = "//ai/wata:wata.ar",
)

deploy(
    name = "wata_strong",
    binary = "//ai/wata:wata_strong.ar",
)

deploy(
    name = "wata_conflict",
    binary = "//ai/wata:wata_conflict.ar",
)
