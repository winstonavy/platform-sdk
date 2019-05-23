#!/bin/bash

set -ex

yum update

yum install -y \
  build-essential \
  curl \
  git \
  golang \
  cmake \
  ninja-build \
  libssl-dev \
  perl \
  pkg-config \
  qtbase5-dev \
  unzip \
  zlib1g-dev
