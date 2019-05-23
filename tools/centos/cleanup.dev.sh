#!/bin/bash
set -ex

yum update

# Let's remove our build dependencies
# and be quite aggressive about it.
yum remove -y \
  build-essential \
  cmake \
  git \
  ninja-build \
  libssl-dev \
  pkg-config \
  qtbase5-dev

yum autoremove -y

# Now reinstall all our runtime dependencies
yum install -y libc-ares2 libssl1.0.0

# And remove all cached debian packages
yum clean
