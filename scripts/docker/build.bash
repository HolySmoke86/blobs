#!/bin/bash

# execute from project root

TGT_REPO="${TGT_REPO:-localhorsttv}"

docker build "scripts/docker/archlinux-run-latest/" -t "${TGT_REPO}/archlinux-run:latest" --no-cache
docker build "scripts/docker/archlinux-build-latest/" -t "${TGT_REPO}/archlinux-build:latest" --no-cache
docker build "scripts/docker/centos-run-latest/" -t "${TGT_REPO}/centos-run:latest" --no-cache
docker build "scripts/docker/centos-build-latest/" -t "${TGT_REPO}/centos-build:latest" --no-cache
docker build "scripts/docker/debian-run-latest/" -t "${TGT_REPO}/debian-run:latest" --no-cache
docker build "scripts/docker/debian-build-latest/" -t "${TGT_REPO}/debian-build:latest" --no-cache
docker build "scripts/docker/debian-run-testing/" -t "${TGT_REPO}/debian-run:testing" --no-cache
docker build "scripts/docker/debian-build-testing/" -t "${TGT_REPO}/debian-build:testing" --no-cache
docker build "scripts/docker/ubuntu-run-latest/" -t "${TGT_REPO}/ubuntu-run:latest" --no-cache
docker build "scripts/docker/ubuntu-build-latest/" -t "${TGT_REPO}/ubuntu-build:latest" --no-cache
docker build "scripts/docker/ubuntu-run-devel/" -t "${TGT_REPO}/ubuntu-run:devel" --no-cache
docker build "scripts/docker/ubuntu-build-devel/" -t "${TGT_REPO}/ubuntu-build:devel" --no-cache

docker push "${TGT_REPO}/archlinux-run:latest"
docker push "${TGT_REPO}/archlinux-build:latest"
docker push "${TGT_REPO}/centos-run:latest"
docker push "${TGT_REPO}/centos-build:latest"
docker push "${TGT_REPO}/debian-run:latest"
docker push "${TGT_REPO}/debian-build:latest"
docker push "${TGT_REPO}/debian-run:testing"
docker push "${TGT_REPO}/debian-build:testing"
docker push "${TGT_REPO}/ubuntu-run:latest"
docker push "${TGT_REPO}/ubuntu-build:latest"
docker push "${TGT_REPO}/ubuntu-run:devel"
docker push "${TGT_REPO}/ubuntu-build:devel"
